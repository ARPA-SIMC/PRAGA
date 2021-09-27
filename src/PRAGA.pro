#-----------------------------------------------------
#
#   PRAGA
#   PRogram for AGrometeorological Analysis
#
#-----------------------------------------------------

QT       += core gui widgets charts network sql xml

TARGET = PRAGA
TEMPLATE = app

INCLUDEPATH +=  ../mapGraphics \
                ../agrolib/crit3dDate ../agrolib/mathFunctions ../agrolib/meteo ../agrolib/gis  \
                ../agrolib/drought ../agrolib/interpolation ../agrolib/solarRadiation ../agrolib/utilities  \
                ../agrolib/dbMeteoPoints ../agrolib/dbMeteoGrid ../agrolib/meteoWidget ../agrolib/phenology ../agrolib/climate \
                ../agrolib/netcdfHandler  ../agrolib/graphics ../agrolib/commonDialogs ../agrolib/importDataXML ../agrolib/project

CONFIG += debug_and_release

QMAKE_CXXFLAGS += -std=c++11

DEFINES += NETCDF


CONFIG(debug, debug|release) {
    LIBS += -L../agrolib/project/debug -lproject
    LIBS += -L../agrolib/commonDialogs/debug -lcommonDialogs
    LIBS += -L../agrolib/climate/debug -lclimate
    LIBS += -L../agrolib/netcdfHandler/debug -lnetcdfHandler
    win32:{
        LIBS += -L../mapGraphics/debug -lMapGraphics
        LIBS += -L$$(NC4_INSTALL_DIR)/lib -lnetcdf
    }
    unix:{
        LIBS += -L../mapGraphics/release -lMapGraphics
        LIBS += -lnetcdf
    }
    macx:{
        LIBS += -L/usr/local/lib/ -lnetcdf
    }
    LIBS += -L../agrolib/phenology/debug -lphenology
    LIBS += -L../agrolib/importDataXML/debug -limportDataXML
    LIBS += -L../agrolib/meteoWidget/debug -lmeteoWidget
    LIBS += -L../agrolib/dbMeteoGrid/debug -ldbMeteoGrid
    LIBS += -L../agrolib/dbMeteoPoints/debug -ldbMeteoPoints
    LIBS += -L../agrolib/utilities/debug -lutilities
    LIBS += -L../agrolib/solarRadiation/debug -lsolarRadiation
    LIBS += -L../agrolib/interpolation/debug -linterpolation
    LIBS += -L../agrolib/drought/debug -ldrought
    LIBS += -L../agrolib/meteo/debug -lmeteo
    LIBS += -L../agrolib/gis/debug -lgis
    LIBS += -L../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../agrolib/mathFunctions/debug -lmathFunctions

} else {
    LIBS += -L../mapGraphics/release -lMapGraphics
    LIBS += -L../agrolib/project/release -lproject
    LIBS += -L../agrolib/commonDialogs/release -lcommonDialogs
    LIBS += -L../agrolib/climate/release -lclimate
    LIBS += -L../agrolib/netcdfHandler/release -lnetcdfHandler
    win32:{
        LIBS += -L$$(NC4_INSTALL_DIR)/lib -lnetcdf
    }
    unix:{
        LIBS += -lnetcdf
    }
    macx:{
        LIBS += -L/usr/local/lib/ -lnetcdf
    }
    LIBS += -L../agrolib/phenology/release -lphenology
    LIBS += -L../agrolib/importDataXML/release -limportDataXML
    LIBS += -L../agrolib/meteoWidget/release -lmeteoWidget
    LIBS += -L../agrolib/dbMeteoGrid/release -ldbMeteoGrid
    LIBS += -L../agrolib/dbMeteoPoints/release -ldbMeteoPoints
    LIBS += -L../agrolib/utilities/release -lutilities
    LIBS += -L../agrolib/solarRadiation/release -lsolarRadiation
    LIBS += -L../agrolib/interpolation/release -linterpolation
    LIBS += -L../agrolib/drought/release -ldrought
    LIBS += -L../agrolib/meteo/release -lmeteo
    LIBS += -L../agrolib/gis/release -lgis
    LIBS += -L../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../agrolib/mathFunctions/release -lmathFunctions
}


SOURCES += \
    ../agrolib/graphics/colorLegend.cpp \
    ../agrolib/graphics/mapGraphicsRasterObject.cpp \
    ../agrolib/graphics/stationMarker.cpp \
    ../agrolib/graphics/rubberBand.cpp \
    main.cpp \
    mainGUI.cpp \
    mainWindow.cpp \
    dialogPragaProject.cpp \
    dialogMeteoComputation.cpp \
    dialogDownloadMeteoData.cpp \
    dialogClimateFields.cpp \
    dialogPragaSettings.cpp \
    dialogSeriesOnZones.cpp \
    dialogXMLComputation.cpp \
    dialogAnomaly.cpp \
    pragaMeteoMaps.cpp \
    saveClimaLayout.cpp \
    pragaProject.cpp \
    pragaShell.cpp \


HEADERS  += \
    ../agrolib/graphics/colorLegend.h \
    ../agrolib/graphics/mapGraphicsRasterObject.h \
    ../agrolib/graphics/stationMarker.h \
    ../agrolib/graphics/rubberBand.h \
    mainGUI.h \
    mainWindow.h \
    dialogPragaProject.h \
    dialogMeteoComputation.h \
    dialogDownloadMeteoData.h \
    dialogClimateFields.h \
    dialogPragaSettings.h \
    dialogSeriesOnZones.h \
    dialogXMLComputation.h \
    dialogAnomaly.h \
    pragaMeteoMaps.h \
    saveClimaLayout.h \
    pragaProject.h \
    pragaShell.h \


FORMS    += mainWindow.ui


