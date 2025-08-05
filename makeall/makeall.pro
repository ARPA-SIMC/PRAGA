TEMPLATE = subdirs

SUBDIRS =       ../agrolib/crit3dDate  ../agrolib/mathFunctions  ../agrolib/gis         \
                ../agrolib/meteo  ../agrolib/interpolation  ../agrolib/solarRadiation   \
                ../agrolib/utilities  ../agrolib/outputPoints  ../agrolib/dbMeteoPoints \
                ../agrolib/dbMeteoGrid ../agrolib/netcdfHandler ../agrolib/waterTable   \
                ../agrolib/commonDialogs ../agrolib/commonChartElements ../agrolib/inOutDataXML  \
                ../agrolib/meteoWidget ../agrolib/proxyWidget ../agrolib/graphics ../agrolib/project  \
                ../src/phenology ../src/climate ../src/drought  ../src/pointStatisticsWidget     \
                ../src/homogeneityWidget ../src/synchronicityWidget ../src/pragaDialogs ../src/pragaProject \
                ../bin/PRAGA.pro

CONFIG += ordered
