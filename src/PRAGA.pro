#-----------------------------------------------------
#
#   PRAGA
#   PRogram for AGrometeorological Analysis
#
#-----------------------------------------------------

QT       += widgets charts network sql xml
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

TARGET = PRAGA
TEMPLATE = app
VERSION = 2.0.3


INCLUDEPATH +=  ../mapGraphics \
                ../agrolib/crit3dDate ../agrolib/mathFunctions ../agrolib/phenology ../agrolib/meteo ../agrolib/gis  \
                ../agrolib/drought ../agrolib/interpolation ../agrolib/solarRadiation ../agrolib/utilities  \
                ../agrolib/outputPoints ../agrolib/dbMeteoPoints ../agrolib/dbMeteoGrid ../agrolib/meteoWidget  \
                ../agrolib/proxyWidget ../agrolib/pointStatisticsWidget ../agrolib/homogeneityWidget \
                ../agrolib/synchronicityWidget ../agrolib/climate ../agrolib/netcdfHandler  \
                ../agrolib/graphics ../agrolib/commonDialogs ../agrolib/commonChartElements \
                ../agrolib/pragaDialogs ../agrolib/inOutDataXML ../agrolib/waterTable \
                ../agrolib/project ../agrolib/pragaProject

CONFIG += debug_and_release

CONFIG += c++11 c++14 c++17

DEFINES += NETCDF


CONFIG(debug, debug|release) {
    LIBS += -L../agrolib/pragaProject/debug -lpragaProject
    LIBS += -L../agrolib/project/debug -lproject
    LIBS += -L../agrolib/pragaDialogs/debug -lpragaDialogs
    LIBS += -L../agrolib/commonDialogs/debug -lcommonDialogs
    LIBS += -L../agrolib/climate/debug -lclimate
    LIBS += -L../agrolib/phenology/debug -lphenology
    LIBS += -L../agrolib/netcdfHandler/debug -lnetcdfHandler
    LIBS += -L../agrolib/graphics/debug -lgraphics
    win32:{
        LIBS += -L../mapGraphics/debug -lMapGraphics
        LIBS += -L$$(NC4_INSTALL_DIR)/lib -lnetcdf
    }
    unix:{
        LIBS += -L../mapGraphics/release -lMapGraphics
        LIBS += -lnetcdf
    }
    macx:{
        LIBS += -L../mapGraphics/debug -lMapGraphics
        LIBS += -L/usr/local/lib/ -lnetcdf
    }
    LIBS += -L../agrolib/waterTable/debug -lwaterTable
    LIBS += -L../agrolib/inOutDataXML/debug -linOutDataXML
    LIBS += -L../agrolib/synchronicityWidget/debug -lsynchronicityWidget
    LIBS += -L../agrolib/homogeneityWidget/debug -lhomogeneityWidget
    LIBS += -L../agrolib/pointStatisticsWidget/debug -lpointStatisticsWidget
    LIBS += -L../agrolib/proxyWidget/debug -lproxyWidget
    LIBS += -L../agrolib/meteoWidget/debug -lmeteoWidget
    LIBS += -L../agrolib/commonChartElements/debug -lcommonChartElements
    LIBS += -L../agrolib/dbMeteoGrid/debug -ldbMeteoGrid
    LIBS += -L../agrolib/drought/debug -ldrought
    LIBS += -L../agrolib/dbMeteoPoints/debug -ldbMeteoPoints
    LIBS += -L../agrolib/outputPoints/debug -loutputPoints
    LIBS += -L../agrolib/utilities/debug -lutilities
    LIBS += -L../agrolib/solarRadiation/debug -lsolarRadiation
    LIBS += -L../agrolib/interpolation/debug -linterpolation
    LIBS += -L../agrolib/meteo/debug -lmeteo
    LIBS += -L../agrolib/gis/debug -lgis
    LIBS += -L../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../agrolib/mathFunctions/debug -lmathFunctions

} else {
    LIBS += -L../agrolib/pragaProject/release -lpragaProject
    LIBS += -L../agrolib/graphics/release -lgraphics
    LIBS += -L../mapGraphics/release -lMapGraphics
    LIBS += -L../agrolib/project/release -lproject
    LIBS += -L../agrolib/pragaDialogs/release -lpragaDialogs
    LIBS += -L../agrolib/commonDialogs/release -lcommonDialogs
    LIBS += -L../agrolib/climate/release -lclimate
    LIBS += -L../agrolib/phenology/release -lphenology
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
    LIBS += -L../agrolib/waterTable/release -lwaterTable
    LIBS += -L../agrolib/inOutDataXML/release -linOutDataXML
    LIBS += -L../agrolib/synchronicityWidget/release -lsynchronicityWidget
    LIBS += -L../agrolib/homogeneityWidget/release -lhomogeneityWidget
    LIBS += -L../agrolib/pointStatisticsWidget/release -lpointStatisticsWidget
    LIBS += -L../agrolib/proxyWidget/release -lproxyWidget
    LIBS += -L../agrolib/meteoWidget/release -lmeteoWidget
    LIBS += -L../agrolib/commonChartElements/release -lcommonChartElements
    LIBS += -L../agrolib/dbMeteoGrid/release -ldbMeteoGrid
    LIBS += -L../agrolib/drought/release -ldrought
    LIBS += -L../agrolib/dbMeteoPoints/release -ldbMeteoPoints
    LIBS += -L../agrolib/outputPoints/release -loutputPoints
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
    mainGUI.cpp \
    mainWindow.cpp


HEADERS  += \
    mainGUI.h \
    mainWindow.h

FORMS    += mainWindow.ui

RESOURCES += bull.ico

win32:
{
    RC_ICONS = bull.ico
}



