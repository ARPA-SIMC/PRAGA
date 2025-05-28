#-------------------------------------------------------
#
#   Point Statistics Widget library
#   This project is part of ARPAE agrolib distribution
#
#-------------------------------------------------------

QT  += widgets charts sql xml

TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release
CONFIG += c++14 c++17

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/pointStatisticsWidget
    } else {
        TARGET = release/pointStatisticsWidget
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/pointStatisticsWidget
    } else {
        TARGET = release/pointStatisticsWidget
    }
}
win32:{
    TARGET = pointStatisticsWidget
}

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis ../../agrolib/meteo  \
                ../../agrolib/utilities ../../agrolib/interpolation ../../agrolib/dbMeteoPoints    \
                ../../agrolib/dbMeteoGrid ../../agrolib/commonDialogs ../../agrolib/commonChartElements  \
                ../phenology ../climate


SOURCES += \
    dialogElaboration.cpp \
    pointStatisticsChartView.cpp \
    pointStatisticsWidget.cpp


HEADERS += \
    dialogElaboration.h \
    pointStatisticsChartView.h \
    pointStatisticsWidget.h 


