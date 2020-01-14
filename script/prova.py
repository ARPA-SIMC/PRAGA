# python3 prova.py BIC BIC3.nc  python3 prova.py VARIABILE NOMENETCDF

import sys
# importing Magics module
from Magics.macro import *
from netCDF4 import Dataset
from osgeo import gdal

if (len(sys.argv)-1 < 2 or len(sys.argv)-1 > 2): 
    print("Invalid number of arguments - Usage:  script variable netcdfFileName")
    sys.exit(0)
    
    
# step values for contour
maxValue = 900.
minValue = -1000.
step = (maxValue - minValue) / 32.

#datasetName = 'BIC3.nc' 
variableName =  sys.argv[1] 
datasetName = sys.argv[2] 

nc_fid = Dataset(datasetName, 'r') 
lats = nc_fid.variables['latitude'][:]
lons = nc_fid.variables['longitude'][:]
maxLat = max(lats)
minLat = min(lats)
maxLon = max(lons)
minLon = min(lons)


print(maxLat)
print(minLat)
print(maxLon)
print(minLon)

ratio = (maxLon - minLon) / (maxLat - minLat)
print(ratio)

width = 1024
length = round (width/ratio)
width_cm = width/40
length_cm = length/40

print(width)
print(length)

output_name= datasetName[:-3] + '_magics'
# output
output = output(output_formats= ['png'],
    output_name= output_name,
    output_name_first_page_number= 'off',
    output_cairo_transparent_background='transparent',
    subpage_frame= 'off',
    page_x_length= width_cm,
    page_y_length= length_cm,
    super_page_x_length= width_cm,
    super_page_y_length= length_cm,
    subpage_x_length= width_cm,
    subpage_y_length= length_cm,
    subpage_x_position= 0.,
    subpage_y_position= 0.,
    output_width= width,
    page_frame= 'off',
    page_id_line = 'off')


# geographical area
area = mmap(
    subpage_map_projection= 'EPSG:4326',
    subpage_coordinates_system = 'latlon',
    subpage_lower_left_longitude= float(minLon),
    subpage_lower_left_latitude= float(minLat),
    subpage_upper_right_longitude= float(maxLon),
    subpage_upper_right_latitude= float(maxLat),
    subpage_frame= 'off')
 
    
# coastlines
coast = mcoast(map_grid= "off",
    map_grid_colour= "tan",
    map_coastline_colour= "tan",
    map_coastline_resolution= "high")

# data 
variable = mnetcdf(netcdf_type = "geomatrix",
    netcdf_filename = datasetName,
    netcdf_value_variable = "var",
    netcdf_field_automatic_scaling = "off",
    netcdf_missing_attribute = "-9999",
    netcdf_field_suppress_below = -9999,
    netcdf_field_suppress_above = 9999)
    
#netcdf_time_dimension_setting = "002-07-01 12:00:00",

# contour
contour = mcont(legend="off",
    contour_shade= "on",
    contour_hilo= "off",
    contour= "off",
    contour_label= "off",
    contour_shade_method= "area_fill",		     
    contour_shade_max_level= maxValue,
    contour_shade_min_level= minValue,
    contour_level_selection_type= "interval",
    contour_interval= step,
    contour_shade_palette_name = 'eccharts_rainbow_black_darkred_32',
    contour_shade_colour_method = 'palette'
    #contour_shade_colour_method= "list",
    #contour_shade_colour_list= [ "blue", "blue",
    #                          "blue_purple","greenish_blue",
    #                          "blue_green","bluish_green",
    #                          "yellow_green","greenish_yellow",
    #                          "yellow","orangish_yellow",
    #                          "orange_yellow","yellowish_orange",
    #                          "orange","reddish_orange",
    #                          "red_orange","orangish_red",
    #                          "red","red",
    #                          "magenta", "violet" ]
    )


plot(output, area, variable, contour)

ds = gdal.Open(output_name+'.png')
output_translate_name = output_name + '_translate.geotiff'
ds = gdal.Translate(output_translate_name, ds, format = 'GTiff', outputSRS = 'EPSG:4326', outputBounds = [float(minLon), float(maxLat), float(maxLon), float(minLat)])
ds = None
ds = gdal.Open(output_translate_name)
output_warp_name = output_name + '_warp.geotiff'
ds = gdal.Warp(output_warp_name, ds, format = 'GTiff', cutlineDSName = 'outputwith4236.shp')
ds = None

