#ifndef PLANEOBJECT_H
#define PLANEOBJECT_H

#include "MapGraphics/MapGraphicsObject.h"

#include <QImage>

class PlaneObject : public MapGraphicsObject
{
    Q_OBJECT
public:
    explicit PlaneObject(const QImage& img,
                                const QRectF& sizeInMeters,
                                MapGraphicsObject *parent = 0);

    QPointF path3[10];
    int counter;
    //pure-virtual from MapGraphicsObject
    virtual QRectF boundingRect() const;

    //pure-virtual from MapGraphicsObject
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget=0);

signals:

public slots:
    //void setPlaneText(const QString, const QPointF &pos);
    void setPlane(const QImage& img, const QRectF& sizeInMeters);
    void setPlane(const QImage& img, const QRectF& sizeInMeters, const QPointF& pos);

private:

    QImage _img;
    QRectF _sizeInMeters;
    QVector<QPointF> path2;
//    QVector<QPointF> path3;

};

#endif // PLANEOBJECT_H
