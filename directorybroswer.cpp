#include "directorybroswer.h"
#include "ui_directorybroswer.h"

DirectoryBroswer::DirectoryBroswer(Base *parent) :
    Base(parent),
    ui(new Ui::DirectoryBroswer)
{
    ui->setupUi(this);
}

DirectoryBroswer::DirectoryBroswer(QString url,Base *parent) :
    Base(parent),
    ui(new Ui::DirectoryBroswer),url(url)
{
    ui->setupUi(this);
    configWindow();
    initialize();
}

DirectoryBroswer::~DirectoryBroswer()
{
    delete ui;
}

/**
 * @brief DirectoryBroswer::initialize 初始化操作
 */
void DirectoryBroswer::initialize(){
    setWindowTitle("浏览目录");

    //初始化表格
    model = new QStandardItemModel();
    model->setColumnCount(5);
    model->setHeaderData(0,Qt::Horizontal,"名称");
    model->setHeaderData(1,Qt::Horizontal,"类型");
    model->setHeaderData(2,Qt::Horizontal,"大小");
    model->setHeaderData(3,Qt::Horizontal,"编辑时间");
    model->setHeaderData(4,Qt::Horizontal,"操作");
    ui->tvDirectoryTable->setModel(model);
    ui->tvDirectoryTable->setColumnWidth(0,280);
    ui->tvDirectoryTable->setColumnWidth(1,45);
    ui->tvDirectoryTable->setColumnWidth(2,45);
    ui->tvDirectoryTable->setColumnWidth(3,130);
    ui->tvDirectoryTable->setColumnWidth(4,80);
    ui->tvDirectoryTable->setStyleSheet("color:black");
    ui->tvDirectoryTable->horizontalHeader()->setStretchLastSection(true);
    ui->tvDirectoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    model->setItem(0,0,new QStandardItem("我爱你们"));
    model->setItem(0,1,new QStandardItem("别"));
    model->item(0,1)->setTextAlignment(Qt::AlignCenter);
    model->setItem(0,2,new QStandardItem("离开"));
    model->item(0,2)->setTextAlignment(Qt::AlignCenter);
    model->setItem(0,3,new QStandardItem("我"));
    model->item(0,3)->setTextAlignment(Qt::AlignCenter);
    model->setItem(0,4,new QStandardItem("返回"));
    model->item(0,4)->setTextAlignment(Qt::AlignCenter);

    acm = new QNetworkAccessManager(this);
    connect(acm,SIGNAL(finished(QNetworkReply*)),this,SLOT(refresh(QNetworkReply*)));
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    acm->get(request);
    history.append(url);
}

/**
 * @brief DirectoryBroswer::refresh 根据get请求返回的数据更新页面
 * @param reply 返回的结果
 */
void DirectoryBroswer::refresh(QNetworkReply *reply){

    if (reply->error() == QNetworkReply::NoError)
     {
         QByteArray bytes;
         QString content,type,name,size,time;
         QUrl mUrl;
         while(reply->canReadLine()){ //解析HTML文件的每一行，将我们所需要的文件信息提取出来
             ItemInfo info;
             bytes = reply->readLine();
             content = mUrl.fromPercentEncoding(bytes);
             content = content.replace("&amp;","&");
             if(content.contains("icons") && !content.contains("Name</a></th>") && !content.contains("PARENTDIR")){//从网页中把有数据的行给挑出来
                 QStringList list = content.split("\"");
                 type = QString(list.at(5)).replace(" ","");
                 name = list.at(7);
                 time = list.at(10);
                 size = list.at(12);
                 if(type =="[DIR]")
                     info.type = "目录"; //目录
                 else
                     info.type = "文件"; //文件
                 if(name.endsWith("/")) //是目录，去除”/“
                    info.itemName = name.mid(0,name.length()-1);
                 else
                    info.itemName = name;
                 info.lastEditTime = time.mid(1,16);
                 int end = size.indexOf("<")-1;
                 info.size = size.mid(1,end).replace(" ","");
                 itemInfoList.append(info);
             }
         }
         updateTable();
     }
     else
     {
        showTip("提示","连接服务器异常\n"+reply->errorString());
        mLog(reply->errorString());
     }
}

void DirectoryBroswer::updateTable()
{
    int count = model->rowCount();
    if(count>2)
        model->removeRows(2,count-2);
    int rowCount = itemInfoList.count();
    ItemInfo info;
    int where = 0;
    for(int i=0;i<rowCount;i++){ //从第二行开始添加
        info = itemInfoList.at(i);
        where = i+1;
        model->setItem(where,0,new QStandardItem(info.itemName));
        model->setItem(where,1,new QStandardItem(info.type));
        model->item(where,1)->setTextAlignment(Qt::AlignCenter);
        model->setItem(where,2,new QStandardItem(info.size));
        model->item(where,2)->setTextAlignment(Qt::AlignCenter);
        model->setItem(where,3,new QStandardItem(info.lastEditTime));
        model->item(where,3)->setTextAlignment(Qt::AlignCenter);
        if(info.type == "文件")
            model->setItem(where,4,new QStandardItem("下载"));
        else
            model->setItem(where,4,new QStandardItem("浏览|下载"));
        model->item(where,4)->setTextAlignment(Qt::AlignCenter);
        //ui->tvDirectoryTable->setIndexWidget(model->index(where,4),new QPushButton("下载"));
    }
    itemInfoList.clear();
}

void DirectoryBroswer::on_tvDirectoryTable_clicked(const QModelIndex &index)
{
    if(index.column() == 4){
        if(index.row() == 0){
            if(history.count()>1){
                history.removeLast();
                QString url = history.last();
                QNetworkRequest request;
                request.setUrl(QUrl(url));
                acm->get(request);
            }
            else{
                this->close();
            }
            return;
        }
        QString name = QString(model->item(index.row(),0)->text().toUtf8());
        QString type = QString(model->item(index.row(),1)->text().toUtf8());
        QString url = history.last();
        if(type == "文件"){
            url += name;
            url = QString(url.toUtf8());
            emit newTask(url,true);//发出下载信号给BrowseManager
        }
        else{ //是目录
            url += name+"/";
            url = QString(url.toUtf8());
            Base::Action a = question("浏 览","下 载","提示","浏览OR下载？");
            if(a== Base::ButtonYes){
                history.append(url);
                QNetworkRequest request;
                request.setUrl(QUrl(url));
                acm->get(request);
            }
            else if(a== Base::ButtonNo){
                emit newTask(url,false);
            }
        }
    }
}
