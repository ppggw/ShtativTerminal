#include "PlaneObject.h"
#include "MapGraphics/guts/Conversions.h"

#include <QDebug>

//QVector<QPointF> PlaneObject::path3;

PlaneObject::PlaneObject(const QImage &img,
                                       const QRectF &sizeInMeters,
                                       MapGraphicsObject *parent) :
    MapGraphicsObject(true, parent), _img(img)
{
    this->setPlane(img, sizeInMeters);
}

//pure-virtual from MapGraphicsObject
QRectF PlaneObject::boundingRect() const
{
    return _sizeInMeters;
}

//pure-virtual from MapGraphicsObject
void PlaneObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen pen = painter->pen();
    pen.setWidthF(1);
    painter->setPen(pen);

    painter->drawImage(this->boundingRect(), _img);
}

//public slot
void PlaneObject::setPlane(const QImage &img, const QRectF &sizeInMeters)
{
    _img = img;

    const qreal width = sizeInMeters.width();
    const qreal height = sizeInMeters.height();
    _sizeInMeters = QRectF(-0.5 * width,
                           -0.5 * height,
                           width, height);

    this->redrawRequested();
}

//public slot
void PlaneObject::setPlane(const QImage &img,
                                    const QRectF &sizeInMeters,
                                    const QPointF &pos)
{
    path2.append(this->pos());
    this->setPlane(img, sizeInMeters);
    this->setPos(pos);;
}


//void PlaneObject::setPlaneText(QString biba,const QPointF &pos)
//{

//    QString text = "QInputDialog";
//      QGraphicsTextItem *textItem = this->scene()->addText(text);
//      textItem->setPos(mapToScene(pos));
////QImage image("sample.png");
////QPainter p(&image);
////p.setPen(QPen(Qt::red));
////p.setFont(QFont("Times", 12, QFont::Bold));
////p.drawText(image.rect(), Qt::AlignCenter, "Text");
////path2.append(this->pos());
////this->setPlane(image, QRectF(0, 0, 100, 100));
////this->setPos(pos);;

//}
