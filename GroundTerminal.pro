QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    MapGraphics/guts/CompositeTileSourceConfigurationWidget.cpp \
    MapGraphics/guts/Conversions.cpp \
    MapGraphics/guts/MapGraphicsNetwork.cpp \
    MapGraphics/guts/MapTileGraphicsObject.cpp \
    MapGraphics/guts/MapTileLayerListModel.cpp \
    MapGraphics/guts/MapTileSourceDelegate.cpp \
    MapGraphics/guts/PrivateQGraphicsInfoSource.cpp \
    MapGraphics/guts/PrivateQGraphicsObject.cpp \
    MapGraphics/guts/PrivateQGraphicsScene.cpp \
    MapGraphics/guts/PrivateQGraphicsView.cpp \
    MapGraphics/tileSources/CompositeTileSource.cpp \
    MapGraphics/tileSources/GridTileSource.cpp \
    MapGraphics/tileSources/OSMTileSource.cpp \
    MapGraphics/CircleObject.cpp \
    MapGraphics/LineObject.cpp \
    MapGraphics/MapGraphicsObject.cpp \
    MapGraphics/MapGraphicsScene.cpp \
    MapGraphics/MapGraphicsView.cpp \
    MapGraphics/MapTileSource.cpp \
    MapGraphics/PolygonObject.cpp \
    MapGraphics/Position.cpp \
    calibwindow.cpp \
    frameupdater.cpp \
    fullwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    udpclient.cpp \
    map.cpp \
    PlaneObject.cpp \
    levelcalibration.cpp

HEADERS += \
    MapGraphics/guts/CompositeTileSourceConfigurationWidget.h \
    MapGraphics/guts/Conversions.h \
    MapGraphics/guts/MapGraphicsNetwork.h \
    MapGraphics/guts/MapTileGraphicsObject.h \
    MapGraphics/guts/MapTileLayerListModel.h \
    MapGraphics/guts/MapTileSourceDelegate.h \
    MapGraphics/guts/PrivateQGraphicsInfoSource.h \
    MapGraphics/guts/PrivateQGraphicsObject.h \
    MapGraphics/guts/PrivateQGraphicsScene.h \
    MapGraphics/guts/PrivateQGraphicsView.h \
    MapGraphics/tileSources/CompositeTileSource.h \
    MapGraphics/tileSources/GridTileSource.h \
    MapGraphics/tileSources/OSMTileSource.h \
    MapGraphics/CircleObject.h \
    MapGraphics/LineObject.h \
    MapGraphics/MapGraphics_global.h \
    MapGraphics/MapGraphicsObject.h \
    MapGraphics/MapGraphicsScene.h \
    MapGraphics/MapGraphicsView.h \
    MapGraphics/MapTileSource.h \
    MapGraphics/PolygonObject.h \
    MapGraphics/Position.h \
    calibwindow.h \
    frameupdater.h \
    fullwindow.h \
    mainwindow.h \
    udpclient.h \
    map.h \
    PlaneObject.h \
    levelcalibration.h

FORMS += \
    calibwindow.ui \
    fullwindow.ui \
    mainwindow.ui \
    MapGraphics/guts/CompositeTileSourceConfigurationWidget.ui \
    map.ui \
    levelcalibration.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += /home/shine/opencv-build/include/opencv4
LIBS += -L/home/shine/opencv-build/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_imgcodecs -lopencv_videoio -lopencv_dnn

RESOURCES += Images.qrc
RESOURCES += MapGraphics/resources.qrc

DISTFILES += \
    config_files/yolo-brig.cfg \
    config_files/yolo-brig.txt \
    config_files/yolo-brig.weights \
    config_files/yolov4-tiny.cfg \
    config_files/yolov4-tiny.weights \
    config_files/yolov5_s.onnx \
    config_files/yolov5_tiny.onnx \
    MapGraphics/libMapGraphics.so.1 \
    MapGraphics/libMapGraphics.so.1.0 \
    MapGraphics/debug/libMapGraphics.a \
    MapGraphics/release/libMapGraphics.a \
    MapGraphics/debug/MapGraphics.dll \
    MapGraphics/release/MapGraphics.dll \
    MapGraphics/libMapGraphics.so \
    MapGraphics/libMapGraphics.so.1.0.0 \
    MapGraphics/images/edit_add.png \
    MapGraphics/images/editdelete.png \
    MapGraphics/object_script.MapGraphics.Debug \
    MapGraphics/object_script.MapGraphics.Release \
    MapGraphics/images/edit_add.png \
    MapGraphics/images/editdelete.png
