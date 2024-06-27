#include "fullwindow.h"
#include "ui_fullwindow.h"

FullWindow::FullWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FullWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);
    ui->graphicsView->installEventFilter(this);
}

FullWindow::~FullWindow()
{
    delete ui;
}

void FullWindow::closeEvent(QCloseEvent *event)
{
    emit hided();
    event->accept();
}

