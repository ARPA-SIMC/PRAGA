#-----------------------------------------------------
#
#   pragaProject library
#   contains project modules for PRAGA executable
#
#   This project is part of agrolib distribution
#
#-----------------------------------------------------

QT       += widgets charts sql xml
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release
CONFIG += c++11 c++14 c++17

DEFINES += _CRT_SECURE_NO_WARNINGS
DEFINES += NETCDF

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/pragaProject
    } else {
        TARGET = release/pragaProject
    }
}
win32:{
    TARGET = pragaProject
}

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/phenology ../../agrolib/meteo \
                ../../agrolib/gis ../../agrolib/drought ../../agrolib/interpolation ../../agrolib/solarRadiation \
                ../../agrolib/utilities ../../agrolib/outputPoints ../../agrolib/dbMeteoPoints ../../agrolib/dbMeteoGrid  \
                ../../agrolib/meteoWidget ../../agrolib/proxyWidget ../../agrolib/pointStatisticsWidget  \
                ../../agrolib/homogeneityWidget ../../agrolib/synchronicityWidget ../../agrolib/climate  \
                ../../agrolib/netcdfHandler ../../agrolib/graphics ../../agrolib/commonDialogs \
                ../../agrolib/commonChartElements ../../agrolib/inOutDataXML ../../agrolib/waterTable ../../agrolib/project \
                ../pragaDialogs


SOURCES += \
    dialogMeteoHourlyComputation.cpp \
    dialogPragaProject.cpp \
    dialogMeteoComputation.cpp \
    dialogPragaSettings.cpp \
    dialogAnomaly.cpp \
    pragaMeteoMaps.cpp \
    saveClimaLayout.cpp \
    pragaProject.cpp \
    pragaShell.cpp


HEADERS  += \
    dialogMeteoHourlyComputation.h \
    dialogPragaProject.h \
    dialogMeteoComputation.h \
    dialogPragaSettings.h \
    dialogAnomaly.h \
    pragaMeteoMaps.h \
    saveClimaLayout.h \
    pragaProject.h \
    pragaShell.h

