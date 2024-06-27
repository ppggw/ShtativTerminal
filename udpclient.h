#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QDataStream>

#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class UdpClient : public QObject
{
    Q_OBJECT
public:
    explicit UdpClient(QObject *parent = nullptr);


signals:
    void send_buffer(QByteArray, ushort);
    void send_to_calib(cv::Mat);

public slots:
    void readyRead();
    void readyRead_2();
    void SendUDP_Packet(QByteArray);

private:
    QUdpSocket *socket, *socket_2;
};

#endif // UDPCLIENT_H
