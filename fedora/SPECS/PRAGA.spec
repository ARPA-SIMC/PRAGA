# Note: define srcarchivename in Travis build only.
%{!?srcarchivename: %global srcarchivename PRAGA-%{version}}

Name:           PRAGA
Version:        1.0.0
Release:        1%{?dist}
Summary:        PRogram for AGrometeorological Analysis

URL:            https://github.com/ARPA-SIMC/PRAGA
Source0:        https://github.com/ARPA-SIMC/PRAGA/archive/v%{version}.tar.gz#/%{srcarchivename}.tar.gz
License:        GPL

BuildRequires:  qt5-qtbase qt5-devel qt5-qtcharts qt5-qtcharts-devel netcdf netcdf-devel

%description
PRAGA is a geographical application for climatological, meteorological and
agrometeorological analysis. It manages point and gridded dataset, and it
enables gridding point data by using interpolation procedures contained in
agrolib interpolation library. The most relevant agroclimatological variables
are managed (air temperature, precipitation, air relative humidity, solar
radiation, wind intensity and direction, reference evapotranspiration, leaf
wetness). It uses specific widgets and libraries for visualizing data on maps
and graphs.

%prep
%autosetup -n %{srcarchivename}

%build
pushd mapGraphics
qmake-qt5 MapGraphics.pro -spec linux-g++-64 CONFIG+=debug CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd makeall
qmake-qt5 makeall.pro -spec linux-g++-64 CONFIG+=debug CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
make qmake_all
make
popd

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p %{buildroot}/%{_bindir}/
cp -a src/PRAGA %{buildroot}/%{_bindir}/

%files
%{_bindir}/PRAGA

%changelog
* Fri Oct  9 2020 Emanuele Di Giacomo <edigiacomo@arpae.it>
- Release 1.0.0
