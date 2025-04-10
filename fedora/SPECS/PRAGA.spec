%global releaseno 1

# Note: define srcarchivename in CI build only.
%{!?srcarchivename: %global srcarchivename %{name}-%{version}-%{releaseno}}

Name:           PRAGA
Version:        2.0.4
Release:        %{releaseno}%{?dist}
Summary:        PRogram for AGrometeorological Analysis

URL:            https://github.com/ARPA-SIMC/PRAGA
Source0:        https://github.com/ARPA-SIMC/PRAGA/archive/v%{version}-%{releaseno}.tar.gz#/%{srcarchivename}.tar.gz
License:        GPL

BuildRequires:  qt5-qtbase
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtcharts
BuildRequires:  qt5-qtcharts-devel
BuildRequires:  netcdf
BuildRequires:  netcdf-devel

Requires:       qt5-qtbase-mysql

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
qmake-qt5 MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd makeall
qmake-qt5 makeall.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make qmake_all
make
popd

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p %{buildroot}/%{_bindir}/
cp -a bin/PRAGA %{buildroot}/%{_bindir}/

%files
%{_bindir}/PRAGA


%changelog
* Wed Apr 09 2025 Fausto Tomei <ftomei@arpae.it> - 2.0.4-1
- Release 2.0.4

* Mon Mar 10 2025 Caterina Toscano <ctoscano@arpae.it> - 2.0.3-2
- Release 2.0.3

* Mon Mar 10 2025 Caterina Toscano <ctoscano@arpae.it> - 2.0.3-1
- Release 2.0.3

* Wed Jan 22 2025 Fausto Tomei <ftomei@arpae.it> - 2.0.2-1
- Release 2.0.2

* Tue Jan 21 2025 Fausto Tomei <ftomei@arpae.it> - 2.0.1-3
- Release 2.0.1

* Tue Jan 21 2025 Fausto Tomei <ftomei@arpae.it> - 2.0.1-2
- Release 2.0.1

* Fri Jan 17 2025 Fausto Tomei <ftomei@arpae.it> - 2.0.1-1
- Release 2.0.1

* Tue Jan 14 2025 Fausto Tomei <ftomei@arpae.it> - 2.0.0-2
- Release 2.0.0

* Tue Jan 07 2025 Fausto Tomei <ftomei@arpae.it> - 2.0.0-1
- Release 2.0.0

* Fri Mar 15 2024 Laura Costantini <laura.costantini0@gmail.com> - 1.8.3-1
- Release 1.8.3

* Thu Mar 14 2024 Laura Costantini <laura.costantini0@gmail.com> - 1.8.2-1
- Release 1.8.2

* Fri Feb 23 2024 Laura Costantini <laura.costantini0@gmail.com> - 1.8.1-1
- Release 1.8.1

* Thu Feb 08 2024 Laura Costantini <laura.costantini0@gmail.com> - 1.8.0-1
- Release 1.8.0

* Tue Jan 16 2024 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.7.6-2
- Release bump for inconsistent spec in 1.7.6-1

* Tue Jan 16 2024 Gabriele Antolini <gantolini@arpae.it> - 1.7.6-1
- Release 1.7.6

* Wed Dec 20 2023 Laura Costantini <laura.costantini0@gmail.com> - 1.7.5-1
- Release 1.7.5

* Wed Dec 06 2023 Laura Costantini <laura.costantini0@gmail.com> - 1.7.4-1
- Release 1.7.4

* Tue Oct 03 2023 Fausto Tomei <ftomei@arpae.it> - 1.7.3-1
- Release 1.7.3

* Wed Sep 27 2023 Fausto Tomei <ftomei@arpae.it> - 1.7.2-1
- Release 1.7.2

* Tue Feb 07 2023 Fausto Tomei <ftomei@arpae.it> - 1.6.0-1
- Release 1.6.0

* Tue Jan 31 2023 Daniele Branchini <dbranchini@arpae.it> - 1.5.8-1
- Release 1.5.8

* Tue May 31 2022 Fausto Tomei <ftmomei@arpae.it> - 1.5.4-1
- Release 1.5.4

* Mon Dec  6 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.5.1-1
- Release 1.5.1

* Mon Nov 29 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.5.0-1
- Release 1.5.0

* Tue Nov  9 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.4.3-1
- Release 1.4.3

* Tue Jul 20 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.3.9-1
- Release 1.3.9

* Thu Jul  8 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.3.5-1
- Release 1.3.5

* Mon Feb  1 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.3.4-1
- Release 1.3.4

* Tue Jan 26 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.3.3-1
- Release 1.3.3

* Wed Jan 20 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.3.2-1
- Release 1.3.2

* Mon Nov 16 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.2.5-1
- Release 1.2.5

* Fri Nov 13 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.2.4-1
- Release 1.2.4

* Thu Nov 12 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.2.3-1
- Release 1.2.3

* Thu Nov  5 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.2.1-1
- Release 1.2.1

* Thu Oct 29 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.1.1-1
- Fixed working directory problem

* Wed Oct 21 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.0-2
- MySQL driver

* Fri Oct  9 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.0-1
- Release 1.0.0
