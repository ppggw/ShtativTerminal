#include "map.h"
#include "ui_map.h"


namespace{
    PlaneObject *DRONE = new PlaneObject(QImage("/home/shine/QtProjects/ShtativTerminal/drone.png"), QRectF(0, 0, 50, 50));
    PlaneObject *TRIPOD = new PlaneObject(QImage("/home/shine/QtProjects/ShtativTerminal/tripod.png"), QRectF(0, 0, 50, 50));
    PlaneObject *TRIANGLE = new PlaneObject(QImage("/home/shine/QtProjects/ShtativTerminal/triangle.png"), QRectF(0, 0, 111, 259));
    float angleNord = 0;
    QPointF coorTripod;
}


Map::Map(QWidget *parent) : QMainWindow(parent), ui(new Ui::Map)
{
    ui->setupUi(this);
    ui->DistLabel->setVisible(false);

    //The view will be our central widget
    this->setCentralWidget(view);

    //Setup some tile sources
    QSharedPointer<OSMTileSource> osmTiles(new OSMTileSource(OSMTileSource::OSMTiles), &QObject::deleteLater);
    QSharedPointer<GridTileSource> gridTiles(new GridTileSource(), &QObject::deleteLater);
    QSharedPointer<CompositeTileSource> composite(new CompositeTileSource(), &QObject::deleteLater);
    composite->addSourceBottom(osmTiles);
    composite->addSourceTop(gridTiles);
    view->setTileSource(composite);

    //Create a widget in the dock that lets us configure tile source layers
    CompositeTileSourceConfigurationWidget * tileConfigWidget = new CompositeTileSourceConfigurationWidget(composite.toWeakRef(),
                                                                                         this->ui->dockWidget);
    this->ui->dockWidget->setWidget(tileConfigWidget);
    delete this->ui->dockWidgetContents;

//    this->ui->menuWindow->addAction(this->ui->dockWidget->toggleViewAction());
    this->ui->dockWidget->toggleViewAction()->setText("&Layers");

    view->setZoomLevel(10);
    view->centerOn(35.878195,56.095246);

    // Create a circle on the map to demonstrate MapGraphicsObject a bit
    // The circle can be clicked/dragged around and should be ~5km in radius
    MapGraphicsObject * circle = new CircleObject(10, false, QColor(255, 0, 0, 100));
    circle->setLatitude(55.752490);
    circle->setLongitude(37.623205);
    scene->addObject(circle);
    tileConfigWidget->hide();

//    connect(timerDist, SIGNAL(timeout()), this, SLOT(onTimeotDist()));
//    timerDist->start(100);
}

Map::~Map()
{
    delete ui;
}

void Map::on_MerlinToCenter_clicked()
{

    if ( ui->MerlinToCenter->text() == "Штатив в центр")
    {
        MapEventMerlinOnCenter =1;
//        this->view->disable();
        ui->MerlinToCenter->setText("Сброс привязки");
    }
    else
    {
        if ( ui->MerlinToCenter->text() == "Сброс привязки")
        {
            MapEventMerlinOnCenter =0;
            ui->MerlinToCenter->setText("Штатив в центр");
            }
    }
}


void Map::setGpsTripod(QPointF coor){
    coorTripod = coor;

    TRIPOD->setPos(coor);
    TRIPOD->setRotation(180);
    view->scene()->addObject(TRIPOD);

    view->ComandPostPosition_Change = 0;
}

void Map::sendAngleNord(float angle){
    TRIANGLE->setRotation(angle);
}


void Map::enableRotateFieldOfView(){
    TRIANGLE->setPos(coorTripod);
    view->scene()->addObject(TRIANGLE);
}


void Map::drawDistance(int distance)
{
    if(!ui->DistLabel->isVisible()){ ui->DistLabel->setVisible(true); }

    QString str = " ДИСТАНЦИЯ ДО ЦЕЛИ: " + QString::number(int32_t(distance)) +(" м");
    ui->DistLabel->setText(str);
//      ui->DistLabel->setVisible(true);
  //    QColor BACK = QRgba(0,255,0,0);
//      ui->DistLabel->setStyleSheet("QLabel { background-color: rgb(255,255, 255) }");
}


void Map::drawDrone(QPointF coor){
    static bool first = true;
    DRONE->setPos(coor);
    if(first){
        first = !first;
        view->scene()->addObject(DRONE);
    }
}


