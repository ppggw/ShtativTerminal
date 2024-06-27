#ifndef FRAMEUPDATER_H
#define FRAMEUPDATER_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QMutex>

#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class FrameUpdater : public QObject
{
    Q_OBJECT
public:
    explicit FrameUpdater(cv::Mat* frame, QObject *parent = nullptr);
    cv::Mat *my_frame;
    cv::Mat temp_frame;
    QTimer *pTimer_MW;
    QMutex	m_mutex;

    cv::VideoCapture source;
    cv::String pipeline;

    bool SourceIsAvailable;

public slots:
    void Timer_MW_timeout(void);
    void OpenSource(void);

signals:
    void onSourceisAvailable();

};

#endif // FRAMEUPDATER_H
