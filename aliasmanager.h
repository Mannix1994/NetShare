#ifndef ALIASMANAGER_H
#define ALIASMANAGER_H

#include "base.h"
#include "browsemanager.h"
#include "idmanager.h"
#include <QFileDialog>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QStandardItem>
#include <QMap>
#include <QSettings>

namespace Ui {
class AliasManager;
}

class AliasManager : public Base
{
    Q_OBJECT

    typedef QMap<QString,QString> QAlias;

public:
    explicit AliasManager(Base *parent = 0);
    QString getAliasList();
    ~AliasManager();

private slots:
    void on_pbChooseDirectory_clicked();

    void on_pbAdd_clicked();

    void on_tvAliasList_clicked(const QModelIndex &index);

    void on_pbShare_clicked();

private:
    Ui::AliasManager *ui;

    QStandardItemModel *model;
    QAlias aliases;
    BrowseManager *browseManger;
    void initialize();
    void getAlias(QAlias *aliases); //获取别名目录，存入QMap中
    void saveAlias(); //保存别名目录
    void updateTable(); //更新表
signals:
    void aliasUpdated();
};

#endif // ALIASMANAGER_H
