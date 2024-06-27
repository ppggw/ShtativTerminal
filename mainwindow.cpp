#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace{
    const uchar direct_x = 0x01;
    const uchar direct_y = 0x02;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ptimer_MW = new QTimer();
    ptimer_MW->setInterval(40);
    ptimer_MW->start();

    pipeline = "udpsrc port=31990 ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H265, "
               "payload=96 ! rtph265depay ! decodebin ! videoconvert ! appsink";

    ui->setupUi(this);
    QString filePath = QDir::currentPath();

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);
    ui->graphicsView->installEventFilter(this);

    ui->comboBox->addItem("Штатив");
    ui->comboBox->addItem("Видео из файла");

    ui->comboBox_neuro->addItem("BRIG");
    ui->comboBox_neuro->addItem("YOLOv4-TINY");
    ui->comboBox_neuro->addItem("YOLOv5-SMALL");
    ui->comboBox_neuro->addItem("YOLOv5-TINY");

    calibwindow = new CalibWindow();
    calibwindow->setWindowTitle("Калибровка расстояния");

    fullwindow = new FullWindow();
    fullwindow->setWindowState(Qt::WindowMaximized);
    fullwindow->setWindowIcon(QIcon(filePath + "/plane.png"));
    fullwindow->setWindowTitle("Ground Terminal");

    UDP_Command_AirUnit = new UdpClient();
    UDP_CommandThread = new QThread();
    UDP_Command_AirUnit->moveToThread(UDP_CommandThread);
    UDP_CommandThread->start();

    My_FrameUpdater = new FrameUpdater(&frame);
    My_FrameUpdaterThread = new QThread();
    My_FrameUpdater->moveToThread(My_FrameUpdaterThread);
    My_FrameUpdaterThread->start();

    map = new Map();
    map_Thread = new QThread();
    map->moveToThread(map_Thread);
    map_Thread->start();

    connect(ptimer_MW, SIGNAL(timeout()), this, SLOT(Timer_MW()));

    connect(fullwindow,SIGNAL(hided()), this, SLOT(show()));
    connect(fullwindow,SIGNAL(hided()), this, SLOT(hided_full()));


    connect(this, SIGNAL(onSendUDP_PacketToAirUnit(QByteArray)), UDP_Command_AirUnit, SLOT(SendUDP_Packet(QByteArray)));
    connect(this, SIGNAL(setGpsTripod(QPointF)), map, SLOT(setGpsTripod(QPointF)));
    connect(this, SIGNAL(sendAngleNord(float)), map, SLOT(sendAngleNord(float)));
    connect(this, SIGNAL(drawDrone(QPointF)), map, SLOT(drawDrone(QPointF)));
    connect(this, SIGNAL(enableRotateFieldOfView()), map, SLOT(enableRotateFieldOfView()));

    connect(UDP_Command_AirUnit, SIGNAL(send_buffer(QByteArray, ushort)), this, SLOT(UDPReady(QByteArray)));
    connect(My_FrameUpdater, SIGNAL(onSourceisAvailable()), this, SLOT(SourceIsAvailable()));

    connect(calibwindow, SIGNAL(onSendUDP_PacketToMain(QByteArray)), UDP_Command_AirUnit, SLOT(SendUDP_Packet(QByteArray)));

    connect(calibwindow,SIGNAL(hided()), this, SLOT(hided_calib()));
    connect(calibwindow, SIGNAL(SendClickPositionCalib(QRect)), this, SLOT(send_click_position_to_calib(QRect)));
    connect(UDP_Command_AirUnit, SIGNAL(send_to_calib(cv::Mat)), this, SLOT(draw_calib_frame(cv::Mat)));
    connect(calibwindow, SIGNAL(SendDispBias(float, float)), this, SLOT(send_bias_disp(float, float)));
    connect(timerMapEvent, SIGNAL(timeout()), this, SLOT(onMapEvent()));
    timerMapEvent->start(100);


    QString name = QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss");

    save_pipeline = "appsrc ! autovideoconvert ! x264enc ! h264parse ! qtmux ! "
                    "filesink location=D:/" + name.toStdString() + ".mkv ";
    ui->savefilepathLineEdit->setText("D:/");

    ui->neuroBtn->setDisabled(true);
    ui->tabWidget->widget(0)->setDisabled(true);

    frame = cv::Mat(ui->graphicsView->height(), ui->graphicsView->width(), CV_8UC3, cv::Scalar(255, 255, 255));
    // костыль. ПО высоте не выравнивается. Мб картинку, но ресурсы чисто для qt. Конвертирование дает проблемное изображение
}

