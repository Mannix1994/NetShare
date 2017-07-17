#include "applicationmanager.h"

ApplicationManager::ApplicationManager(QObject *parent) : QObject(parent)
{
}

/**
 * @brief ApplicationManager::init 第一次运行此程序时，进行一些初始化的操作。
 * 新建配置文件conf.ini、修改配置文件httpd.conf、安装Apache服务
 */
void ApplicationManager::init()
{
#ifdef WINDOWS
    //新建配置文件conf.ini
    QSettings *config = new QSettings("conf.ini", QSettings::IniFormat);
    QString path = QDir::currentPath()+"/Apache24";
#ifdef DEBUG
    mLog(path);
#endif
    config->setValue("MainWidget/first",false);
    config->setValue("ServerManager/directory",path);
    config->setValue("ServerManager/searchable",true);
    config->setValue("ServerManager/discoverable",true);
    qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
    uint id = 100000 + qrand()%10000000;
    config->setValue("IDManager/id",QString::number(id));

    //修改配置文件httpd.conf
    QString backuppath = path+"/conf/httpd_backup.conf";
    QString httpdconfPath = path+"/conf/httpd.conf";
    mLog(backuppath);
    if(QFile::exists(backuppath)){
        QFile file(httpdconfPath),backup(backuppath);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            file.close();
            mLog("写入配置文件失败");
            return;
        }
        if(!backup.open(QIODevice::ReadOnly | QIODevice::Text)){
            backup.close();
            mLog("打开原配置文件失败");
            return;
        }
        QTextStream in(&backup);
        QTextStream out(&file);
        in.setCodec("UTF-8");
        out.setCodec("UTF-8");
        QString text;
        while(!in.atEnd()){
            text = in.readLine()+"\n";
            if(text.contains("Define SRVROOT")){ //只修改"Define SRVROOT"一行
                text = "Define SRVROOT \""+path+"\"\n";
                mLog(text);
            }
            out<<text;
        }
        out.flush();
        backup.close();
        file.close();
        //安装服务
        installService();
    }
    else{
        mLog("原配置文件不存在");
    }
    delete config;
#endif
}
/**
 * @brief ApplicationManager::log 记录日志。
 * @param mlog 需要记录的信息
 */
void ApplicationManager::log(QString mlog){

#ifdef WINDOWS
    QString path = QDir::currentPath()+"/debug.log";
    QFile file(path);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)){
        file.close();
        return;
    }
    else{
        QTextStream out(&file);
        out.setCodec("GBK");
        out<<mlog<<"\n";
        out.flush();
    }
    file.close();
#endif
}

/**
 * @brief ApplicationManager::installService 安装Apache服务
 */
void ApplicationManager::installService(){
#ifdef WINDOWS
    QProcess *p = new QProcess();
    QString path = QDir::currentPath()+"/Apache24/bin/httpd.exe";
    if(QFile::exists(path)){
        p->start(path,QStringList()<<"-k"<<"install");
        p->waitForFinished();
        QByteArray bytes = p->readAll();
        QTextCodec *gbk = QTextCodec::codecForName("gb2312");
        QString data = gbk->toUnicode(bytes);
        mLog(data);
    }
    delete p;
#endif
}

/**
 * @brief ApplicationManager::uninstallService 卸载Apache服务
 */
void ApplicationManager::uninstallService(){
#ifdef WINDOWS
    QProcess *p = new QProcess();
    QString path = QDir::currentPath()+"/Apache24/bin/httpd.exe";
    if(QFile::exists(path)){
        p->start(path,QStringList()<<"-k"<<"stop"); //先停止
        p->waitForFinished();
        p->start(path,QStringList()<<"-k"<<"uninstall"); //然后卸载
        p->waitForFinished();
        QByteArray bytes = p->readAll();
        QTextCodec *gbk = QTextCodec::codecForName("gb2312");
        QString data = gbk->toUnicode(bytes);
        mLog(data);
    }
    delete p;
#endif
}
