#ifndef LEVELCALIBRATION_H
#define LEVELCALIBRATION_H

#include <QWidget>
#include <QPixmap>
#include <QPalette>
#include <QBitmap>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <QByteArray>


namespace Ui {
class levelCalibration;
}

class levelCalibration : public QWidget
{
    Q_OBJECT

public:
    explicit levelCalibration(QWidget *parent = 0);
    ~levelCalibration();

private:
    Ui::levelCalibration *ui;
    QPoint m_Position;
    QPoint pointPosition{0, 0};

private slots:
    void hideWindow();

public slots:
    void RepaintPointLevel(QPoint);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);

signals:
    void onSendUDP_PacketToAirUnit(QByteArray);
};

#endif // LEVELCALIBRATION_H