MainWindow::~MainWindow()
{
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0xcc;
    ba[1] = 0xdd;
    ba[2] = 0x00;
    ba[3] = 0x00;
    emit onSendUDP_PacketToAirUnit(ba);

    delete ui;
}

void MainWindow::sendMessage(uchar first, uchar second, uchar third, uchar forth){
    QByteArray ba;
    ba.resize(4);
    ba[0] = first;
    ba[1] = second;
    ba[2] = third;
    ba[3] = forth;
    emit onSendUDP_PacketToAirUnit(ba);
}

void MainWindow::Timer_MW(){
    SourceIsAvailableCounter++;

    if (SourceIsAvailableCounter > 30 && !frame.empty())
    {
        if (EnableVideoFlag == true)
        {
            EnableVideoFlag = false;

            cv::Mat transparentImg(frame.size(), CV_8UC3, cv::Scalar(255, 255, 255));
            cv::addWeighted(transparentImg, 0.5, frame, 0.5 , 0.0, frame);
        }

        double scale = 0.1;
        double fontScale = cv::min(frame.cols,frame.rows)/(25/scale);

        cv::String text = "no connection";
        cv::Size textsize = cv::getTextSize(text, cv::FONT_HERSHEY_DUPLEX, fontScale, 2, 0);
        cv::Scalar color = cv::Scalar(0,0,0);

        cv::putText(frame,
                    text,
                    cv::Point(frame.cols / 2 - (textsize.width / 2), frame.rows / 2 + (textsize.height / 2)), //top-left position
                    cv::FONT_HERSHEY_DUPLEX,
                    fontScale,
                    color,
                    2);
        if(ui->groupBox_8->isEnabled()){
            ui->groupBox_8->setDisabled(true);
            ui->groupBox_7->setDisabled(true);
            ui->groupBox_3->setDisabled(true);
            ui->groupBox_4->setDisabled(true);
            ui->tabWidget->widget(0)->setDisabled(true);
            flow = false;
        }
    }

    if(!frame.empty())
    {
        if(calibwindow_flag == true && click_rect.x() != 0){
            cv::rectangle(frame, cv::Point(click_rect.x(), click_rect.y()),
                                 cv::Point(click_rect.x() + click_rect.width(), click_rect.y() + click_rect.height()), cv::Scalar(0, 0, 255), 2);
        }

        if(save_source.isOpened()) {save_source.write(frame);}

        QImage qimg(frame.data,
                    frame.cols,
                    frame.rows,
                    frame.step,
                    QImage::Format_RGB888);
//            pixmap->setPixmap( QPixmap::fromImage(qimg.rgbSwapped()) );
        if(calibwindow_flag == true)
        {
            calibwindow->pixmap.setPixmap(QPixmap::fromImage(qimg));
            calibwindow->ui->graphicsView->fitInView(&calibwindow->pixmap, Qt::IgnoreAspectRatio);
        }

        if(fullwindow_flag == true)
        {
            fullwindow->pixmap.setPixmap( QPixmap::fromImage(qimg) );
            fullwindow->ui->graphicsView->fitInView(&pixmap, Qt::IgnoreAspectRatio);
        }
        else{
            pixmap.setPixmap( QPixmap::fromImage(qimg) );
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
        }
    }
    qApp->processEvents();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if(source.isOpened())
    {
        QMessageBox::warning(this,
                             "Предупреждение",
                             "Остановите видеопоток перед закрытием приложения!");
        event->ignore();
    }
//    else
//    {
        if(source.isOpened() || flow == true){
            source.release();
            sendMessage(0xaa, 0xaa, 0x01, 0x02);
            flow = false;
            ptimer_MW->stop();
        }

        event->accept();
//    }
}


