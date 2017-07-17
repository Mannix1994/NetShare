#ifndef ApplicationManager_H
#define ApplicationManager_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTextStream>
#include <QDebug>
#include <QProcess>
#include <QTextCodec>
#include <QTime>

#define WINDOWS //代表Windows平台
#define DEBUG //调试模式

//显示调试信息的宏
#define mDebug(x)     (qDebug()    << "Debug   " + QString(__DATE__).replace(" ",".") + " " + QString(__TIME__) + " " + QString(__FILE__) + " " + QString(__FUNCTION__) + " " + QString::number(__LINE__) + ">>" + " " + x)
#define mWarning(x)   (qWarning()  << "Warning " + QString(__DATE__).replace(" ",".") + " " + QString(__TIME__) + " " + QString(__FILE__) + " " + QString(__FUNCTION__) + " " + QString::number(__LINE__) + ">>" + " " + x)
#define mError(x)     (qCritical() << "Error   " + QString(__DATE__).replace(" ",".") + " " + QString(__TIME__) + " " + QString(__FILE__) + " " + QString(__FUNCTION__) + " " + QString::number(__LINE__) + ">>" + " " + x)
#define mInfo(x)      (qInfo()     << "Infor   " + QString(__DATE__).replace(" ",".") + " " + QString(__TIME__) + " " + QString(__FILE__) + " " + QString(__FUNCTION__) + " " + QString::number(__LINE__) + ">>" + " " + x)

//日志记录宏
#define mLog(x)       (ApplicationManager::log(QString("Log      ")+QString(__DATE__).replace(" ",".") + " " + QString(__TIME__) + " " + QString(__FILE__) + " " + QString(__FUNCTION__) + " " + QString::number(__LINE__) + ">>" + " " + x))


class ApplicationManager : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationManager(QObject *parent = 0);
    static void init(); //第一次运行时运行此函数，完成http.conf和conf.ini的设置，安装Apache服务
    static void log(QString mlog); //写日志
    static void uninstallService(); //卸载Apache服务

private:
    static void installService(); //安装Apache服务
signals:

public slots:
};

#endif // ApplicationManager_H
