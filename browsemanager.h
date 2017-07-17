#ifndef BROWSEMANAGER_H
#define BROWSEMANAGER_H

#include "base.h"

#include<QStringList>
#include<QDesktopServices>
#include <QClipboard>
#include "directorybroswer.h"

namespace Ui {
class BrowseManager;
}

class BrowseManager : public Base
{
    Q_OBJECT

public:
    explicit BrowseManager(Base *parent = 0);
    BrowseManager(QString ip,QString alias,Base *parent = 0);
    BrowseManager(QStringList iplist, QString alias, bool show, Base *parent = 0);
    ~BrowseManager();

private slots:
    void on_pbBrowse_clicked();
    void addTask(QString url,bool idFile);

    void on_pbCopy_clicked();

private:
    QString ip;
    QStringList iplist;
    QString alias;
    Ui::BrowseManager *ui;
    DirectoryBroswer *directoryBroswer;
    QClipboard *cboard;
    void initialize();

signals:
    void newTask(QString url,bool isFile);
};

#endif // BROWSEMANAGER_H
