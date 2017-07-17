#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include "base.h"
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QList>
#include <QThread>
#include <QRunnable>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QMutex>
#include "directorybroswer.h"

namespace Ui {
class DownloadManager;
}

class DownloadManager : public Base
{
    Q_OBJECT

    typedef struct{
        QString path; //下载路劲
        QNetworkRequest request; //下载请求
    }DownloadTask;

public:
    explicit DownloadManager(Base *parent = 0);
    bool isDownloading;
    void stopDownload();
    ~DownloadManager();

private slots:
    void on_pbDirectory_clicked();
    void downloadFinished(); //下载完成
    void updateProgressBar(qint64 bytesread, qint64 totalbytes);//更新进度条
    void hasDataToRead();
    void on_pbStop_clicked();
    void on_pbOpen_clicked();
    void on_pbDownload_clicked();
    void addDownloadTask(QNetworkReply *re);
    void on_pbStopAll_clicked();

public slots:
    void addTask(QString url,bool isFile); //添加下载任务

private:
    Ui::DownloadManager *ui;
    QList<DownloadTask> waittoDownload;
    QNetworkAccessManager *acm; //下载管理
    QNetworkAccessManager *accessManager; //添加下载连接
    QFile *file;
    QNetworkReply *reply;
    QNetworkReply *rep;
    QString downloadingUrl; //正在下载文件的Url
    int urlLength; //记录url的长度
    QMutex mutex; //互斥锁，防止urlLength被错误的修改
    void initialize();
    void download();
    void updateUndownloadList();
signals:
    startDownloaded(QString url,bool isDownloading);
};

#endif // DOWNLOADMANAGER_H
