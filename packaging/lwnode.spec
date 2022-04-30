# Copyright (c) 2021-present Samsung Electronics Co., Ltd
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

Name:          lwnode
Summary:       -
Version:       1.0.0
Release:       1
Group:         System/Servers
License:       Apache-2.0 and MIT and BSD-2-Clause and BSD-3-Clause and BOEHM-GC and ICU and LGPL-2.1+ and Zlib
Source:        %{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: make
BuildRequires: python
BuildRequires: ninja
BuildRequires: zip
BuildRequires: rsync
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(capi-appfw-app-common)
BuildRequires: pkgconfig(capi-system-info),
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(icu-uc)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: nghttp2-devel
BuildRequires: pkgconfig(libcares)
BuildRequires: pkgconfig(sqlite3)

%if (0%{?tizen_version_major} >= 6)
BuildRequires: pkgconfig(openssl1.1)
%else
  %if (0%{?tizen_version_major} == 5) && (0%{?tizen_version_minor} == 5)
BuildRequires: pkgconfig(openssl1.1)
  %else
BuildRequires: pkgconfig(openssl)
  %endif
%endif

%if 0%{?asan} == 1
BuildRequires: libasan
%endif

##############################################
# Packages for profiles
##############################################

%package devel
Summary:     Development files for Lightweight Node.js
Group:       System/Servers
Requires:    %{name} = %{version}
%description devel
Development files for Lightweight Node.js.

# Initialize the variables
%{!?target: %define target lwnode}
%{!?lib_type: %define lib_type shared}

%description
Lightweight Node.js is a memory efficient Node.js implementation,
which runs on top of Escargot, a memory optimized JavaScript Engine
developed by Samsung Research, instead of the default V8 JS engine.

%prep
%setup -q


##############################################
# Build
##############################################

%build
gcc --version


##############################################
## Build rules for each profile
##############################################

%ifarch armv7l armv7hl
%define tizen_arch arm
%endif
%ifarch aarch64
%define tizen_arch arm64
%endif
%ifarch i686
%define tizen_arch x32
%endif
%ifarch x86_64
%define tizen_arch x64
%endif

%if 0%{?asan} == 1
%define asan_config --nopt --enable-asan
%endif

%if "%{target}" == "lwnode"
%define target_lib liblwnode
%define target_src out/tizen/Release

%if 0%{?static_escargot} == 1
  %define engine_config --nopt --static-escargot
%endif
%endif

%if "%{lib_type}" == "shared"
%define lib_type_config --shared
%endif

%if (0%{?tizen_version_major} == 4) && (0%{?tizen_version_minor} == 0)
  %define libshared --nopt --shared-zlib --nopt --shared-cares
%else
  %define libshared --nopt --shared-zlib --nopt --shared-cares \\\
                    --nopt --shared-openssl --nopt --shared-nghttp2
%endif

echo "Build Target:" %{target}
echo $CFLAGS

%if "%{target}" == "modules"
./lwnode/build-modules.sh %{?modules_list} --os=tizen
%endif

%if "%{target}" == "lwnode"
# building liblwnode.so
./configure.py --tizen --verbose \
            --nopt --dest-cpu='%{tizen_arch}' \
            %{?lib_type_config} \
            %{?asan_config} \
            %{?libshared} \
            %{?engine_config}

%if "%{target}" == "lwnode" && "%{lib_type}" == "shared"
  ninja -C %{target_src} %{target_lib}
%endif

# building a static lwnode executable
./configure.py --tizen --verbose \
            --nopt --dest-cpu='%{tizen_arch}' \
            %{?asan_config} \
            %{?libshared} \
            %{?engine_config}
ninja -C %{target_src} %{target}
%endif

##############################################
## Install
##############################################

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_libdir}

rm -f %{target_src}/lib/*.tmp %{target_src}/lib/*.TOC
%if "%{target}" == "lwnode"
  %if %{?static_escargot:0}%{!?static_escargot:1}
    cp %{target_src}/gen/escargot/libescargot.so %{buildroot}%{_libdir}
  %endif
  %if "%{lib_type}" == "shared"
    cp %{target_src}/lib/liblwnode.so* %{buildroot}%{_libdir}
  %endif

# for devel files
%if %{?debug_symbols:0}%{!?debug_symbols:1}
  strip -v -g %{target_src}/%{target}
%endif

cp %{target_src}/%{target} %{buildroot}%{_bindir}
cp %{target_src}/%{target}.dat %{buildroot}%{_bindir}

%endif # "%{target}" == "lwnode"

%clean
rm -fr ./*.list
rm -fr ./*.manifest

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

##############################################
## Packaging rpms
##############################################

%files
%manifest packaging/%{name}.manifest
%defattr(-,root,root,-)
%if "%{target}" == "lwnode"
  %if %{?static_escargot:0}%{!?static_escargot:1}
    %{_libdir}/libescargot.so
  %endif
  %if "%{lib_type}" == "shared"
    %{_libdir}/liblwnode.so*
  %endif
  %{_bindir}/%{target}.dat
%endif
%license LICENSE LICENSE.Apache-2.0 LICENSE.NodeJS LICENSE.MIT LICENSE.BSD-2-Clause LICENSE.BSD-3-Clause LICENSE.BOEHM-GC LICENSE.ICU LICENSE.LGPL-2.1+ LICENSE.Zlib

%files devel
%manifest packaging/%{name}.manifest
%if "%{target}" == "lwnode"
  %{_bindir}/%{target}
%endif
