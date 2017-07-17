#include "helpmanager.h"
#include "ui_helpmanager.h"

HelpManager::HelpManager(Base *parent) :
    Base(parent),
    ui(new Ui::HelpManager)
{
    ui->setupUi(this);
    configWindow();
    initialize();
}

HelpManager::~HelpManager()
{
    delete ui;
}

void HelpManager::initialize(){
    setWindowTitle("使用帮助");
}
