#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace{
    const uchar direct_x = 0x01;
    const uchar direct_y = 0x02;
}


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    qApp->installEventFilter(this);
    names = {
            {"saveairBtn", {"Начать запись", "Остановить запись"}},
            {"neuroBtn", {"Включить нейросеть", "Выключить нейросеть"}},
            {"engineBtn", {"Включить слежение", "Выключить слежение"}},
            {"scanningBt", {"Включить", "Выключить"}}
        };

    ptimer_MW = new QTimer();
    ptimer_MW->setInterval(40);
    ptimer_MW->start();

    pipeline = "udpsrc port=31990 ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H265, "
               "payload=96 ! rtph265depay ! decodebin ! videoconvert ! appsink";

    ui->setupUi(this);
    QString filePath = QDir::currentPath();

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

    ui->comboBox->addItem("Штатив");
    ui->comboBox->addItem("Видео из файла");

    ui->comboBox_neuro->addItem("YOLOv5-SMALL");
    ui->comboBox_neuro->addItem("YOLOv5-TINY");

    calibwindow = new CalibWindow();
    calibwindow->setWindowTitle("Калибровка расстояния");

    levelCalibrationWindow = new levelCalibration();

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

    QList<QString> intList;
    intList << "0" << "1" << "2" << "3" << "5" << "7" << "9";
    ui->comboBoxSpeed->addItems(intList);

    QMovie* mo = new QMovie();
    mo->setFileName("/home/shine/QtProjects/ShtativTerminal/load_neuro.gif");
    mo->start();
    ui->movieLabel->setMovie(mo);
    ui->movieLabel->hide();

    connect(ptimer_MW, SIGNAL(timeout()), this, SLOT(Timer_MW()));

    connect(fullwindow,SIGNAL(hided()), this, SLOT(show()));
    connect(fullwindow,SIGNAL(hided()), this, SLOT(hided_full()));

    connect(this, SIGNAL(onSendUDP_PacketToAirUnit(QByteArray)), UDP_Command_AirUnit, SLOT(SendUDP_Packet(QByteArray)));
    connect(this, SIGNAL(setGpsTripod(QPointF)), map, SLOT(setGpsTripod(QPointF)));
    connect(this, SIGNAL(sendAngleNord(float)), map, SLOT(sendAngleNord(float)));
    connect(this, SIGNAL(drawDrone(QPointF)), map, SLOT(drawDrone(QPointF)));
    connect(this, SIGNAL(setCalibeGPSTripod(QPointF)), calibwindow, SLOT(setCalibeGPSTripod(QPointF)));
    connect(this, SIGNAL(enableRotateFieldOfView()), map, SLOT(enableRotateFieldOfView()));
    connect(this, SIGNAL(drawDistance(int)), map, SLOT(drawDistance(int)));
    connect(this, SIGNAL(RepaintPointLevel(QPoint)), levelCalibrationWindow, SLOT(RepaintPointLevel(QPoint)));
    connect(this, SIGNAL(lostConnectionSignal()), SLOT(lostConnection()));

    connect(ui->comboBoxSpeed, SIGNAL(currentTextChanged(QString)), this, SLOT(changeSpeed(QString)));

    connect(UDP_Command_AirUnit, SIGNAL(send_buffer(QByteArray, ushort)), this, SLOT(UDPReady(QByteArray)));
    connect(My_FrameUpdater, SIGNAL(onSourceisAvailable()), this, SLOT(SourceIsAvailable()));

    connect(levelCalibrationWindow, SIGNAL(onSendUDP_PacketToAirUnit(QByteArray)), UDP_Command_AirUnit, SLOT(SendUDP_Packet(QByteArray)));

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
    ui->comboBoxSpeed->installEventFilter(this);
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
        emit lostConnectionSignal();
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

    if (object == ui->graphicsView &&  ( event->type() == QEvent::MouseButtonPress )) {
          // получение координат и пересчет для большого
          QPoint click_position;
          QMouseEvent *pSrcMouseEvent = static_cast<QMouseEvent *>( event );
          click_position = pSrcMouseEvent->pos();
          float coef_x = ( float(frame.cols) * 1920 )/( ui->graphicsView->width() * 1280 );
          float coef_y = ( float(frame.rows) * 1080 )/( ui->graphicsView->height() * 720 );
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
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << direct_y << (short)80 << (uchar)0x01;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x08 << (uchar)0x00 << (uchar)0x20 << (uchar)0x29;
//                qDebug() << "Right" << ba.toHex();
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_Down:
//                qDebug() << "Left";
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << direct_y << (short)80 << (uchar)0x00;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x10 << (uchar)0x00 << (uchar)0x20 << (uchar)0x31;
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_Right:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << direct_x << (short)80 << (uchar)0x01;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x02 << (uchar)0x20 << (uchar)0x00 << (uchar)0x23;
//                qDebug() << "Up" << ba.toHex();
                emit onSendUDP_PacketToAirUnit(ba);
                break;
            case Qt::Key_Left:
//                qDebug() << "Down" << ba.toHex();
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << direct_x << (short)80 << (uchar)0x00;
//                stream << (uchar)0xff << (uchar)0x01 << (uchar)0x00 << (uchar)0x04 << (uchar)0x20 << (uchar)0x00 << (uchar)0x25;
                emit onSendUDP_PacketToAirUnit(ba);
                break;
        }
    }
    if (event->type() == QEvent::KeyPress && object == ui->comboBoxSpeed)
    {
        return true;
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
                        ui->saveairBtn->setText(names["saveairBtn"].first);
                        break;
                    }
                    case 0x01:
                    {
                        ui->saveairBtn->setText(names["saveairBtn"].second);

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
                ui->engineBtn->setText(names["engineBtn"].second);
                ui->scanningBt->setText(names["scanningBt"].first);
                break;
            }
            case 0x05:
            {
                ui->movieLabel->hide();
                ui->neuroBtn->setEnabled(true);
                QMessageBox::information(this, "", "Успешная инициализация " + QString::fromStdString(net_name));

                break;
            }
            case 0x04:
            {
                ui->neuroBtn->setDisabled(true);
                ui->neuroBtn->setText(names["neuroBtn"].first);
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
                receive >> gps.drone_y;
                receive >> gps.drone_x;
                emit drawDrone(QPointF{gps.drone_x, gps.drone_y});

                int distance;
                receive >> distance;
                emit drawDistance(distance);
            }
        if((uchar)buf.at(1) == 0xFF){
            // сначал широта, потом долгота
                QDataStream receive(buf);
                uchar u;
                receive >> u;
                receive >> u;
                receive >> gps.shtativ_y;
                receive >> gps.shtativ_x;
                emit setGpsTripod(QPointF{gps.shtativ_x, gps.shtativ_y});
                emit setCalibeGPSTripod(QPointF{gps.shtativ_x, gps.shtativ_y});

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
        net_name = "yolov5_s";
        net = 0x02;
        break;
    case 1:
        net_name = "yolov5_tiny";
        net = 0x03;
        break;
    }
    sendMessage(0xee, 0xee, 0x00, net);
    ui->neuroBtn->setDisabled(true);

    ui->movieLabel->show();
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
    if (ui->engineBtn->text() == names["engineBtn"].first)
    {
        ui->engineBtn->setText(names["engineBtn"].second);
        ui->engineBtn->setStyleSheet("background-color: green;");
    }
    else {
        ui->engineBtn->setText(names["engineBtn"].first);
        ui->engineBtn->setStyleSheet("background-color: rgb(238, 238, 238)");
    }
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
    if (ui->scanningBt->text() == names["scanningBt"].first){
        ui->scanningBt->setText(names["scanningBt"].second);
        ui->to_start->setEnabled(false);
    }
    else {
        ui->scanningBt->setText(names["scanningBt"].first);
        ui->to_start->setEnabled(true);
    }
    sendMessage(0xaa, 0xaa, 0x01, 0x06);
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
    if (ui->neuroBtn->text() == names["neuroBtn"].first) {
        ui->neuroBtn->setText(names["neuroBtn"].second);
        ui->neuroBtn->setStyleSheet("background-color: green");
    }
    else {
        ui->neuroBtn->setText(names["neuroBtn"].first);
        ui->neuroBtn->setStyleSheet("background-color: rgb(238, 238, 238)");
    }
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
        map->view->centerOn(QPointF(gps.shtativ_x, gps.shtativ_y));
    }
}


