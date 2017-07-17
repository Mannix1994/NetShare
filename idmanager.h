#ifndef IDMANAGER_H
#define IDMANAGER_H

#include "base.h"
#include <QNetworkInterface>
#include <QList>
#include <QHostAddress>
#include <QClipboard>
#include <QUdpSocket>
#include <QCloseEvent>
#include <QHostInfo>
#include <QHostAddress>

namespace Ui {
class IDManager;
}

class IDManager : public Base
{
    Q_OBJECT

public:
    explicit IDManager(Base *parent = 0);
    QString getServerID();
    bool hasIP(QString IP); //本机是否有这个IP
    bool hasID(QString ID); //本机是否有这个ID
    static QStringList getIPList(); //获取本机的所有非本地IP
    ~IDManager();

public slots:
    void confirm(); //确认ID是否唯一

private slots:
    void on_pbRefresh_clicked();
    void on_pbCopy_clicked();
    void on_pbClose_clicked();
    void readPendingDatagrams();
    void on_leID_textChanged(const QString &arg1);

    void on_pbCopyID_clicked();

private:
    Ui::IDManager *ui;
    QClipboard *cboard;
    QUdpSocket *udpSocket;
    void initialize();
    void getIP();//获取本机IP

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // IDMANAGER_H
