#ifndef ServerManager_H
#define ServerManager_H

#include "base.h"
#include <QFileDialog>
#include <QProcess>
#include <QTextCodec>
#include <QTcpSocket>

namespace Ui {
class ServerManager;
}

class ServerManager : public Base
{
    Q_OBJECT

typedef enum Mode{Start,Restart,Stop,Silent} Mode;

public:
    explicit ServerManager(Base *parent = 0);
    bool isSearchable();
    bool isDiscoverable();
    bool isServerRunning;
    void startServer();
    void stopServer();
    ~ServerManager();
public slots:
    void restartServer();

private slots:
    void on_pbChooseDirectory_clicked();
    void on_cbSearchable_clicked();
    void on_pbOpenMonitor_clicked();
    void on_pbStart_clicked();
    void readData();
    void on_pbStop_clicked();
    void on_pbRestart_clicked();
    void on_cbDiscoverable_clicked(bool checked);

private:
    Ui::ServerManager *ui;
    QProcess *server;
    //bool checkMode; //是否处于检测模式
    void initialize();
    void execute(Mode mode,QStringList args);
    bool isHostAccessible(QString host);

signals:
    void serverStarted();
};

#endif // ServerManager_H
