#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(Base *parent) :
    Base(parent),ui(new Ui::MainWidget),
    rowCount(0),fileInfo({0,QStringList()})
{
    ui->setupUi(this);
    configWindow(); //配置窗口
    initialize(); //初始化
}

void MainWidget::configWindow(){
    Base::configWindow();
    //设置托盘图标
    if(QSystemTrayIcon::isSystemTrayAvailable())
    {
        createActions();
        createTrayIcon();
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        trayIcon->show();
    }
}

void MainWidget::initialize()
{
    Base::initialize();
    bool firstTime = configIni->value("MainWidget/first",true).toBool();
    if(firstTime){
        ApplicationManager::init(); //初次运行，进行一些配置
    }

    helpManager = new HelpManager();
    idManager = new IDManager();
    serverManager = new ServerManager();
    aliasManager = new AliasManager();
    downloadManager = new DownloadManager();
    connect(serverManager,SIGNAL(serverStarted()),idManager,SLOT(confirm()));
    connect(aliasManager,SIGNAL(aliasUpdated()),this,SLOT(updateMessage()));
    connect(aliasManager,SIGNAL(aliasUpdated()),serverManager,SLOT(restartServer()));
    connect(downloadManager,SIGNAL(startDownloaded(QString,bool)),this,SLOT(downloading(QString,bool)));
    serverManager->startServer();

    //设置UDP服务器
    udpSocket = new QUdpSocket();
    udpSocket->bind(23333,QAbstractSocket::ShareAddress);
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(readPendingDatagrams()));

    //初始化表格
    model = new QStandardItemModel();
    model->setColumnCount(4);
    model->setHeaderData(0,Qt::Horizontal,"IP");
    model->setHeaderData(1,Qt::Horizontal,"ID");
    model->setHeaderData(2,Qt::Horizontal,"别名列表");
    model->setHeaderData(3,Qt::Horizontal,"操作");

    ui->tvServerTable->setModel(model);
    ui->tvServerTable->setColumnWidth(0,120);
    ui->tvServerTable->setColumnWidth(1,80);
    ui->tvServerTable->setColumnWidth(2,80);
    ui->tvServerTable->setColumnWidth(3,30);
    ui->tvServerTable->setStyleSheet("color:black");
    ui->tvServerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tvServerTable->horizontalHeader()->setStretchLastSection(true);
}

MainWidget::~MainWidget()
{
    delete ui;
    delete idManager;
    delete serverManager;
    delete aliasManager;
    delete udpSocket;
    delete model;
}

void MainWidget::on_pbIDManager_clicked()
{
    idManager->show();
}


void MainWidget::on_pbServerManager_clicked()
{
    serverManager->show();
}

void MainWidget::on_pbAliasManager_clicked()
{
    aliasManager->show();
}
/**
 * @brief MainWidget::readPendingDatagrams 根据收到的信息，更新在线服务器列表；发送自己的ID和别名列表
 */
