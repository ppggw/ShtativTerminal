#ifndef CALIBWINDOW_H
#define CALIBWINDOW_H

#include <QWidget>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>
#include <QString>

#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <vector>

//#include "ui_calibwindow.h"


namespace Ui {
class CalibWindow;
}



class CalibWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CalibWindow(QWidget *parent = nullptr);
    ~CalibWindow();
    Ui::CalibWindow *ui;

    QGraphicsPixmapItem pixmap, pixmap_disp;
    std::vector<float> disp, distance;
    std::vector<QString> str_disp, str_distance;

//    QPoint click_position;
    QRect click_position;
//    cv::Mat frame;
//    cv::Mat frame_for_disp;

private slots:
    void on_pushButton_pressed();
    void on_pushBt_del_ind_pressed();
    void on_pushBt_del_all_pressed();


    void on_calcBt_pressed();

    void on_SwitchDistanceBtn_clicked();

private:
    void draw();

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void closeEvent(QCloseEvent *);

signals:
    void onSendUDP_PacketToMain(QByteArray);
    void hided();
    void SendClickPositionCalib(QRect);
    void SendDispBias(float, float);
};

#endif // CALIBWINDOW_H
