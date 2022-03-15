#
# Copyright 2020-present Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

Name:          node-sqlite3-ep
Summary:       -
Version:       1.0.0
Release:       1
Group:         Application Framework/Service
License:       Apache-2.0
Source:        %{name}-%{version}.tar.gz
Source1:       %{name}.service
Source1001:    %{name}.manifest
BuildRequires: cmake
BuildRequires: make
BuildRequires: ninja
BuildRequires: rsync
BuildRequires: zip
BuildRequires: python
BuildRequires: libgcc
BuildRequires: glibc-devel
BuildRequires: libstdc++-devel
BuildRequires: zlib-devel
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(capi-appfw-app-common)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(icu-uc)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(sqlite3)

%%if (0%{?tizen_version_major} >= 6)
BuildRequires: libopenssl1.1-devel
%else
  %if (0%{?tizen_version_major} == 5) && (0%{?tizen_version_minor} == 5)
BuildRequires: libopenssl-devel
  %else
BuildRequires: libopenssl-devel
  %endif
%endif

%description

%define _jsdir /usr/apps/%{name}
%define _tmpdir /tmp
%define _prodir ./script

##############################################
# Prep
##############################################
%prep
%setup -q


##############################################
# Build
##############################################
%build

%ifarch armv7l
%define tizen_arch arm
%endif
%ifarch aarch64
%define tizen_arch aarch64
%endif
%ifarch i686
%define tizen_arch i686
%endif
%ifarch x86_64
%define tizen_arch x86_64
%endif

%if "%{tizen_arch}" == "arm"
find . -name CMakeCache.txt | xargs rm -f
find . -name CMakeFiles | xargs rm -fr
rm -fr ./out_tizen
cmake CMakeLists.txt -B./out_tizen -H. -G Ninja
ninja -vC ./out_tizen
mkdir -p ./node_modules/sqlite3/lib/binding/napi-v3-linux-arm
cp -f ./out_tizen/node_sqlite3.node ./node_modules/sqlite3/lib/binding/napi-v3-linux-arm
rm -rf ./out_tizen
%endif

##############################################
## Install
##############################################
%install
mkdir -p %{buildroot}%{_libdir}/systemd/system
mkdir -p %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_libdir}/systemd/system/%{name}.service
ln -sf ../%{name}.service %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants/%{name}.service

rm -rf %{buildroot}%{_jsdir}
mkdir -p %{buildroot}%{_jsdir}
mkdir -p %{_prodir}

if [ -f "tools/sqlite3-filter.txt" ]; then
  # rsync is only used when building rpm in dev environment
  find node_modules | rsync -avmL --delete --delete-excluded --filter="merge tools/sqlite3-filter.txt" --files-from=- ./ %{_prodir}
else
  cp -fr node_modules %{_prodir}
fi

cp -rf download %{_prodir}
cp -rf out/backend/* %{_prodir} # *.js files should have been generated
mkdir -p %{buildroot}/%{_tmpdir}
zip -r %{buildroot}/%{_tmpdir}/script.zip %{_prodir}

%clean
rm -f ./*.list
rm -f ./*.manifest
rm -rf %{_prodir}

##############################################
## Scripts
##############################################

%post
/sbin/ldconfig
echo -e "\033[0;33mclean project folder\033[0m"
rm -rf %{_jsdir}

echo -e "\033[0;33munzip script\033[0m"
unzip -q %{_tmpdir}/script.zip -d %{_jsdir}
if [ $? != 0 ];then
    echo -e "\033[0;31mfaile to unzip script\033[0m"
fi
rm %{_tmpdir}/script.zip

systemctl daemon-reload
if [ $1 == 1 ]; then
    echo -e "\033[0;33msqlite3-service start\033[0m"
    systemctl start %{name}.service
fi

if [ $1 == 2 ]; then
    echo -e "\033[0;33msqlite3-service restart\033[0m"
    systemctl restart %{name}.service
fi

%postun
/sbin/ldconfig

##############################################
## Packaging rpms
##############################################

%files
%defattr(-,root,root,-)
%manifest packaging/%{name}.manifest
%{_tmpdir}/script.zip
%{_libdir}/systemd/system/%{name}.service
%{_libdir}/systemd/system/multi-user.target.wants/%{name}.service
%license LICENSE LICENSE.BOEHM-GC LICENSE.BSD-3-Clause LICENSE.MIT LICENSE.NodeJS
