#----------------------------------------------------
#
#   Synchronicity Widget library
#   This project is part of ARPAE PRAGA distribution
#
#----------------------------------------------------

QT  += widgets charts sql xml

TEMPLATE = lib
CONFIG += staticlib

CONFIG += debug_and_release
CONFIG += c++11 c++14 c++17

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/synchronicityWidget
    } else {
        TARGET = release/synchronicityWidget
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/synchronicityWidget
    } else {
        TARGET = release/synchronicityWidget
    }
}
win32:{
    TARGET = synchronicityWidget
}

INCLUDEPATH += ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis  \
                ../../agrolib/meteo ../../agrolib/utilities ../../agrolib/dbMeteoPoints \
                ../../agrolib/dbMeteoGrid  ../../agrolib/commonDialogs  \
                ../../agrolib/commonChartElements ../../agrolib/interpolation \
                ../phenology ../climate

SOURCES += \
    interpolationChartView.cpp \
    synchronicityChartView.cpp \
    synchronicityWidget.cpp


HEADERS += \
    interpolationChartView.h \
    synchronicityChartView.h \
    synchronicityWidget.h 

