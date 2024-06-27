#include "udpclient.h"

UdpClient::UdpClient(QObject *parent) : QObject(parent)
{
    // create a QUDP socket
    socket = new QUdpSocket(this);
    socket_2 = new QUdpSocket(this);
    socket->bind(QHostAddress("192.168.144.111"), 31580);
    socket_2->bind(QHostAddress("192.168.144.111"), 31581);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket_2, SIGNAL(readyRead()), this, SLOT(readyRead_2()));
}

void UdpClient::readyRead()
{
    QByteArray buffer;
    ushort length;
    buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    socket->readDatagram(buffer.data(), buffer.size(),
                         &sender, &senderPort);
    length = buffer.size();

//    qDebug() << "Message from: " << sender.toString();
//    qDebug() << "Message port: " << senderPort;
//    qDebug() << "Message: " << buffer;

    emit send_buffer(buffer, length);
}

void UdpClient::readyRead_2()
{
    // when data comes in
    QByteArray buffer;
    buffer.resize(socket_2->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    socket_2->readDatagram(buffer.data(), buffer.size(),
                         &sender, &senderPort);

//    qDebug() << "Message from: " << sender.toString();
//    qDebug() << "Message port: " << senderPort;
//    qDebug() << "Message: " << buffer;

    std::vector<uchar> bufferv(buffer.begin(), buffer.end());
    if (!bufferv.empty())
    {
        cv::Mat Output  =  cv::imdecode(bufferv , cv::IMREAD_COLOR);

        emit send_to_calib(Output);
    }
}

void UdpClient::SendUDP_Packet(QByteArray Data)
{
//    qDebug() << Data;
    socket->writeDatagram(Data, QHostAddress("192.168.144.25"), 31590);
}


















