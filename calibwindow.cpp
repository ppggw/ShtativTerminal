#include "calibwindow.h"
#include "ui_calibwindow.h"


CalibWindow::CalibWindow(QWidget *parent): QDialog(parent), ui(new Ui::CalibWindow)
{
    ui->setupUi(this);
    this->setLayout(ui->gridLayout_2);

    ui->gridLayout_2->setMargin(10);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);
    ui->graphicsView->installEventFilter(this);

    ui->graphicsView_disp->setScene(new QGraphicsScene(this));
    ui->graphicsView_disp->scene()->addItem(&pixmap_disp);
    ui->graphicsView_disp->installEventFilter(this);    
}


CalibWindow::~CalibWindow()
{
    delete ui;
}

void CalibWindow::closeEvent(QCloseEvent *event)
{
    emit hided();
    event->accept();
}


void CalibWindow::draw(){
    int col = 1;
    for(size_t i=0; i != str_disp.size(); ++i){
        ui->textEdit_data->append("[" + QString::number(col) +"] " + "Разница = " + str_disp[i] + ", расстояние = " + str_distance[i]);
        col++;
    }
}

void CalibWindow::on_pushButton_pressed()
{
    if (ui->lineEdit_z->text() != "" && ui->lineEdit_disp->text() != ""){
        ui->textEdit_data->clear();

        // для вывода введенных данных
        if(ui->lineEdit_disp->text().toFloat() != 0 & ui->lineEdit_z->text().toFloat() != 0){
            str_disp.push_back(ui->lineEdit_disp->text());
            str_distance.push_back(ui->lineEdit_z->text());
            // для последующего расчета
            disp.push_back(ui->lineEdit_disp->text().toFloat());
            distance.push_back(ui->lineEdit_z->text().toFloat());
        }
        else{QMessageBox::warning(this, "Предупреждение", "Введите корректное значение разности и дальности");}

        draw();
        ui->lineEdit_z->clear();
        ui->lineEdit_disp->clear();
    }
}


void CalibWindow::on_pushBt_del_ind_pressed()
{
    if (ui->lineEdit_ind->text() != ""){
        int index = ui->lineEdit_ind->text().toInt() - 1;
        if (index <= str_disp.size()){
            str_disp.erase(str_disp.begin() + index);
            str_distance.erase(str_distance.begin() + index);
            disp.erase(disp.begin() + index);
            distance.erase(distance.begin() + index);

            ui->textEdit_data->clear();
            draw();
            ui->lineEdit_ind->clear();
        }
        else{
            QMessageBox::warning(this, "Предупреждение", "Вводите значения индекса из перечисленных в окне");
        }
    }
}

void CalibWindow::on_pushBt_del_all_pressed()
{
    str_disp.clear();
    str_distance.clear();
    disp.clear();
    distance.clear();

    ui->textEdit_data->clear();
}

bool CalibWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->graphicsView && (event->type() == QEvent::MouseButtonDblClick)) {
          // получение координат и пересчет для большого
          int frame_width = 1920;
          int frame_height = 1080;
          QPoint click;
          int dx = 50;

          QMouseEvent *pSrcMouseEvent = static_cast<QMouseEvent *>( event );
          click = pSrcMouseEvent->pos();
          float coef_x = float(frame_width) / ui->graphicsView->width();
          float coef_y = float(frame_height) / ui->graphicsView->height();
//          click_position.setX(int(click_position.x() * coef_x));
//          click_position.setY(int(click_position.y() *  coef_y));
          click_position.setX(int(click.x() * coef_x) - dx/2);
          click_position.setY(int(click.y() * coef_y) - dx/2);
          click_position.setWidth(dx);
          click_position.setHeight(dx);
          if(click_position.x() > 0 && click_position.y() >0){emit SendClickPositionCalib(click_position);}
          else{QMessageBox::warning(this, "Предупреждение", "Недопустимая область");}
       }

    if (event->type() == QEvent::KeyPress)
    {
        QByteArray ba;
        QDataStream stream(&ba, QIODevice::WriteOnly);
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        switch(keyEvent->key())
        {
            case Qt::Key_Up:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << (uchar)0x01 << (short)5 << (uchar)0x01;
//                qDebug() << "Right" << ba.toHex();
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_Down:
//                qDebug() << "Left";
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << (uchar)0x01 << (short)5 << (uchar)0x00;
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_Right:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << (uchar)0x02 << (short)50 << (uchar)0x00;
//                qDebug() << "Up" << ba.toHex();
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_Left:
//                qDebug() << "Down" << ba.toHex();
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x04 << (uchar)0x02 << (short)50 << (uchar)0x01;
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_1:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x01;
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_2:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x02;
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_3:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x03;
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_5:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x05;
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_7:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x07;
                emit onSendUDP_PacketToMain(ba);
                break;
            case Qt::Key_8:
                stream << (uchar)0xfa << (uchar)0xce << (uchar)0x02 << (uchar)0x00 << (uchar)0x09;
                emit onSendUDP_PacketToMain(ba);
                break;
        }
    }

    // false means it should be send to target also. as in , we dont remove it.
    // if you return true , you will take the event and widget never sees it so be carefull with that.
    return false;
}



void CalibWindow::on_calcBt_pressed()
{
    if (disp.empty() == false && distance.empty() == false){
        std::vector<float> disp_cop;
        std::copy(disp.begin(), disp.end(), back_inserter(disp_cop));

        float minElem = *std::min_element(disp_cop.begin(), disp_cop.end());
        float bias = minElem * 0.9;

        for(size_t i=0; i != disp_cop.size(); ++i){
            disp_cop[i] = 1 / (disp_cop[i] - bias);
        }
        cv::Mat z(distance);
        cv::Mat coef(disp_cop);
        cv::Mat coeff;
        cv::solve(coef, z, coeff, cv::DECOMP_QR);

        float disparity = coeff.at<float>(0);

        QMessageBox msgBox;
        msgBox.setWindowTitle("Успех");
        msgBox.setText("Калибровка закончена");
        msgBox.exec();

        emit SendDispBias(bias, disparity);
    }
}