bool MainWindow::eventFilter(QObject *object, QEvent *event)
{

    if ( object == ui->graphicsView &&  ( event->type() == QEvent::MouseButtonDblClick )  ) {
        this->hide();
        fullwindow->show();
        fullwindow_flag = true;
    }

    if (object == ui->graphicsView &&  ( event->type() == QEvent::MouseButtonPress ) && source.isOpened()) {
          // получение координат и пересчет для большого
          QPoint click_position;
          QMouseEvent *pSrcMouseEvent = static_cast<QMouseEvent *>( event );
          click_position = pSrcMouseEvent->pos();
          float coef_x = float(frame.cols) / ui->graphicsView->width();
          float coef_y = float(frame.rows) / ui->graphicsView->height();
          click_position.setX(int(click_position.x() * coef_x));
          click_position.setY(int(click_position.y() *  coef_y));
          send_click_poisition(click_position);
       }

    if (event->type() == QEvent::KeyPress)
    {
        QByteArray ba;
        QDataStream stream(&ba, QIODevice::WriteOnly);
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        switch(keyEvent->key())
        {
            case Qt::Key_Up:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << direct_y << (short)40 << (uchar)0x01;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x08 << (uchar)0x00 << (uchar)0x20 << (uchar)0x29;
//                qDebug() << "Right" << ba.toHex();
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_Down:
//                qDebug() << "Left";
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << direct_y << (short)40 << (uchar)0x00;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x10 << (uchar)0x00 << (uchar)0x20 << (uchar)0x31;
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_Right:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << direct_x << (short)100 << (uchar)0x01;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x02 << (uchar)0x20 << (uchar)0x00 << (uchar)0x23;
//                qDebug() << "Up" << ba.toHex();
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_Left:
//                qDebug() << "Down" << ba.toHex();
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << direct_x << (short)100 << (uchar)0x00;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x04 << (uchar)0x20 << (uchar)0x00 << (uchar)0x25;
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_Control:
    //                qDebug() << "Down" << ba.toHex();
                    stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << (uchar)0x02 << (short)50 << (uchar)0x01;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x00 << (uchar)0x00 << (uchar)0x00 << (uchar)0x01;
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_1:
                QMessageBox::information(this, "", "Скорость двигателя: 1");
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x01;
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_2:
                QMessageBox::information(this, "", "Скорость двигателя: 2");
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x02;
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_3:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x03;
                emit onSendUDP_PacketToAirUnit(ba);
                QMessageBox::information(this, "", "Скорость двигателя: 3");
                break;
            case Qt::Key_5:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x05;
                emit onSendUDP_PacketToAirUnit(ba);
                QMessageBox::information(this, "", "Скорость двигателя: 4");
                break;
            case Qt::Key_7:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x07;
                emit onSendUDP_PacketToAirUnit(ba);
                QMessageBox::information(this, "", "Скорость двигателя: 5");
                break;
            case Qt::Key_9:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x09;
                emit onSendUDP_PacketToAirUnit(ba);
                QMessageBox::information(this, "", "Скорость двигателя: 9");
                break;
            case Qt::Key_0:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x00;
                emit onSendUDP_PacketToAirUnit(ba);
                QMessageBox::information(this, "", "Скорость двигателя: 0");
                break;
        }
    }

    return false;
}

void MainWindow::hided_full()
{
    fullwindow_flag = false;
//    ui->textEdit->append(QString::number(fullwindow_flag));
}

void MainWindow::hided_calib()
{
    calibwindow_flag = false;
    distance = -1;
//    ui->textEdit->append(QString::number(fullwindow_flag));
}

void MainWindow::SourceIsAvailable()
{
    SourceIsAvailableCounter = 0;
    EnableVideoFlag = true;
}

