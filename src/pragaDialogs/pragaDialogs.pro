#------------------------------------------------------
#
#   library pragaDialogs
#   IT contains dialogs for PRAGA executable
#
#   This project is part of ARPAE PRAGA distribution
#
#------------------------------------------------------

QT  += widgets sql xml

TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release
CONFIG += c++11 c++14 c++17

DEFINES += _CRT_SECURE_NO_WARNINGS

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/pragaDialogs
    } else {
        TARGET = release/pragaDialogs
    }
}
win32:{
    TARGET = pragaDialogs
}

INCLUDEPATH +=  ../../agrolib/mathFunctions ../../agrolib/crit3dDate ../../agrolib/gis  \
                ../../agrolib/meteo ../../agrolib/interpolation ../../agrolib/dbMeteoPoints \
                ../../agrolib/dbMeteoGrid  ../../agrolib/project \
                ../phenology ../climate

SOURCES += \
    dialogAddMissingStation.cpp \
    dialogAddRemoveDataset.cpp \
    dialogCellSize.cpp \
    dialogClimateFields.cpp \
    dialogComputeData.cpp \
    dialogComputeDroughtIndex.cpp \
    dialogDownloadMeteoData.cpp \
    dialogExportDataGrid.cpp \
    dialogSelectDataset.cpp \
    dialogSeriesOnZones.cpp \
    dialogShiftData.cpp \
    dialogXMLComputation.cpp

HEADERS += \
    dialogAddMissingStation.h \
    dialogAddRemoveDataset.h \
    dialogCellSize.h \
    dialogClimateFields.h \
    dialogComputeData.h \
    dialogComputeDroughtIndex.h \
    dialogDownloadMeteoData.h \
    dialogExportDataGrid.h \
    dialogSelectDataset.h \
    dialogSeriesOnZones.h \
    dialogShiftData.h \
    dialogXMLComputation.h

