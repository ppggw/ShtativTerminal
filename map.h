#ifndef MAP_H
#define MAP_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include "MapGraphics/MapGraphicsView.h"
#include "MapGraphics/MapGraphicsScene.h"
#include "MapGraphics/tileSources/GridTileSource.h"
#include "MapGraphics/tileSources/OSMTileSource.h"
#include "MapGraphics/tileSources/CompositeTileSource.h"
#include "MapGraphics/guts/CompositeTileSourceConfigurationWidget.h"
#include "MapGraphics/CircleObject.h"
#include "MapGraphics/PolygonObject.h"
#include <QTimer>

#include <QSharedPointer>
#include <QtDebug>
#include <QThread>
#include <QImage>
#include <QAction>

#include "PlaneObject.h"


namespace Ui {
class Map;
}

class Map : public QMainWindow
{
    Q_OBJECT

public:
    explicit Map(QWidget *parent = 0);
    ~Map();
    MapGraphicsScene * scene = new MapGraphicsScene(this);
    MapGraphicsView * view = new MapGraphicsView(scene,this);
    uint8_t MapEventMerlinOnCenter,MapEventHideInfo,MapMission;
    QTimer* timerDist      = new QTimer(this);
    uint32_t dist_to_map;


private slots:
    void on_MerlinToCenter_clicked();
    void onTimeotDist();

public slots:
    void setGpsTripod(QPointF);
    void sendAngleNord(float);
    void enableRotateFieldOfView();
    void drawDrone(QPointF);

private:
    Ui::Map *ui;
};

#endif // MAP_H
