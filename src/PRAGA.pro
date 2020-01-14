#-----------------------------------------------------
#
#   PRAGA
#   PRogram for AGrometeorological Analysis
#
#-----------------------------------------------------

QT       += core gui widgets network sql xml

TARGET = PRAGA
TEMPLATE = app

INCLUDEPATH +=  ../mapGraphics \
                ../agrolib/crit3dDate ../agrolib/mathFunctions ../agrolib/meteo ../agrolib/gis  \
                ../agrolib/interpolation ../agrolib/solarRadiation ../agrolib/utilities  \
                ../agrolib/dbMeteoPoints ../agrolib/dbMeteoGrid ../agrolib/climate \
                ../agrolib/netcdfHandler  ../agrolib/graphics ../agrolib/project

CONFIG += debug_and_release
QMAKE_CXXFLAGS += -std=c++11

DEFINES += NETCDF


    win32:{
        CONFIG(debug, debug|release) {
            LIBS += -L../mapGraphics/debug -lMapGraphics
        } else {
            LIBS += -L../mapGraphics/release -lMapGraphics
        }
    }
    unix:{
        LIBS += -L../mapGraphics/release -lMapGraphics
    }


CONFIG(debug, debug|release) {
    LIBS += -L../agrolib/graphics/debug -lgraphics
    LIBS += -L../agrolib/project/debug -lproject
    LIBS += -L../agrolib/climate/debug -lclimate
    LIBS += -L../agrolib/netcdfHandler/debug -lnetcdfHandler
    win32:{
        LIBS += -L$$(NC4_INSTALL_DIR)/lib -lnetcdf
    }
    unix:{
        LIBS += -lnetcdf
    }
    macx:{
        LIBS += -L/usr/local/lib/ -lnetcdf
    }
    LIBS += -L../agrolib/dbMeteoGrid/debug -ldbMeteoGrid
    LIBS += -L../agrolib/dbMeteoPoints/debug -ldbMeteoPoints
    LIBS += -L../agrolib/utilities/debug -lutilities
    LIBS += -L../agrolib/solarRadiation/debug -lsolarRadiation
    LIBS += -L../agrolib/interpolation/debug -linterpolation
    LIBS += -L../agrolib/meteo/debug -lmeteo
    LIBS += -L../agrolib/gis/debug -lgis
    LIBS += -L../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../agrolib/mathFunctions/debug -lmathFunctions

} else {
    LIBS += -L../agrolib/graphics/release -lgraphics
    LIBS += -L../agrolib/project/release -lproject
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
    LIBS += -L../agrolib/dbMeteoGrid/release -ldbMeteoGrid
    LIBS += -L../agrolib/dbMeteoPoints/release -ldbMeteoPoints
    LIBS += -L../agrolib/utilities/release -lutilities
    LIBS += -L../agrolib/solarRadiation/release -lsolarRadiation
    LIBS += -L../agrolib/interpolation/release -linterpolation
    LIBS += -L../agrolib/meteo/release -lmeteo
    LIBS += -L../agrolib/gis/release -lgis
    LIBS += -L../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../agrolib/mathFunctions/release -lmathFunctions
}


SOURCES += \
    main.cpp \
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
    pragaShell.cpp


HEADERS  += \
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
    pragaShell.h


FORMS    += mainWindow.ui


