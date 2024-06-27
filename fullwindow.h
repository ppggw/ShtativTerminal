#ifndef FULLWINDOW_H
#define FULLWINDOW_H

#include <QWidget>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>

#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

namespace Ui {
class FullWindow;
}

class FullWindow : public QWidget
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event);

public:
    explicit FullWindow(QWidget *parent = nullptr);
    ~FullWindow();
    Ui::FullWindow *ui;

    QGraphicsPixmapItem pixmap;

private:
//    Ui::FullWindow *ui;

signals:
    void hided();
};

#endif // FULLWINDOW_H
