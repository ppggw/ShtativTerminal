#ifndef LEVELCALIBRATION_H
#define LEVELCALIBRATION_H

#include <QWidget>
#include <QPixmap>
#include <QPalette>
#include <QBitmap>
#include <QPushButton>


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

private slots:
    void hideWindow();
};

#endif // LEVELCALIBRATION_H
