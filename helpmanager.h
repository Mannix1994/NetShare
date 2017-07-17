#ifndef HELPMANAGER_H
#define HELPMANAGER_H

#include <QWidget>

#include  "base.h"

namespace Ui {
class HelpManager;
}

class HelpManager : public Base
{
    Q_OBJECT

public:
    explicit HelpManager(Base *parent = 0);
    ~HelpManager();

private:
    Ui::HelpManager *ui;
    void initialize();
};

#endif // HELPMANAGER_H
