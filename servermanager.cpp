#include "servermanager.h"
#include "ui_servermanager.h"

ServerManager::ServerManager(Base *parent) :
    Base(parent),
    isServerRunning(false),ui(new Ui::ServerManager),mode(Silent)
{
    ui->setupUi(this);
    configWindow();
    initialize();
}

ServerManager::~ServerManager()
{
    delete ui;
    delete configIni;
}

/**
 * @brief ServerManager::initialize 一些初始化操作
 */
void ServerManager::initialize(){

    //读取配置
    setWindowTitle("服务器管理");
    configIni = new QSettings("conf.ini", QSettings::IniFormat);
    bool searchable = configIni->value("ServerManager/searchable",true).toBool();
    ui->cbSearchable->setChecked(searchable);
    bool discoverable = configIni->value("ServerManager/discoverable",true).toBool();
    ui->cbDiscoverable->setChecked(discoverable);

#ifdef WINDOWS
    QString directory = configIni->value("ServerManager/directory","").toString();
    ui->leServerDirectory->setText(directory);
#else
    ui->leServerDirectory->setText("Linux下目录不可更改");
#endif

    //服务器控制进程
    server = new QProcess();
    server->setProcessChannelMode(QProcess::MergedChannels);
    connect(server,SIGNAL(readyRead()),this,SLOT(readData()));
    connect(server,SIGNAL(readyReadStandardOutput()),this,SLOT(readData()));
    connect(server,SIGNAL(readyReadStandardOutput()),this,SLOT(readData()));

#ifdef WINDOWS
    mode = Silent;
    server->start("tasklist.exe", QStringList()<<"-fi"<<"imagename eq httpd.exe");
    server->waitForFinished(); //调用Start函数之后，应调用waitFor之类的函数，避免出现device not opend的情况
    //mode = Silent;
#endif
}

/**
 * @brief ServerManager::isSearchable 返回服务器是否可被搜索
 * @return 可搜索返回true
 */
bool ServerManager::isSearchable()
{
    return ui->cbSearchable->isChecked();
}

/**
 * @brief ServerManager::on_pbChooseDirectory_clicked 选择目录
 */
void ServerManager::on_pbChooseDirectory_clicked()
{
#ifdef WINDOWS //如果是Windows平台
    QString directory = QFileDialog::getExistingDirectory();
    if(QFile::exists(directory+"/bin/httpd.exe")){
        ui->leServerDirectory->setText(directory);
        configIni->setValue("ServerManager/directory",directory);
    }
    else{
        showTip("警告","服务器目录不正确");
    }
#endif
}

/**
 * @brief ServerManager::on_cbSearchable_clicked 写入配置文件
 */
void ServerManager::on_cbSearchable_clicked()
{
    configIni->setValue("ServerManager/searchable",ui->cbSearchable->isChecked());
}

/**
 * @brief ServerManager::on_pbOpenMonitor_clicked打开Apache监视器
 */
void ServerManager::on_pbOpenMonitor_clicked()
{
#ifdef WINDOWS //如果是Windows平台
    QString path = ui->leServerDirectory->text()+"/bin/ApacheMonitor.exe";
    if(QFile::exists(path)){
        server->startDetached(path);
    }
    else{
        showTip("警告","服务器目录不正确");
    }
#endif
}

/**
 * @brief ServerManager::on_pbStart_clicked 启动服务器
 */
void ServerManager::on_pbStart_clicked()
{
    QStringList args;
    args<<"-k"<<"start";
    this->execute(Check,args);
}

/**
 * @brief ServerManager::on_pbStop_clicked 停止服务器
 */
void ServerManager::on_pbStop_clicked()
{
    QStringList args;
    args<<"-k"<<"stop";
    this->execute(Check,args);
}

/**
 * @brief ServerManager::on_pbRestart_clicked 重启服务器
 */
void ServerManager::on_pbRestart_clicked()
{
    QStringList args;
    args<<"-k"<<"restart";
    this->execute(Check,args);
}

