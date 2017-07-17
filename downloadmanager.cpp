#include "downloadmanager.h"
#include "ui_downloadmanager.h"

DownloadManager::DownloadManager(Base *parent) :
    Base(parent),isDownloading(false),ui(new Ui::DownloadManager),
    acm(new QNetworkAccessManager(this)),accessManager(new QNetworkAccessManager(this)),
    reply(0),rep(0),downloadingUrl(""),urlLength(0)
{
    ui->setupUi(this);
    configWindow();
    initialize();
}

DownloadManager::~DownloadManager()
{
    delete ui;
}

void DownloadManager::initialize(){
    setWindowTitle("下载管理");
    ui->pbStop->setEnabled(false);
    ui->pbStopAll->setEnabled(false);
    //读取下载目录
    configIni = new QSettings("conf.ini", QSettings::IniFormat);
    ui->leDirectory->setText(configIni->value("DownloadManager/directory",
                                              QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first()).toString());
    connect(accessManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(addDownloadTask(QNetworkReply*)));
}

/**
 * @brief DownloadManager::on_pbDirectory_clicked 选择下载目录
 */
void DownloadManager::on_pbDirectory_clicked()
{
    QString directory = QFileDialog::getExistingDirectory();
    if(!directory.isEmpty()){
       ui->leDirectory->setText(directory);
       configIni->setValue("DownloadManager/directory",directory);
    }
    else{
        showTip("提示","未正确选择目录，未修改下载路径");
    }
}

/**
 * @brief DownloadManager::addTask 添加下载任务
 * @param mUrl 需要下载文件的url
 */
void DownloadManager::addTask(QString mUrl, bool isFile){
    this->show();
    if(isFile){
        QNetworkRequest re;
        re.setUrl(QUrl(mUrl));
        DownloadTask task = {"/",re};
        waittoDownload.append(task);
        this->updateUndownloadList();
        if(!isDownloading)
            this->download();
    }
    else{
        //
        bool tag = false;
        if(rep){
            mutex.lock();
            tag = rep->canReadLine();
            mutex.unlock();
        }
        if(!tag){
            int index = mUrl.mid(0,mUrl.length()-1).lastIndexOf("/");
            urlLength = index;
            QNetworkRequest re;
            re.setUrl(QUrl(mUrl));
            accessManager->get(re);
        }
        else{
            showTip("提示","正在解析上一个文件夹，稍后再试");
        }
    }
}

/**
 * @brief DownloadManager::download 下载文件
 */
void DownloadManager::download()
{
    isDownloading = true;
    ui->pbStop->setEnabled(true);
    ui->pbStopAll->setEnabled(true);
    ui->pbPercent->setValue(0);
    if(waittoDownload.count()>0){
        QString filename = waittoDownload.first().request.url().toString().split('/').last();
        ui->lbDownloading->setText(filename);
        QString directory = ui->leDirectory->text()+waittoDownload.first().path;
        QDir dir;
        if(!dir.exists(directory))
            dir.mkdir(directory);
        QString filepath = directory+filename;
        if(QFile::exists(filepath))
        {
            bool tag = question("提示","该文件已存在，是否继续下载？\n(下载的文件将被重命名)");
            if(tag){
                QString label = "_233";
                while(tag){
                    int index = filename.lastIndexOf('.');
                    filepath = directory+"/"+filename.mid(0,index)+label+filename.mid(index);
                    tag = QFile::exists(filepath);
                    label += "3";
                    ui->lbDownloading->setText(filepath.split('/').last());
                }
            }
            else //终止此次下载任务，继续下一个下载任务
            {
                isDownloading = false;
                ui->pbStop->setEnabled(false);
                waittoDownload.removeFirst();
                this->updateUndownloadList();
                if(waittoDownload.count()>0){ //如果还有未下载任务，则开始下载
                    this->download();
                }
                return;
            }
        }
        file = new QFile(filepath);
        if(!file->open(QIODevice::ReadWrite | QIODevice::Truncate)){
            mLog("写入文件失败:"+file->errorString());
            isDownloading = false;
            return;
        }
        reply = acm->get(waittoDownload.first().request);
        downloadingUrl = waittoDownload.first().request.url().toString();
        connect(reply,SIGNAL(finished()),this,SLOT(downloadFinished()));
        connect(reply,SIGNAL(readyRead()),this,SLOT(hasDataToRead()));
        connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(updateProgressBar(qint64,qint64)));
        emit startDownloaded(downloadingUrl,true); //发送信号给MainWidget
        waittoDownload.removeFirst();
        this->updateUndownloadList();
    }
}

/**
 * @brief DownloadManager::downloadFinished 下载完成，关闭文件。若是下载被取消，则删除文件。
 */