void MainWindow::UDPReady(QByteArray buf)
{
//    qDebug() << buf.toHex();
    switch((uchar)buf.at(0)){
        case 0xAA:{
            switch((uchar)buf.at(1)){
            case 0xAA:
            {
                if((uchar)buf.at(5) == 0x00){
                    sendDateToJetson();
                }
                switch((uchar)buf.at(3)){
                    case 0x00:
                    {
                        ui->saveairBtn->setText("Начать запись");
                        ui->saveairLineEdit->setText("Запись видео не идёт");
                        if (lamp1_flag == true) {ui->saveairLabel->setStyleSheet("background-color: green;"); lamp1_flag = false;}
                        else {ui->saveairLabel->setStyleSheet("background-color: white;"); lamp1_flag = true;}

                        break;
                    }
                    case 0x01:
                    {
                        ui->saveairBtn->setText("Остановить запись");
                        ui->saveairLineEdit->setText("Запись видео идёт");
                        if (lamp1_flag == true) {ui->saveairLabel->setStyleSheet("background-color: green;"); lamp1_flag = false;}
                        else {ui->saveairLabel->setStyleSheet("background-color: white;"); lamp1_flag = true;}

                        break;
                    }
                }
                switch((uchar)buf.at(4)){
                    case 0x01:
                    {
                        ui->groupBox_8->setEnabled(true);
                        ui->groupBox_7->setEnabled(true);
                        ui->groupBox_3->setEnabled(true);
                        ui->groupBox_4->setEnabled(true);
                        ui->tabWidget->widget(0)->setEnabled(true);
                        flow = true;
                        if(!My_FrameUpdater->source.isOpened()){My_FrameUpdater->OpenSource();}
                        break;
                    }
                }
            }
            break;
        }
        }
        break;

        case 0xBB:{
            writeCalibFrame(buf);
        }
        break;

        case 0xEE:{
            switch((uchar)buf.at(3)){
            case 0x02:
            {
                ui->engineBtn->setText("Выключить двигатели");
                ui->scanningBt->setText("Выключено");
                break;
            }
            case 0x05:
            {
                ui->neuroBtn->setEnabled(true);
                QMessageBox::information(this, "", "Успешная инициализация " + QString::fromStdString(net_name));

                break;
            }
            case 0x04:
            {
                ui->neuroBtn->setDisabled(true);
                ui->neuroBtn->setText("Включить нейросеть");
                break;
            }
            }
        }
        break;
        case 0xFF:{
        if((uchar)buf.at(1) == 0xAA){
            // сначал широта, потом долгота
                QDataStream receive(buf);
                uchar u;
                receive >> u;
                receive >> u;
                receive >> cor_GPS_drone_y;
                receive >> cor_GPS_drone_x;
                emit drawDrone(QPointF{cor_GPS_drone_y, cor_GPS_drone_x});
            }
        if((uchar)buf.at(1) == 0xFF){
            // сначал широта, потом долгота
                QDataStream receive(buf);
                uchar u;
                receive >> u;
                receive >> u;
                receive >> cor_GPS_y;
                receive >> cor_GPS_x;
                emit setGpsTripod(QPointF{cor_GPS_x, cor_GPS_y});

                QByteArray ba;
                ba.resize(4);
                ba[0] = 0xcc;
                ba[1] = 0xdd;
                ba[2] = 0x00;
                ba[3] = 0x00;
                emit onSendUDP_PacketToAirUnit(ba);
            }
        if((uchar)buf.at(1) == 0xDD){
            QDataStream receive(buf);
            uchar u;
            float angle_nord;
            receive >> u;
            receive >> u;
            receive >> angle_nord;

            emit sendAngleNord(angle_nord);
            }
        }
    }
}


void MainWindow::writeCalibFrame(QByteArray buf){
    QDataStream stream(buf);
    uchar preambule;
    stream >> preambule;
    stream >> preambule;
    stream >> disp;
    stream >> distance;
//    ui->textEdit->append(QString::fromStdString("Disparity = " + std::to_string(disp) + " Distance = " + std::to_string(distance)));
//            calibwindow->disparity_calib = disp;
    if(calibwindow_flag == true){calibwindow->ui->lineEdit_disp->setText(QString::number(disp));}
}


void MainWindow::draw_calib_frame(cv::Mat frame_calib){
    QImage qimg(frame_calib.data,
                frame_calib.cols,
                frame_calib.rows,
                frame_calib.step,
                QImage::Format_RGB888);
    calibwindow->pixmap_disp.setPixmap( QPixmap::fromImage(qimg) );
    calibwindow->ui->graphicsView_disp->fitInView(&calibwindow->pixmap_disp, Qt::IgnoreAspectRatio);
}


void MainWindow::on_saveairBtn_pressed()
{
    sendMessage(0xaa, 0xaa, 0x01, 0x01);
}

void MainWindow::on_initneuroBtn_pressed()
{
    uchar net;
    switch(ui->comboBox_neuro->currentIndex())
    {
    case 0:
        net_name = "yolo-brig";
        net = 0x00;
        break;
    case 1:
        net_name = "yolov4-tiny";
        net = 0x01;
        break;
    case 2:
        net_name = "yolov5_s";
        net = 0x02;
        break;
    case 3:
        net_name = "yolov5_tiny";
        net = 0x03;
        break;
    }
    sendMessage(0xee, 0xee, 0x00, net);
    ui->neuroBtn->setDisabled(true);
}