void MainWidget::readPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray bytes;
        bytes.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(bytes.data(),bytes.size(),&sender,&senderPort);
        QString str = QString::fromUtf8(bytes);
        QHostAddress host(QHostAddress(sender.toIPv4Address()));
        QStringList list = str.split(":");
        if(list.count() == 0){
            mWarning("出错了兄弟");
            return;
        }
        if(str=="FIND"){ //收到广播信息
            //返回本机ID和别名列表
            if(serverManager->isDiscoverable() && serverManager->isServerRunning){//如果本机可被"发现",且本机的服务器在运行
                //向sender发送本机ID和别名列表
                QString id = idManager->getServerID();
                QString alias = aliasManager->getAliasList();
                if((!id.isEmpty()) && (!alias.isEmpty()))
                {
                    //发送数据
                    QByteArray bytes = ("HELLO:"+id+":"+alias).toUtf8();
                    udpSocket->writeDatagram(bytes,host,23333);
                }
            }
        }
        else if(list.first()=="SEARCH"){ //搜索主机
            if(list.count() < 2){
                mWarning("出错了兄弟");
                return;
            }
            if(idManager->hasIP(list.at(1)) || idManager->hasID(list.at(1))){
                if(serverManager->isSearchable() && serverManager->isServerRunning){//如果本机可被搜索,且本机的服务器在运行
                    //向sender发送本机ID和别名列表
                    QString id = idManager->getServerID();
                    QString alias = aliasManager->getAliasList();
                    if((!id.isEmpty()) && (!alias.isEmpty()))
                    {
                        //发送数据
                        QByteArray bytes = ("HELLO:"+id+":"+alias).toUtf8();
                        udpSocket->writeDatagram(bytes,host,23333);
                    }
                }
            }
        }
        else if(list.first()=="HELLO"){//收到主机信息
            if(list.count() < 3){
                mWarning("出错了兄弟");
                return;
            }
            HostInfo info = {list.at(1),list.at(2)};
            hostList.insert(host.toString(),info);
            updateTable();
        }
        else if(list.first()=="UPDATE"){//收到更新信息
            if(list.count() < 3){
                mWarning("出错了兄弟");
                return;
            }
            //判断该IP是否在表中已存在，如已存在，更新其信息
            if(hostList.contains(host.toString())){
                HostInfo info = {list.at(1),list.at(2)};
                hostList.insert(host.toString(),info);
                updateTable();//更新表
            }
        }
        else if(list.first()=="DOWNLOADING"){ //收到别人正在下载本机文件的信息
            if(list.count() < 2){
                mWarning("出错了兄弟");
                return;
            }
            if(list.at(1) == "true"){
                fileInfo.isDownloading++;
                fileInfo.files.append(list.at(2));
            }
            else if(list.at(1) == "false"){
                fileInfo.isDownloading--;
                fileInfo.files.removeOne(list.at(2));
            }
        }
    }
}

void MainWidget::updateTable(){
    //更新表
    model->removeRows(0,model->rowCount());
    HostList::Iterator it = hostList.begin();
    HostList::Iterator end = hostList.end();
    int count = 0;
    while(it != end){
        model->setItem(count,0,new QStandardItem(it.key()));
        model->setItem(count,1,new QStandardItem(it.value().id));
        model->setItem(count,2,new QStandardItem(it.value().aliaes));
        model->setItem(count,3,new QStandardItem("浏览"));
        it++;
        count++;
    }
}

/**
 * @brief MainWidget::on_pbSearch_clicked 发送搜索指定服务器的信息
 */
void MainWidget::on_pbSearch_clicked()
{
    model->removeRows(0,model->rowCount());
    hostList.clear();
    QString key = ui->leSearch->text();
    if(!key.isEmpty()){
        QByteArray bytes = ("SEARCH:"+key).toUtf8();
        QHostAddress host("255.255.255.255");
        udpSocket->writeDatagram(bytes,host,23333);
    }
}

/**
 * @brief MainWidget::on_pbRefresh_clicked 搜索本局域网内全部在线服务器
 */
void MainWidget::on_pbRefresh_clicked()
{
    model->removeRows(0,model->rowCount());
    hostList.clear();
    QByteArray bytes("FIND");
    QHostAddress host("255.255.255.255");
    udpSocket->writeDatagram(bytes,host,23333);
}

/**
 * @brief MainWidget::on_tvServerTable_clicked 在线服务器列表被点击的槽函数
 * @param index 被点击的QModelIndex
 */
void MainWidget::on_tvServerTable_clicked(const QModelIndex &index)
{
    if(index.column()==3){//“浏览”那一列被点击
        //打开浏览管理器
        QString ip = model->index(index.row(),0).data().toString();
        QString alias = model->index(index.row(),2).data().toString();

        browseManager = new BrowseManager(ip,alias);
        connect(browseManager,SIGNAL(newTask(QString,bool)),this,SLOT(addTask(QString,bool)));
        browseManager->show();
    }
}

/**
 * @brief MainWidget::closeEvent 重写关闭事件
 * @param event
 */
void MainWidget::closeEvent(QCloseEvent *event)
{
    Base::Action close = question("关 闭","隐 藏","提示","关闭网络共享？");
    if(close == Base::ButtonYes){
        if(downloadManager->isDownloading){
            if(question("提示","当前正在下载文件，退出将停止下载\n是否退出？")){
                downloadManager->stopDownload();
                if(serverManager->isServerRunning && fileInfo.isDownloading>0){
                    if(question("提示","是否关闭服务器？\n(你的朋友正在下载文件"+getFilesList(fileInfo.files,"\n")+"，不建议关闭)")){
                        serverManager->stopServer();
                    }
                }
                event->accept();
            }
            else
                event->ignore();
        }
        else
        {
            if(serverManager->isServerRunning && fileInfo.isDownloading>0){
                if(question("提示","是否关闭服务器？\n(你的朋友正在下载文件"+getFilesList(fileInfo.files,"\n")+"，不建议关闭)")){
                    serverManager->stopServer();
                }
            }
            else if(serverManager->isServerRunning){
                if(question("提示","是否关闭服务器？")){
                    serverManager->stopServer();
                }
            }
            event->accept();
        }
    }
    else if(close == Base::ButtonNo){
        this->hide();
        this->showMessage();
        event->ignore();
    }
    else{
       event->ignore();
    }
}