void DownloadManager::downloadFinished(){ //下载完成
    isDownloading = false;
    ui->pbStop->setEnabled(false);
    ui->pbStop->setEnabled(false);
    emit startDownloaded(downloadingUrl,false); //发送信号给MainWidget
    file->close();
    if(reply->error() == QNetworkReply::OperationCanceledError){ //如果是主动停止下载
        file->remove();
        ui->lbDownloading->setText("下载被取消");
    }
    else if(reply->error() == QNetworkReply::NoError)
    {
        ui->lbDownloading->setText("下载完成");
    }
    else{
        ui->lbDownloading->setText("有错误发生");
        mLog(reply->errorString());
        showWarning("提示","有错误发生!\n"+reply->errorString());
    }
    if(waittoDownload.count()>0){ //如果还有未下载任务，则开始下载
        this->download();
    }
    else{ //所有文件都被下载
        ui->teList->clear();
        ui->teList->append(QString("所有文件均已下载！"));
        ui->pbStopAll->setEnabled(false);
    }
}

/**
 * @brief DownloadManager::updateProgressBar 更新进度条
 * @param bytesread 收到的字节数
 * @param totalbytes 总的字计数
 */
void DownloadManager::updateProgressBar(qint64 bytesread, qint64 totalbytes){
    ui->pbPercent->setMaximum(totalbytes);
    ui->pbPercent->setValue(bytesread);
}

/**
 * @brief DownloadManager::hasDataToRead 收到数据，将数据写入文件
 */
void DownloadManager::hasDataToRead(){

    if(reply->error() == QNetworkReply::NoError)
    {
        if(file->isWritable())
            file->write(reply->readAll());
    }
    else{
        file->resize(0);
        showTip("提示",reply->errorString()); //下载失败原因
    }
}

/**
 * @brief DownloadManager::stopDownload 停止下载所有任务
 */
void DownloadManager::stopDownload(){ //停止下载，并清空下载任务
    if(reply)
        reply->abort();
    ui->pbPercent->setValue(0);
    waittoDownload.clear();
}

/**
 * @brief DownloadManager::on_pbStop_clicked 停止下载当前任务
 */
void DownloadManager::on_pbStop_clicked()
{
    if(reply)
        reply->abort();
    ui->pbPercent->setValue(0);
}

/**
 * @brief DownloadManager::updateUndownloadList 更新尚未下载的文件列表
 */
void DownloadManager::updateUndownloadList(){
    ui->teList->clear();
    QString item;
    int count = waittoDownload.count();
    for(int i=0;i<count;i++){
        item = QString::number(i+1)+". "+waittoDownload.at(i).request.url().toString().split('/').last();
        ui->teList->append(item);
    }
}

/**
 * @brief DownloadManager::on_pbOpen_clicked 浏览下载文件夹
 */
void DownloadManager::on_pbOpen_clicked()
{
    QString path = ui->leDirectory->text();
    if(!path.isEmpty()){
#ifdef WINDOWS
        QProcess::startDetached("explorer.exe",QStringList()<<path.replace("/","\\"));
#else
        QDesktopServices::openUrl(QUrl(path, QUrl::TolerantMode));
#endif
    }
    else
        showTip("提示","文件夹路径为空");
}

/**
 * @brief DownloadManager::on_pbDownload_clicked 添加下载任务
 */
void DownloadManager::on_pbDownload_clicked()
{
    QString mUrl = ui->leLink->text();
    ui->leLink->clear();
    if(!mUrl.isEmpty()){
        DirectoryBroswer *b = new DirectoryBroswer(mUrl);
        connect(b,SIGNAL(newTask(QString)),this,SLOT(addTask(QString)));
        b->show();
        this->close();
    }
    else
        showTip("提示","链接为空");
}

/**
 * @brief DownloadManager::addDownloadTask 浏览文件夹，递归第八文件添加到待下载任务里
 * @param re
 */
void DownloadManager::addDownloadTask(QNetworkReply *re){

    this->rep = re;
    if(rep->error() == QNetworkReply::NoError){
        //
        QByteArray bytes;
        QString content,type,mUrl;
        QString newurl = rep->url().toString();
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        while(rep->canReadLine()){ //解析HTML文件的每一行，将我们所需要的文件信息提取出来
            bytes = rep->readLine();
            content = codec->toUnicode(bytes.fromPercentEncoding(bytes));
            if(content.contains("icons") && !content.contains("Name</a></th>") && !content.contains("PARENTDIR")){//从网页中把有数据的行给挑出来
                QStringList list = content.split("\"");
                type = QString(list.at(5)).replace(" ","");
                mUrl = list.at(7);
                if(type =="[DIR]"){ //如果是文件夹
                    //
                    QNetworkRequest re;
                    re.setUrl(QUrl(newurl+mUrl));
                    accessManager->get(re);
                }
                else{ //如果是文件
                    QString path = newurl.mid(urlLength);
                    QNetworkRequest re;
                    re.setUrl(QUrl(newurl+mUrl));
                    DownloadTask task = {path,re};
                    waittoDownload.append(task);
                    this->updateUndownloadList();
                    if(!isDownloading)
                        this->download();
                }
            }
        }
    }
    else{
        showWarning("提示","连接错误："+rep->errorString());
        mLog(rep->errorString());
    }
}
/**
 * @brief DownloadManager::on_pbStopAll_clicked 取消下载所有文件
 */
void DownloadManager::on_pbStopAll_clicked()
{
    this->stopDownload();
}