void MainWindow::on_filepathBtn_pressed()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Open file", "D:/");
    ui->filepathLineEdit->setText(fileName);
    pipeline = "filesrc location=" + fileName.toStdString() + " ! qtdemux ! decodebin ! videoconvert ! appsink";
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        pipeline = "udpsrc port=31990 ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H265, "
                   "payload=96 ! rtph265depay ! decodebin ! videoconvert ! appsink";
        ui->groupBox_3->setEnabled(false);
        ui->groupBox_4->setEnabled(false);
        ui->groupBox_7->setEnabled(false);
        break;
    case 1:
        pipeline = "filesrc location=" + ui->filepathLineEdit->text().toStdString() + " ! qtdemux ! decodebin ! videoconvert ! appsink";
        ui->groupBox_3->setEnabled(false);
        ui->groupBox_4->setEnabled(false);
        ui->groupBox_7->setEnabled(true);
        break;
    }
}

void MainWindow::on_savefilepathBtn_pressed()
{
    QString directoryName = QFileDialog::getExistingDirectory(this, "Open Directory", "D:/");
    ui->savefilepathLineEdit->setText(directoryName);
    QString name = QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss");

    save_pipeline = "appsrc ! autovideoconvert ! x264enc ! h264parse ! qtmux ! "
                    "filesink location=" + directoryName.toStdString() + "/" + name.toStdString() + ".mkv ";
}

void MainWindow::on_saveBtn_pressed()
{
    using namespace cv;

    if(save_source.isOpened())
        {
            ui->saveBtn->setText("Начать запись");
            save_source.release();
            ui->saveLabel->setStyleSheet("background-color: white;");
            return;
        }

    save_source.open(save_pipeline, cv::CAP_GSTREAMER, 30, cv::Size(source.get(cv::CAP_PROP_FRAME_WIDTH), source.get(cv::CAP_PROP_FRAME_HEIGHT)));
    ui->saveBtn->setText("Остановить запись");
    ui->saveLabel->setStyleSheet("background-color: green;");
}

void MainWindow::send_click_poisition(QPoint click_position)
{
    // нужно ли это, когда при самом клике делаются те же операции
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << (uchar)0xaa;
    stream << (uchar)0xbb;
    stream << click_position.x();
    stream << click_position.y();

    emit onSendUDP_PacketToAirUnit(ba);
}

void MainWindow::send_click_position_to_calib(QRect click)
{
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << (uchar)0xaa;
    stream << (uchar)0xcc;
    stream << click.x();
    stream << click.y();
    stream << click.width();
    stream << click.height();
    emit onSendUDP_PacketToAirUnit(ba);
}

void MainWindow::on_engineBtn_pressed()
{
    if (ui->engineBtn->text() == "Включить двигатели")
    {ui->engineBtn->setText("Выключить двигатели");}
    else {ui->engineBtn->setText("Включить двигатели");}
    sendMessage(0xaa, 0xaa, 0x01, 0x03);
}

void MainWindow::on_airvideoRadioBtn1_pressed()
{
    sendMessage(0xaa, 0xaa, 0x01, 0x05);
}

void MainWindow::on_airvideoRadioBtn2_pressed()
{
    sendMessage(0xaa, 0xaa, 0x01, 0x04);
}


void MainWindow::on_scanningBt_pressed()
{
//     повтороное нажатие приостанвливает цикл сканирования
//    if(net_name != ""){
//        if (ui->scanningBt->text() == "Сканирование вкл.") {ui->scanningBt->setText("Сканирование выкл.");}
//        else {ui->scanningBt->setText("Сканирование вкл.");}
//        sendMessage(0xaa, 0xaa, 0x01, 0x06);
//    }
//    else{
//        QMessageBox::warning(this,"Ошибка", "Включите нейросеть!");
//    }
    if (ui->scanningBt->text() == "Выключено"){
        ui->scanningBt->setText("Включено");
        ui->to_start->setEnabled(false);
    }
    else {
        ui->scanningBt->setText("Выключено");
        ui->to_start->setEnabled(true);
    }
    QByteArray ba;
    ba.resize(4);
    ba[0] = 0xaa;
    ba[1] = 0xaa;
    ba[2] = 0x01;
    ba[3] = 0x06;
    emit onSendUDP_PacketToAirUnit(ba);
}

void MainWindow::on_to_start_pressed()
{
    // полностью сбрасывает сканирование. Потом будет откатывать в 0 положение движков
    ui->scanningBt->setText("Выключено");
    sendMessage(0xaa, 0xaa, 0x01, 0x07);
}


void MainWindow::send_bias_disp(float bias, float disparity){
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << (uchar)0xaa;
    stream << (uchar)0xdd;
    stream << bias;
    stream << disparity;
    emit onSendUDP_PacketToAirUnit(ba);
}

