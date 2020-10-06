#!/bin/bash

# build mapGraphics
cd mapGraphics
qmake MapGraphics.pro -spec linux-g++-64 CONFIG+=debug CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
make

cd -

# build PRAGA
cd makeall
qmake makeall.pro -spec linux-g++-64 CONFIG+=debug CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile qmake_all 
make 

cd -

# download linuxdeployqt
wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod +x linuxqtdeploy

# build appimage
cp src/PRAGA deploy/appimage/usr/bin/PRAGA
LD_LIBRARY_PATH=`pwd`/mapGraphics/release ./linuxqtdeploy --appimage-extract-and-run deploy/appimage/usr/share/applications/PRAGA.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
