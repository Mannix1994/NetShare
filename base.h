#ifndef BASE_H
#define BASE_H

#include <QWidget>
#include <QToolButton>
#include <QPoint>
#include <QMouseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QException>
#include "applicationmanager.h"

namespace Ui {
class Base;
}

class Base : public QWidget
{
    Q_OBJECT

public:

    explicit Base(QWidget *parent = 0);
    QPoint last; //窗口最后的坐标
    QSettings *configIni; //配置文件读写指针
    enum Action{ButtonYes,ButtonNo,ButtonClose};
    QToolButton *minButton; //最小化按钮
    QToolButton *closeButton; //关闭按钮
    virtual void configWindow();
    static void showTip(QString title, QString tip);
    static void showWarning(QString title, QString tip);
    static void showError(QString title, QString tip);
    virtual bool question(QString title, QString content);
    virtual Action question(QString buttonYesText, QString buttonNoText, QString title, QString content);
    virtual void initialize()=0;
    ~Base();

private:
    Ui::Base *ui;

public slots:
    void slot_minWindow();

protected:
    void mousePressEvent (QMouseEvent *e);
    void mouseMoveEvent (QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
};

#endif // BASE_H
