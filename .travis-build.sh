#!/bin/bash
set -exo pipefail

image=$1

if [[ $image =~ ^centos:8 ]]
then
    pkgcmd="dnf"
    builddep="dnf builddep"
    sed -i '/^tsflags=/d' /etc/dnf/dnf.conf
    dnf install -q -y epel-release
    dnf install -q -y 'dnf-command(config-manager)'
    dnf config-manager --set-enabled PowerTools
    dnf groupinstall -q -y "Development Tools"
    dnf install -q -y 'dnf-command(builddep)'
    dnf install -q -y git
    dnf install -q -y rpmdevtools
    dnf copr enable -y simc/stable
    # TODO install packages and compile code
elif [[ $image =~ ^ubuntu: ]]
then
  apt-get update
  apt-get -y remove postgres* mysql-server* #not update postgres* mysql-server*
  apt-get -y dist-upgrade
  apt-get -y install software-properties-common
  add-apt-repository -y ppa:ubuntu-toolchain-r/test
  add-apt-repository -y ppa:beineri/opt-qt-5.12.6-xenial
  apt-get -y update
  apt-get install -qq g++-8 gcc-8
  update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 90
  update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 90
  apt-get install -qq qt512-meta-minimal
  apt-get install -qq libx11-xcb-dev libglu1-mesa-dev
  update-alternatives --install /usr/bin/qmake qmake /opt/qt512/bin/qmake 90
  export QT_DIR=/opt/qt512
  export PATH=$QT_DIR/bin/:$PATH
  apt-get install libnetcdf-dev libnetcdf11
  gcc --version
  g++ --version
  qmake -v
  bash deploy/build.sh
  wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
  bash upload.sh *.AppImage*
fi
