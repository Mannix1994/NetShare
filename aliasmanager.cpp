#include "aliasmanager.h"
#include "ui_aliasmanager.h"

AliasManager::AliasManager(Base *parent) :
    Base(parent),ui(new Ui::AliasManager),browseManger(0)
{
    ui->setupUi(this);
    configWindow();
    initialize();
}

/**
 * @brief AliasManager::initialize 一些初始化操作
 */
void AliasManager::initialize(){
    Base::initialize();
    setWindowTitle("映射管理");
    //初始化表格
    model = new QStandardItemModel();
    model->setColumnCount(3);
    model->setHeaderData(0,Qt::Horizontal,"别名");
    model->setHeaderData(1,Qt::Horizontal,"本机目录");
    model->setHeaderData(2,Qt::Horizontal,"操作");
    ui->tvAliasList->setModel(model);
    ui->tvAliasList->setColumnWidth(0,60);
    ui->tvAliasList->setColumnWidth(1,150);
    ui->tvAliasList->setColumnWidth(2,40);
    ui->tvAliasList->setStyleSheet("color:black");
    ui->tvAliasList->horizontalHeader()->setStretchLastSection(true);
    ui->tvAliasList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    getAlias(&aliases);
    updateTable();

    ui->leDirectory->setReadOnly(true);
}

AliasManager::~AliasManager()
{
    delete ui;
}

/**
 * @brief AliasManager::getAliasList 获取别名列表
 * @return 返回以','分割的别名列表
 */
QString AliasManager::getAliasList(){
    QString list("");
    QAlias::Iterator it = aliases.begin();
    while(it != aliases.end()){
        list+=it.key()+",";
        it++;
    }
    return list.mid(0,list.count()-1);
}
/**
 * @brief AliasManager::on_pbChooseDirectory_clicked 选择目录
 */
void AliasManager::on_pbChooseDirectory_clicked()
{
    QString directory = QFileDialog::getExistingDirectory();
    ui->leDirectory->setText(directory);
}

/**
 * @brief AliasManager::getAlias 在文件中读取映射信息
 * @param aliases
 */
void AliasManager::getAlias(QAlias *aliases)
{
#ifdef Q_OS_WIN32
    QString filePath = configIni->value("ServerManager/directory","").toString()+"/conf/alias.conf";
#elif defined(Q_OS_LINUX)
    QString filePath = "/etc/apache2/mods-enabled/mAlias.conf";
#endif
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        showTip("警告","打开Alias文件失败");
    }
    else{
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QString text;
        while(!in.atEnd()){
            text = in.readLine();
            if(text.contains("Alias")){
                text = text.replace("\t","");
                text = text.replace("\r","");
                text = text.replace("\n","");
                text = text.replace("\"","");
                QStringList list = text.split(" ");
                QString key,value;
                key = list.at(1);
                key = key.mid(1);
                value = list.at(2);
                //value = value.mid(1,value.length()-3);
                aliases->insert(key,value);
            }
        }
        in.flush();
    }
    file.close();
}

/**
 * @brief AliasManager::saveAlias 保存映射信息到本机文件
 */
void AliasManager::saveAlias()
{
#ifdef Q_OS_WIN32
    QString filePath = configIni->value("ServerManager/directory","").toString()+"/conf/alias.conf";
#elif defined(Q_OS_LINUX)
    QString filePath = "/etc/apache2/mods-enabled/mAlias.conf";
#endif
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        showTip("警告","打开Alias文件失败");
    }
    else{
        QTextStream out(&file);
        out.setCodec("UTF-8");
        QString line1,line2,line3,line4,line5,line6,all;
        line3 = "\tOptions Indexes\n";
        line4 = "\tIndexOptions NameWidth=100\n";
        line5 = "\tRequire all granted\n";
        line6 = "</Directory>\n\n";
        QAlias::Iterator it = aliases.begin();
        while(it != aliases.end()){
            line1 = "Alias /"+it.key()+" \""+it.value()+"\"\n";
            line2 = "<Directory \""+it.value()+"\">\n";
            all = line1+line2+line3+line4+line5+line6;
#ifdef DEBUG
            mDebug(all);
#endif
            out<<all;
            it++;
        }
        out.flush();
    }
    file.close();
}

/**
 * @brief AliasManager::updateTable 更新表格中的映射目录
 */
void AliasManager::updateTable()
{
    //更新表
    model->removeRows(0,model->rowCount());
    QAlias::Iterator it = aliases.begin();
    int count = 0;
    while(it != aliases.end()){
        model->setItem(count,0,new QStandardItem(it.key()));
        model->setItem(count,1,new QStandardItem(it.value()));
        model->setItem(count,2,new QStandardItem("删除"));
        it++;
        count++;
    }
}

/**
 * @brief AliasManager::on_pbAdd_clicked 添加一条新的映射
 */
void AliasManager::on_pbAdd_clicked()
{
    QString alias=ui->leAliasName->text();
    QString directory = ui->leDirectory->text();
    if(alias.isEmpty()){
        showTip("提示","别名为空");
    }
    else if(directory.isEmpty()){
        showTip("提示","目录为空");
    }
    else{
        aliases.insert(alias,directory);
        updateTable();
        saveAlias();
        ui->leAliasName->clear();
        ui->leDirectory->clear();
        if (question("提示","检测到别名表已更新，是否重启服务器?")) {
            emit aliasUpdated(); //发送映射更新信号，通知重启服务器
        }
    }
}

/**
 * @brief AliasManager::on_tvAliasList_clicked 表格被点击时的槽函数
 * @param index 被点击的QModelIndex
 */
void AliasManager::on_tvAliasList_clicked(const QModelIndex &index)
{
    if(index.column()==2){
        if (question("提示","确认删除?")) {
            int row = index.row();
            QString key = model->index(row,0).data().toString();
            aliases.remove(key);
            updateTable();
            saveAlias();
            if (question("提示","检测到别名表已更新，是否重启服务器?")) {
                emit aliasUpdated();
            }
        }
    }
}

/**
 * @brief AliasManager::on_pbShare_clicked 分享链接
 */
void AliasManager::on_pbShare_clicked()
{
    browseManger = new BrowseManager(IDManager::getIPList(),this->getAliasList(),false);
    browseManger->show();
}
