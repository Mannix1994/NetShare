#ifndef DIRECTORYBROSWER_H
#define DIRECTORYBROSWER_H

#include "base.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QCloseEvent>
#include <QList>
#include <QStandardItemModel>
#include <QPushButton>

namespace Ui {
class DirectoryBroswer;
}

class DirectoryBroswer : public Base
{
    Q_OBJECT

    typedef struct{
        QString type; //条目类型
        QString itemName;
        QString lastEditTime;
        QString size;
    }ItemInfo;

public:
    explicit DirectoryBroswer(Base *parent = 0);
    DirectoryBroswer(QString url,Base *parent = 0);
    ~DirectoryBroswer();

private:
    Ui::DirectoryBroswer *ui;
    QNetworkAccessManager *acm; //请求管理器
    QStandardItemModel *model;
    QString url;
    QList<QString> history; //连接地址记录
    QList<ItemInfo> itemInfoList; //记录条目信息
    void initialize();
    void updateTable();

private slots:
    void refresh(QNetworkReply *reply);

    void on_tvDirectoryTable_clicked(const QModelIndex &index);

signals:
    void newTask(QString url,bool isFile);

};

#endif // DIRECTORYBROSWER_H
