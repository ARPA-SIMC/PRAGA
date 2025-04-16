#!/bin/bash

set -e 

image=$1

if [[ $image =~ ^centos:8 ]]
then
    echo "qmake $QMAKE"
    # build mapGraphics
    cd mapGraphics
    $QMAKE MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build PRAGA
    cd makeall
    $QMAKE makeall.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make -f Makefile qmake_all 
    make 
    
elif [[ $image =~ ^ubuntu: ]]
then
    # build mapGraphics
    cd mapGraphics
    $QMAKE MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make

    cd -

    # build PRAGA
    cd makeall
    $QMAKE makeall.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make -f Makefile qmake_all 
    make 

    cd -

    # download linuxdeployqt
    wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
    chmod +x linuxqtdeploy

    # build appimage
    cp bin/PRAGA deploy/appimage/usr/bin/PRAGA
    LD_LIBRARY_PATH=`pwd`/mapGraphics/release ./linuxqtdeploy --appimage-extract-and-run deploy/appimage/usr/share/applications/PRAGA.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
else
    echo "Unknown image $image"
    exit 1
fi
