TEMPLATE = subdirs

SUBDIRS =       ../agrolib/crit3dDate  ../agrolib/mathFunctions  ../agrolib/gis  \
                ../agrolib/meteo  ../agrolib/interpolation  ../agrolib/solarRadiation  \
                ../agrolib/utilities  ../agrolib/dbMeteoPoints  ../agrolib/dbMeteoGrid  \
                ../agrolib/climate  ../agrolib/netcdfHandler \
                ../agrolib/project ../agrolib/meteoWidget \
                ../src/PRAGA.pro

CONFIG += ordered