/**
 * @brief ServerManager::execute 控制服务器
 * @param m 模式
 * @param args 参数
 */
void ServerManager::execute(Mode m,QStringList args){
#ifdef WINDOWS
    QString path = ui->leServerDirectory->text()+"/bin/httpd.exe";
    if(QFile::exists(path)){
        mode = Server;
        server->setProcessChannelMode(QProcess::MergedChannels);
        server->start(path,args);
        server->waitForFinished();
        mode = m;
        server->start("tasklist.exe", QStringList()<<"-fi"<<"imagename eq httpd.exe");
        server->waitForFinished();
        mode = Silent;
    }
    else{
        showTip("警告","服务器目录不正确");
    }
#endif
}

/**
 * @brief ServerManager::readData 读取运行结果
 */
void ServerManager::readData()
{
    QByteArray bytes = server->readAll();
    QTextCodec *gbk = QTextCodec::codecForName("gb2312");
    QString data = gbk->toUnicode(bytes);
    if(!data.isEmpty()){
#ifdef DEBUG
        mLog(data+"\n");
#endif
        if(mode == Server){
            if(data.contains("service has stopped")){
                isServerRunning = false;
            }
            else if(data.contains("service has started")){
                isServerRunning = true;
            }
            else if(data.contains("service has restarted")){
                isServerRunning = true;
            }
            else if(data.contains("service is not started")){
                isServerRunning = false;
                showTip("提示","服务器已处于关闭状态");
            }
            else// if(data.contains("no listening sockets available")) //服务器启动错误，显示
            {
                isServerRunning = false;
                ui->leStatus->setText("停止中...");
                setEnabled(true);
                showTip("警告",data);
            }
        }
        else if(mode == Check){
            if(data.contains("httpd.exe")){
                isServerRunning = true;
                ui->leStatus->setText("运行中...");
                ui->pbStop->setEnabled(true);
                ui->pbStart->setEnabled(false);
                showTip("提示","服务器已启动");
                emit serverStarted();
            }
            else if(data.contains("没有运行的任务匹配指定标准")){
                isServerRunning = false;
                ui->leStatus->setText("停止中...");
                ui->pbStart->setEnabled(true);
                ui->pbStop->setEnabled(false);
                showTip("提示","服务器已停止");
            }
        }
        else if(mode == Silent)
        {
            if(data.contains("httpd.exe")){
                isServerRunning = true;
                ui->leStatus->setText("运行中...");
                ui->pbStop->setEnabled(true);
                ui->pbStart->setEnabled(false);
                emit serverStarted();
            }
            else if(data.contains("没有运行的任务匹配指定标准")){
                isServerRunning = false;
                ui->leStatus->setText("停止中...");
                ui->pbStart->setEnabled(true);
                ui->pbStop->setEnabled(false);
            }
        }
    }
}

/**
 * @brief ServerManager::startServer 启动服务器
 */
void ServerManager::startServer(){
    if(!isServerRunning){
       this->execute(Silent,QStringList()<<"-k"<<"start");
    }
}

/**
 * @brief ServerManager::stopServer 停止服务器
 */
void ServerManager::stopServer(){
    if(isServerRunning){
        this->execute(Silent,QStringList()<<"-k"<<"stop");
    }
}

/**
 * @brief ServerManager::restartServer 停止服务器
 */
void ServerManager::restartServer(){
    on_pbRestart_clicked();
}

/**
 * @brief ServerManager::on_cbDiscoverable_clicked 可被发现槽函数
 * @param checked 可被发现返回true
 */
void ServerManager::on_cbDiscoverable_clicked(bool checked)
{
    configIni->setValue("ServerManager/discoverable",checked);
}

/**
 * @brief ServerManager::isDiscoverable 可被搜索
 * @return 可被搜索返回true
 */
bool ServerManager::isDiscoverable(){
    return ui->cbDiscoverable->isChecked();
}