void MainWindow::on_calibBt_pressed()
{
    if(calibwindow_flag != true){
        if(flow == true){
            calibwindow_flag = true;
            calibwindow->show();
        }
        else{
            QMessageBox::warning(this, "Предупреждение", "Для калибровки необходимо запустить поток");
        }
    }

//    calibwindow_flag = true;
//    calibwindow->show();
}

void MainWindow::on_neuroBtn_clicked()
{
    if (ui->neuroBtn->text() == "Включить нейросеть") {ui->neuroBtn->setText("Выключить нейросеть");}
    else {ui->neuroBtn->setText("Включить нейросеть");}
    sendMessage(0xee, 0xee, 0x01, 0x00);
}

void MainWindow::on_dtn_send_corners_clicked()
{
    int corn_x, corn_y;
    if (ui->lineEdit_corners_x->text() != "" && ui->lineEdit_corners_y->text() != ""){
        // для вывода введенных данных
        if(ui->lineEdit_corners_x->text().toInt() != 0 & ui->lineEdit_corners_y->text().toInt() != 0){
            corn_x = ui->lineEdit_corners_x->text().toInt();
            corn_y =  ui->lineEdit_corners_y->text().toInt();
        }
        else{QMessageBox::warning(this, "Предупреждение", "Введите корректное значение сектора сканирования");}

        QByteArray ba;
        QDataStream stream(&ba, QIODevice::WriteOnly);
        stream << (uchar)0xfe;
        stream << (uchar)0xfe;
        stream << corn_x;
        stream << corn_y;
        emit onSendUDP_PacketToAirUnit(ba);
        QMessageBox::information(this, "", "Введено новое поле сканирования");
    }
}

void MainWindow::on_setZero_clicked()
{
    sendMessage(0xaa, 0xaa, 0x01, 0x08);
    QMessageBox::information(this, "", "Введено нулевое направление штатива");
}


void MainWindow::on_pushButGPS_clicked()
{
    float cor_x, cor_y;
    if (ui->lineEdit_cor_x->text() != "" && ui->lineEdit_cor_y->text() != ""){
        // для вывода введенных данных
        if(ui->lineEdit_cor_x->text().toFloat() != 0 && ui->lineEdit_cor_y->text().toFloat() != 0){
            cor_x = ui->lineEdit_cor_x->text().toFloat();
            cor_y =  ui->lineEdit_cor_y->text().toFloat();
        }
        else{QMessageBox::warning(this, "Предупреждение", "Введите корректные координаты штатива");}

        QByteArray ba;
        QDataStream stream(&ba, QIODevice::WriteOnly);
        stream << (uchar)0xfe;
        stream << (uchar)0xee;
        stream << cor_x;
        stream << cor_y;
        emit onSendUDP_PacketToAirUnit(ba);

        QMessageBox::information(this, "", "Введены координаты штатива");
    }
}

void MainWindow::on_pushBut_GPS_Nord_clicked()
{
    sendMessage(0xaa, 0xaa, 0x01, 0x09);
    QMessageBox::information(this, "", "Введено направление штатива на север");
    emit enableRotateFieldOfView();
}

void MainWindow::on_radioButtonShowGPS_clicked()
{
    sendMessage(0xaa, 0xaa, 0x01, 0x010);
}

void MainWindow::sendDateToJetson(){
    QString m_date = QDateTime::currentDateTime().toString("dd_MM_yyyy");
    QString m_time = QDateTime::currentDateTime().toString("hh_mm_ss");
    int m_day, m_month, m_year, m_hour, m_minute, m_second;
    m_day = std::stoi(m_date.toStdString().substr(0, 2));
    m_month = std::stoi(m_date.toStdString().substr(3, 2));
    m_year = std::stoi(m_date.toStdString().substr(6, 4));

    m_hour = std::stoi(m_time.toStdString().substr(0, 2));
    m_minute = std::stoi(m_time.toStdString().substr(3, 2));
    m_second = std::stoi(m_time.toStdString().substr(6, 2));

    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << (uchar)0xDD;
    stream << (uchar)0xDD;
    stream << m_year;
    stream << m_month;
    stream << m_day;
    stream << m_hour;
    stream << m_minute;
    stream << m_second;

    emit onSendUDP_PacketToAirUnit(ba);
}

void MainWindow::on_pushBtn_Map_clicked()
{
    map->show();
}


void MainWindow::onMapEvent()
{
    if(map->MapEventMerlinOnCenter == 1)
    {
        map->view->centerOn(QPointF(cor_GPS_x, cor_GPS_y));
    }
}

