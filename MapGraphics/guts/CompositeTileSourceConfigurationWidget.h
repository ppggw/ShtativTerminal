#ifndef COMPOSITETILESOURCECONFIGURATIONWIDGET_H
#define COMPOSITETILESOURCECONFIGURATIONWIDGET_H

#include <QWidget>
#include <QWeakPointer>
#include <QListView>

#include "MapGraphics/MapGraphics_global.h"
#include "MapGraphics/tileSources/CompositeTileSource.h"

namespace Ui {
class CompositeTileSourceConfigurationWidget;
}

class MAPGRAPHICSSHARED_EXPORT CompositeTileSourceConfigurationWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit CompositeTileSourceConfigurationWidget(QWeakPointer<CompositeTileSource> composite = QWeakPointer<CompositeTileSource>(),
                                                    QWidget *parent = 0);
    ~CompositeTileSourceConfigurationWidget();

    void setComposite(QWeakPointer<CompositeTileSource> nComposite);

private slots:
    void handleCurrentSelectionChanged(QModelIndex,QModelIndex);
    void handleCompositeChange();
    void addOSMTileLayer();

    void on_removeSourceButton_clicked();

    void on_opacitySlider_valueChanged(int value);

    void on_moveDownButton_clicked();

    void on_moveUpButton_clicked();

    void on_listView_doubleClicked(const QModelIndex &index);

private:
    void init();
    Ui::CompositeTileSourceConfigurationWidget *ui;
    QWeakPointer<CompositeTileSource> _composite;
};

#endif // COMPOSITETILESOURCECONFIGURATIONWIDGET_H