void MainWindow::on_pushButton_clicked()
{
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << (uchar)0xFD;
    stream << (uchar)0xDD;
    stream << 0x01;

    emit onSendUDP_PacketToAirUnit(ba);

    levelCalibrationWindow->show();
}


void MainWindow::changeSpeed(QString speed){
    uchar HexSpeed = std::stoi(speed.toStdString(), 0, 16);

    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << HexSpeed;
    emit onSendUDP_PacketToAirUnit(ba);
    QMessageBox::information(this, "", "Скорость двигателя: " + speed);
    ui->comboBoxSpeed->setFocus(Qt::OtherFocusReason);
}


void MainWindow::lostConnection(){
    ui->neuroBtn->setStyleSheet("background-color: rgb(238, 238, 238)");
    ui->neuroBtn->setDisabled(true);
    ui->neuroBtn->setText(names["neuroBtn"].first);

    ui->engineBtn->setStyleSheet("background-color: rgb(238, 238, 238)");
    ui->engineBtn->setText(names["engineBtn"].first);
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    if(checked){
        sendMessage(0xee, 0xee, 0x02, 0x00);
    }
    else{
        sendMessage(0xee, 0xee, 0x02, 0x01);
    }
}



void MainWindow::on_checkBox_Korel_clicked(bool checked)
{
    if(checked){
        sendMessage(0xee, 0xee, 0x03, 0x00);
    }
    else{
        sendMessage(0xee, 0xee, 0x03, 0x01);
    }
}
