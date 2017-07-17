#include "idmanager.h"
#include "ui_idmanager.h"

IDManager::IDManager(Base *parent) :
    Base(parent),
    ui(new Ui::IDManager)
{
    ui->setupUi(this);
    configWindow();
    initialize();
}

IDManager::~IDManager()
{
    delete ui;
    delete udpSocket;
}

/**
 * @brief IDManager::initialize 初始化操作
 */
void IDManager::initialize(){
    setWindowTitle("ID管理");
    //设置剪贴板
    cboard = QApplication::clipboard();

    //读取本机ID
    configIni = new QSettings("conf.ini", QSettings::IniFormat);

    //设置UDP服务器
    udpSocket = new QUdpSocket();
    udpSocket->bind(2333,QAbstractSocket::ShareAddress);
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(readPendingDatagrams()));
    getIP();

    ui->leID->setText(configIni->value("IDManager/id","").toString());

    confirm();
}

/**
 * @brief IDManager::getServerID 获取本机的ID
 * @return 返回ID
 */
QString IDManager::getServerID()
{
    QString id = ui->leID->text();
    if(!(id.isEmpty()))
        return id;
    else
        return "";
}

/**
 * @brief IDManager::getIP 刷新IP
 */
void IDManager::getIP(){
    QString localHostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHostName);
    foreach(QHostAddress host,info.addresses()){
        QString ip = host.toString();
        if(host.protocol() == QAbstractSocket::IPv4Protocol && ip.mid(0,3)!="169"){
            ui->cbIP->addItem(ip);
        }
    }
    //检查是否只有一个IP且只为"127.0.0.1"，若是则说明电脑未联网
    int count = ui->cbIP->count();
    if(count==1 && ui->cbIP->currentText()=="127.0.0.1"){
        showWarning("提示","电脑尚未联网哦亲!  ");
    }
    else if(count == 0){
        showWarning("提示","电脑尚未联网哦亲！ ");
    }
}

/**
 * @brief IDManager::hasIP 本机是否有指定IP
 * @param IP 要查询的IP
 * @return 是返回true，否则返回false
 */
bool IDManager::hasIP(QString IP)
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach(QHostAddress ip,list){
        QHostAddress host(ip.toIPv4Address());
        QString item = host.toString();
        if(item == IP)
            return true;
    }
    return false;
}

/**
 * @brief IDManager::hasID 判断本机的ID是否和参数ID相同
 * @param ID 要比对的ID
 * @return 是则返回true
 */
bool IDManager::hasID(QString ID)
{
    return ID == getServerID();
}

/**
 * @brief IDManager::on_pbRefresh_clicked 刷新IP
 */
void IDManager::on_pbRefresh_clicked()
{
    ui->cbIP->clear();
    getIP();
}

/**
 * @brief IDManager::on_pbCopy_clicked 复制当前IP到剪贴板
 */
void IDManager::on_pbCopy_clicked()
{
    cboard->setText(ui->cbIP->currentText());
    showTip("提示","复制成功");
}

/**
 * @brief IDManager::on_pbClose_clicked 关闭窗口
 */
void IDManager::on_pbClose_clicked()
{
    close();
}

/**
 * @brief IDManager::confirm 确认本机ID是否已经被别的主机占用
 */
void IDManager::confirm()
{
    QByteArray bytes = ("CONFIRM:"+ui->leID->text()).toUtf8();
    QHostAddress host("255.255.255.255");
    udpSocket->writeDatagram(bytes,host,2333);
}

/**
 * @brief IDManager::readPendingDatagrams 根据收到的信息做回复
 */
void IDManager::readPendingDatagrams(){

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
        if(list.count()<2){
            mWarning("出错了兄弟！");
            return;
        }
        #ifdef DEBUG
            mLog(str+" "+host.toString());
        #endif
        if(!hasIP(host.toString())){ //如果不是是本机的IP，做回复
            if(list.at(0) == "CONFIRM"){
                if(list.at(1) == ui->leID->text())
                {
                    QByteArray byte("ITSMINE");
                    udpSocket->writeDatagram(byte,host,2333);
                }
            }
            else if(list.at(0)=="ITSMINE"){
                this->show();
                ui->leID->setStyleSheet("color:red");//文本颜色
                ui->laTip->setText("ID已存在");
                ui->laTip->setStyleSheet("color:red");
                showTip("警告","ID已存在,请更换一个ID");
            }
        }
    }
}

/**
 * @brief IDManager::on_leID_textChanged ID变化时，检查此ID是否可用
 * @param arg1 变化后的ID
 */
void IDManager::on_leID_textChanged(const QString &arg1)
{
    if(!arg1.isEmpty()){
        if(arg1.contains(':') || arg1.contains("：")){
            showWarning("警告","ID之中不能包含冒号");
            QString tmp = ui->leID->text();
            ui->leID->setText(tmp.mid(0,tmp.length()-1));
        }
        else{
            ui->leID->setStyleSheet("color:black");//文本颜色
            ui->laTip->setText("ID可用");
            ui->laTip->setStyleSheet("color:black");
            configIni->setValue("IDManager/id",arg1);
            confirm();
        }
    }
    else{
        ui->laTip->setText("ID不能为空");
        ui->laTip->setStyleSheet("color:red");
    }
}

/**
 * @brief IDManager::closeEvent 关闭之前检查ID是否为空
 * @param event 退出事件
 */
void IDManager::closeEvent(QCloseEvent *event){
    if(ui->leID->text().isEmpty()){
        showError("错误","ID为空");
        event->ignore();
    }
    else
        event->accept();
}

/**
 * @brief IDManager::on_pbCopyID_clicked 复制ID
 */
void IDManager::on_pbCopyID_clicked()
{
    cboard->setText(ui->cbIP->currentText());
    showTip("提示","复制成功");
}

/**
 * @brief IDManager::getIPList 获取本机所有IP
 * @return 若IP大于一个,返回一个以','分割的QString
 */
QStringList IDManager::getIPList(){
    QStringList iplist;
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach(QHostAddress ip,list){
        QHostAddress host(ip.toIPv4Address());
        QString item = host.toString();
        if(item != "0.0.0.0" && item != "127.0.0.1" && item.mid(0,3) !="169")
            iplist.append(host.toString());
    }
    return iplist;
}