/**
 * @brief MainWidget::updateMessage 向本局域网的在线服务器发送更新后的信息
 */
void MainWidget::updateMessage(){
    QHostAddress host("255.255.255.255");
    QString id = idManager->getServerID();
    QString alias = aliasManager->getAliasList();
    if((!id.isEmpty()) && (!alias.isEmpty()))
    {
        //发送数据
        QByteArray bytes = ("UPDATE:"+id+":"+alias).toUtf8();
        udpSocket->writeDatagram(bytes,host,23333);
    }
}

/**
 * @brief MainWidget::on_pbDownloadManager_clicked 显示下载管理器界面
 */
void MainWidget::on_pbDownloadManager_clicked()
{
    downloadManager->show();
}

/**
 * @brief MainWidget::addTask 向下载管理器添加下载任务
 * @param url 需要下载的文件的Url
 */
void MainWidget::addTask(QString url, bool isFile){
    //mDebug(url);
    downloadManager->addTask(url,isFile);
}

void MainWidget::on_pbHelp_clicked()
{
    helpManager->show();
}


/**
 * @brief MainWidget::createActions 创建托盘图标Action
 */
void MainWidget::createActions()
{
    QPixmap minPix  = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
    QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
    restoreAction = new QAction("恢复(&R)", this);
    restoreAction->setIcon(minPix);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    quitAction = new QAction("退出(&Q)", this);
    quitAction->setIcon(closePix);
    connect(quitAction,SIGNAL(triggered()), this, SLOT(close()));
}

/**
 * @brief MainWidget::createTrayIcon 创建托盘图标
 */
void MainWidget::createTrayIcon()
{
    //QPixmap icon  = style()->standardPixmap(QStyle::SP_DriveNetIcon);
    //QString icopath = QDir::currentPath()+"/48.ico";
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/Icon/48.ico"));
    //鼠标放托盘图标上提示信息
    trayIcon->setToolTip("网络共享");
}

/**
 * @brief MainWidget::iconActivated 托盘图标被点击
 * @param reason
 */
void MainWidget::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        if(this->isMinimized())
        {
           this->showFullScreen();
        }
        else
           this->show();
    }
}

/**
 * @brief MainWidget::showMessage 显示信息
 */
void MainWidget::showMessage()
{
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(1);
    trayIcon->showMessage("网络共享", "你居然！！！把我隐藏了！！！", icon,1500);
}

/**
 * @brief MainWidget::minWindow 最小化
 */
void MainWidget::minWindow()
{
    this->showMinimized();
}

/**
 * @brief MainWidget::showNormal 显示
 */
void MainWidget::showNormal(){
    this->show();
}

/**
 * @brief MainWidget::downloading 向url对应的主机发送正在下载的文件信息
 * @param url
 */
void MainWidget::downloading(QString mUrl, bool isDownloading){

    QString tag = "false:";
    if(isDownloading)
        tag = "true:";
    QStringList list = mUrl.split('/');
    if(list.count()>=3){
        QString ip = list.at(2).split(':').first();
        QString data = "DOWNLOADING:"+tag+list.last();
        //向url对应的主机发送信息
        udpSocket->writeDatagram(data.toUtf8(),QHostAddress(ip),23333);
    }
    else{
        mWarning("Something wrong happened");
    }
}

/**
 * @brief MainWidget::getFilesList 返回以seprator分割的QString
 * @param list 原QStringList
 * @param seprator 分割符
 */
QString MainWidget::getFilesList(QStringList list,QString seprator){
    QString tmp;
    for(int i=0;i<list.count();i++){
        tmp += list.at(i)+seprator;
    }
    return tmp.mid(0,tmp.length()-1);
}
