#include "browsemanager.h"
#include "ui_browsemanager.h"

BrowseManager::BrowseManager(Base *parent) :
    Base(parent),
    ui(new Ui::BrowseManager)
{
    ui->setupUi(this);
    configWindow();
}

BrowseManager::BrowseManager(QString ip, QString alias, Base *parent) :
    Base(parent),alias(alias),
    ui(new Ui::BrowseManager)
{
    ui->setupUi(this);
    configWindow();
    iplist.append(ip);
    initialize();
}

BrowseManager::BrowseManager(QStringList iplist, QString alias, bool show, Base *parent) :
    Base(parent),iplist(iplist),alias(alias),
    ui(new Ui::BrowseManager)
{
    ui->setupUi(this);
    configWindow();
    initialize();
    ui->pbBrowse->setVisible(show);
}

BrowseManager::~BrowseManager()
{
    delete ui;
    delete directoryBroswer;
}

/**
 * @brief BrowseManager::initialize 初始化操作
 */
void BrowseManager::initialize(){
    setWindowTitle("选择目录");
    ui->cbIP->addItems(iplist);
    QStringList aliasList = alias.split(",");
    ui->cbAliasName->addItems(aliasList);
    ui->cbAliasName->insertSeparator(1);

    cboard = QApplication::clipboard();
}

/**
 * @brief BrowseManager::on_pbBrowse_clicked 浏览按钮槽函数
 */
void BrowseManager::on_pbBrowse_clicked()
{
    if(ui->cbAliasName->currentIndex() != 0){
        QString url="http://"+ui->cbIP->currentText()+":886/"+ui->cbAliasName->currentText()+"/";
        //QDesktopServices::openUrl(QUrl(url));
        directoryBroswer = new DirectoryBroswer(url);
        connect(directoryBroswer,SIGNAL(newTask(QString,bool)),this,SLOT(addTask(QString,bool)));
        directoryBroswer->show();
        this->close();
    }
    else
        showTip("提示","未选择要浏览的目录");
}

/**
 * @brief BrowseManager::addTask 发送newTask(url)信号，由MainWidget接收
 * @param url 要下载的url
 */
void BrowseManager::addTask(QString url, bool idFile){
    emit newTask(url,idFile); //向MainWidget发送信号
}

/**
 * @brief BrowseManager::on_pbCopy_clicked 复制链接
 */
void BrowseManager::on_pbCopy_clicked()
{
    if(ui->cbAliasName->currentIndex() != 0){
        QString url="http://"+ui->cbIP->currentText()+":886/"+ui->cbAliasName->currentText()+"/";
        cboard->setText(url);
        showTip("提示","复制成功");
    }
    else
    {
        showTip("提示","未选择要浏览的目录");
    }
}
