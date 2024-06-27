#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QDir>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QTime>
#include <QTimer>
#include <QThread>

#include <iostream>
#include <typeinfo>
#include <queue>
#include <iterator>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <chrono>

#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "fullwindow.h"
#include "ui_fullwindow.h"
#include "udpclient.h"
#include "calibwindow.h"
#include "ui_calibwindow.h"
#include "frameupdater.h"
#include "map.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    FullWindow *fullwindow;
    bool fullwindow_flag = false;

    CalibWindow *calibwindow;
    bool calibwindow_flag = false;

    cv::Mat post_process(cv::Mat &, std::vector<cv::Mat> &, const std::vector<std::string> &);

    UdpClient *UDP_Command_AirUnit;
    QThread *UDP_CommandThread;
    int disp;
    int distance = -1;

    void sendDateToJetson();

protected:
    void closeEvent(QCloseEvent *event);
//    void mouseDoubleClickEvent(QMouseEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    void sendMessage(uchar, uchar, uchar, uchar);

private slots:
    void on_initneuroBtn_pressed();
    void on_saveairBtn_pressed();
    void on_saveBtn_pressed();
    void on_savefilepathBtn_pressed();
    void on_filepathBtn_pressed();
    void on_engineBtn_pressed();
    void on_airvideoRadioBtn1_pressed();
    void on_airvideoRadioBtn2_pressed();

    void hided_full();
    void hided_calib();
    void on_comboBox_currentIndexChanged(int);
    void send_click_poisition(QPoint);
    void send_click_position_to_calib(QRect);
    void on_scanningBt_pressed();
    void on_to_start_pressed();
    void on_calibBt_pressed();
    void writeCalibFrame(QByteArray buf);
    void draw_calib_frame(cv::Mat);
    void send_bias_disp(float, float);

    void on_neuroBtn_clicked();
    void Timer_MW();
    void on_dtn_send_corners_clicked();
    void on_setZero_clicked();
    void on_pushButGPS_clicked();
    void on_pushBut_GPS_Nord_clicked();
    void on_radioButtonShowGPS_clicked();
    void on_pushBtn_Map_clicked();
    void onMapEvent();

private:
    Ui::MainWindow *ui;
    QTimer *ptimer_MW;
    FrameUpdater *My_FrameUpdater;
    QThread* My_FrameUpdaterThread, *map_Thread;
    Map* map;

    QTimer* timerMapEvent    = new QTimer(this);

    QGraphicsPixmapItem pixmap;
    cv::VideoCapture source;
    cv::VideoWriter save_source;
    cv::String pipeline, save_pipeline;

    cv::dnn::DetectionModel model;
    std::vector<std::string> class_list;
    std::vector<cv::Scalar> colors;
    std::string net_name;
    cv::Mat frame;
    QRect click_rect;

    bool lamp1_flag, lamp2_flag;
    bool flow = false;

    float cor_GPS_x = 0, cor_GPS_y = 0;
    float cor_GPS_drone_x =0, cor_GPS_drone_y = 0;
    int SourceIsAvailableCounter = 0;
    bool EnableVideoFlag = false;


signals:
    void onShowFullWindow();
    void onInitUDP_Command();
    void onSendUDP_PacketToAirUnit(QByteArray);
    void onUDPReady(QByteArray buf);
    void setGpsTripod(QPointF);
    void sendAngleNord(float);
    void enableRotateFieldOfView();
    void drawDrone(QPointF);

public slots:
    void UDPReady(QByteArray buf);
    void SourceIsAvailable();
};
#endif // MAINWINDOW_H