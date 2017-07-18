#define LOG //是否将调试日志记录到文件

#include "mainwidget.h"
#include <QApplication>
#include <QtMsgHandler>
#include <QMessageLogContext>
#include <QDateTime>
#include <QMutex>
#include "singleapplication.h"
#include "applicationmanager.h"

void MessageOutput(QtMsgType type , const QMessageLogContext &context , const QString &msg);

int main(int argc, char *argv[])
{
    if(argc==1){//正常运行程序
#ifdef LOG
    qInstallMessageHandler(MessageOutput); //截取qDebug等信息
#endif
    SingleApplication a(argc, argv);
    if(!a.isRunning()) {
        MainWidget w;
        a.w = &w;
        w.show();
        return a.exec();
       }
       return 0;
    }
    else if(argc == 2){ //如果有参数uninstall，则执行ApplicationManager::uninstallService();
        mDebug(argv[1]);
        if(qstrcmp(argv[1],"uninstall") == 0){
            ApplicationManager::uninstallService();
            return 0;
        }
    }
   return 0;
}

#ifdef LOG
void MessageOutput(QtMsgType type , const QMessageLogContext &context , const QString &msg)
{

//    QString log;
//    switch(type)
//    {
//    case QtDebugMsg:
//        log = QString("Debug: ");
//        break;
//    case QtWarningMsg:
//        log = QString("Error: ");
//        break;
//    case QtCriticalMsg:
//        log = QString("Critical Error: ");
//        break;
//    case QtFatalMsg:
//        log = QString("Fatal Error: ");
//        abort();
//        break;
//    default:
//        log = QString("Unknow Msg Type : ");
//        break;
//    };

    static QMutex mutex;
    mutex.lock();
    ApplicationManager::log(msg);
    mutex.unlock();
}
#endif
