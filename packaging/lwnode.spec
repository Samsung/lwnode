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
License:       Apache 2.0
Source:        %{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: make
BuildRequires: python
BuildRequires: ninja
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(capi-appfw-app-common)
BuildRequires: pkgconfig(capi-system-info),
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(icu-uc)
BuildRequires: pkgconfig(glib-2.0)

%if (0%{?tizen_version_major} >= 6)
BuildRequires: pkgconfig(openssl1.1)
%else
  %if (0%{?tizen_version_major} == 5) && (0%{?tizen_version_minor} == 5)
BuildRequires: pkgconfig(openssl1.1)
  %else
BuildRequires: pkgconfig(openssl)
  %endif
%endif

##############################################
# Packages for profiles
##############################################

%package devel
Summary:     Development files for Lightweight node.js
Group:       System/Servers
Requires:    %{name} = %{version}
%description devel
Development files for Lightweight node.js.

# Initialize the variables
%{!?node_engine: %define node_engine escargot}
%{!?lib_type: %define lib_type shared}


%description
Node.js on Escargot is a memory efficient node.js implementation,
which runs on top of Escargot, a memory optimized JavaScript Engine developed
by Samsung Research, instead of the default V8 JS engine.


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

%ifarch armv7l
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
CFLAGS+="-fsanitize=address -fsanitize-recover=address -U_FORTIFY_SOURCE -fno-omit-frame-pointer -fno-common"
CXXFLAGS+="-fsanitize=address -fsanitize-recover=address -U_FORTIFY_SOURCE -fno-omit-frame-pointer -fno-common"
LDFLAGS+="-fsanitize=address"
%endif

%if "%{node_engine}" == "escargot"
%define target lwnode
%define target_lib liblwnode
%define target_src out/tizen/Release
%define extra_config --without-bundled-v8 --engine escargot
%else
%define target node
%define target_src out/v8/Release
%endif

%if "%{lib_type}" == "shared"
%define lib_type_config --shared
%endif

echo "Building:" %{target}

./configure --tizen --without-npm \
            --without-inspector --without-node-code-cache --without-node-snapshot \
            --with-intl none --shared-openssl --shared-zlib --dest-os linux --dest-cpu '%{tizen_arch}' \
            --ninja %{?extra_config} %{?lib_type_config}
%if "%{node_engine}" == "escargot" && "%{lib_type}" == "shared"
ninja -C %{target_src} %{target_lib}
%endif
ninja -C %{target_src} %{target}


##############################################
## Install
##############################################

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_libdir}

rm -f %{target_src}/lib/*.tmp %{target_src}/lib/*.TOC
%if "%{node_engine}" == "escargot" && "%{lib_type}" == "shared"
cp %{target_src}/lib/liblwnode.so* %{buildroot}%{_libdir}
cp %{target_src}/gen/escargot/libescargot.so %{buildroot}%{_libdir}
%endif

# for devel files
strip -v -g %{target_src}/%{target}
cp %{target_src}/%{target} %{buildroot}%{_bindir}

%clean
rm ./*.list
rm ./*.manifest

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
%if "%{node_engine}" == "escargot" && "%{lib_type}" == "shared"
%{_libdir}/libescargot.so
%{_libdir}/liblwnode.so*
%endif
%license LICENSE.Apache-2.0 LICENSE.BOEHM-GC LICENSE.BSD-3-Clause LICENSE.MIT LICENSE.NodeJS

%files devel
%manifest packaging/%{name}.manifest
%{_bindir}/%{target}
