#include "levelcalibration.h"
#include "ui_levelcalibration.h"

levelCalibration::levelCalibration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::levelCalibration)
{
    ui->setupUi(this);
    setWindowTitle("Калибровка уровня штатива");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint);
    resize(512, 512);
    QPixmap icon(":/backgrounds/cs go donk 666 aim cool.png");
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Normal, QPalette::Window, QBrush(icon));
    pal.setBrush(QPalette::Inactive, QPalette::Window, QBrush(icon));
    setPalette(pal);
    setMask(icon.mask());

    QPushButton* closeBtn = new QPushButton("X", this);
    closeBtn->setFixedSize(16, 16);
    closeBtn->move(248, 0);
    closeBtn->setStyleSheet("background-color: red;");
    closeBtn->show();
    connect(closeBtn, SIGNAL(clicked(bool)), this, SLOT(hideWindow()));

//    repaint();
    hide();
}

levelCalibration::~levelCalibration()
{
    delete ui;
}


void levelCalibration::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap));
    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    painter.drawEllipse(pointPosition.x(), pointPosition.y(), 20, 20);
}


void levelCalibration::mousePressEvent(QMouseEvent *event)
{
    QPoint winPt  = this->pos();
    QPoint mousePt = event->globalPos();

    m_Position = mousePt - winPt;
}


void levelCalibration::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() - m_Position);
}


void levelCalibration::RepaintPointLevel(QPoint pointPosition_){
    pointPosition = pointPosition_;
    repaint();
}


void levelCalibration::hideWindow(){
    QByteArray ba;
    ba.resize(3);
    ba[0] = 0xfd;
    ba[1] = 0xdd;
    ba[2] = 0x01;
    emit onSendUDP_PacketToAirUnit(ba);

    hide();
}
