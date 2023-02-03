## HOWTO install mySQL (MariaDB) driver for Qt6 - visual studio WIN64

Qt instructions: https://doc.qt.io/qt-6/sql-driver.html#qmysql

1) Add to qt installation:
- Sources
- CMake (in Developer and Designer Tools)
- Ninja (in Developer and Designer Tools)
 
2) Download MariaDB connector for WIN64:
https://mariadb.com/downloads/connectors/

3) Check the path of these files (the path will be used in steps 6 and 7):  
"MariaDB Connector path"/lib/libmariadb.lib  
"MariaDB Connector path"/lib/libmariadb.dll  
"MariaDB Connector path"/include/mysql.h  

4) Open Qt command (menu start->Qt)  

5) Run vcvars64.bat (to set the VC environment)  
`C:\"Program Files (x86)"\"Microsoft Visual Studio"\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat`  

6) Build the *release* plugin as follows: 
```
mkdir build_sqldrivers_release
cd build_sqldrivers_release
qt-cmake -G Ninja -DCMAKE_BUILD_TYPE=Release <Qt_installation_path>\Src\qtbase\src\plugins\sqldrivers -DCMAKE_INSTALL_PREFIX=<Qt_installation_path>\<your platform> -DMySQL_INCLUDE_DIR="<Maria DB connector path>\include" -DMySQL_LIBRARY="<Maria DB connector path>\lib\libmariadb.lib"
cmake --build .
cmake --install .
```  
*example of cmake command:*  
```
qt-cmake -G Ninja -DCMAKE_BUILD_TYPE=Release C:\Qt\6.2.4\Src\qtbase\src\plugins\sqldrivers -DCMAKE_INSTALL_PREFIX=C:\Qt\6.2.4\msvc2019_64 -DMySQL_INCLUDE_DIR="C:\Program Files\MariaDB\MariaDB Connector C 64-bit\include" -DMySQL_LIBRARY="C:\Program Files\MariaDB\MariaDB Connector C 64-bit\lib\libmariadb.lib"
```  
7) Build the *debug* plugin (in another directory):
```
mkdir build_sqldrivers_debug
cd build_sqldrivers_debug
qt-cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug <Qt_installation_path>\Src\qtbase\src\plugins\sqldrivers -DCMAKE_INSTALL_PREFIX=<Qt_installation_path>\<your platform> -DMySQL_INCLUDE_DIR="<Maria DB connector path>\include" -DMySQL_LIBRARY="<Maria DB connector path>\lib\libmariadb.lib"
cmake --build .
cmake --install .
```

8) When you distribute your application, remember to include libmariadb.dll in your installation package. 
It must be placed in the same folder as the application executable.
