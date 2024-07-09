#include "levelcalibration.h"
#include "ui_levelcalibration.h"

levelCalibration::levelCalibration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::levelCalibration)
{
    ui->setupUi(this);
    setWindowTitle("Калибровка уровня штатива");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint);
//    raise();
//    activateWindow();
    resize(512, 512);
    QPixmap icon("/home/shine/QtProjects/ShtativTerminal/cs go donk 666 aim cool.png");
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Normal, QPalette::Window, QBrush(icon));
    pal.setBrush(QPalette::Inactive, QPalette::Window, QBrush(icon));
    setPalette(pal);
    setMask(icon.mask());

    QPushButton closeBtn("X", this);
    closeBtn.setFixedSize(30, 30);
    closeBtn.move(220, 220);
    connect(&closeBtn, SIGNAL(clicked(bool)), this, SLOT(hideWindow()));

    hide();
}

levelCalibration::~levelCalibration()
{
    delete ui;
}


void levelCalibration::hideWindow(){
    hide();
}
