#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "base.h"
#include "aliasmanager.h"
#include "idmanager.h"
#include "mainwidget.h"
#include "servermanager.h"
#include "browsemanager.h"
#include "downloadmanager.h"
#include "applicationmanager.h"
#include "helpmanager.h"

#include <QUdpSocket>
#include <QStandardItemModel>
#include <QStringList>
#include <QStandardItem>
#include <QByteArray>
#include <QModelIndex>
#include <QCloseEvent>
#include <QAction>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMap>
#include <QDesktopWidget>

namespace Ui {
class MainWidget;
}

class MainWidget : public Base
{
    Q_OBJECT

    typedef struct{
        int isDownloading; //正在被别的机器下载的文件数量
        QStringList files; //被下载的文件名称
    }DownloadingInfo; //正在下在的文件信息
    typedef struct {
        QString id; //对方ID
        QString aliaes; //以逗号分隔的别名列表
    }HostInfo; //主机信息
    typedef QMap<QString,HostInfo> HostList; //主机信息列表


public:
    explicit MainWidget(Base *parent = 0);
    BrowseManager *browseManager; //浏览别名界面类
    ~MainWidget();

private slots:
    void on_pbIDManager_clicked(); //打开ID管理界面
    void on_pbServerManager_clicked(); //打开服务器管理界面
    void on_pbAliasManager_clicked(); //打开目录映射界面
    void readPendingDatagrams(); //udpSoket槽函数
    void on_pbSearch_clicked(); //搜素指定服务器
    void on_pbRefresh_clicked(); //搜索全部在线服务器
    void on_tvServerTable_clicked(const QModelIndex &index); //表被点击槽函数
    void updateMessage(); //向局域网广播更新后的别名目录
    void on_pbDownloadManager_clicked(); //下载管理器界面
    void addTask(QString url,bool isFile); //向下载管理器添加下载任务
    void on_pbHelp_clicked();
    void downloading(QString mUrl, bool isDownloading); //正在下载某文件的槽函数

    //托盘图标
    void minWindow();
    void showNormal(); //恢复显示窗口
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage();

private:
    Ui::MainWidget *ui;
    IDManager *idManager;
    ServerManager *serverManager;
    AliasManager *aliasManager;
    DownloadManager *downloadManager;
    HelpManager *helpManager;
    QUdpSocket *udpSocket; //Udp服务器，收发指令
    QStandardItemModel  *model; //服务器列表
    int rowCount; //当前有多少行主机信息
    DownloadingInfo fileInfo;
    HostList hostList;
    void initialize(); //初始化操作
    void configWindow();
    QString getFilesList(QStringList list,QString seprator);
    void updateTable();

    //托盘图标
    QAction *restoreAction;
    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    void createActions();
    void createTrayIcon();

protected:
    void closeEvent(QCloseEvent *event); //重写关闭事件

};

#endif // MAINWIDGET_H
