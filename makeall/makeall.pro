TEMPLATE = subdirs

SUBDIRS =       ../agrolib/crit3dDate  ../agrolib/mathFunctions  ../agrolib/gis  \
                ../agrolib/meteo  ../agrolib/interpolation  ../agrolib/solarRadiation  \
                ../agrolib/utilities  ../agrolib/outputPoints ../agrolib/dbMeteoPoints  ../agrolib/dbMeteoGrid  \
                ../agrolib/phenology ../agrolib/climate ../agrolib/drought ../agrolib/netcdfHandler \
                ../agrolib/commonDialogs ../agrolib/importDataXML ../agrolib/project ../agrolib/meteoWidget \
                ../agrolib/proxyWidget ../agrolib/pointStatisticsWidget ../agrolib/graphics ../agrolib/pragaDialogs \
                ../src/PRAGA.pro

CONFIG += ordered
