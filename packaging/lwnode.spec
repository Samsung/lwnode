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
%{!?target: %define target lwnode}
%{!?target_lib: %define target_lib liblwnode}
%{!?node_engine: %define node_engine escargot}
%{!?build_profile: %define build_profile none}


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

echo "Building:" %{target}

./configure --without-npm --without-bundled-v8 \
            --without-inspector --without-node-code-cache --without-node-snapshot \
            --with-intl none --shared-openssl --shared-zlib --dest-os linux --dest-cpu '%{tizen_arch}' \
            --engine escargot --ninja --shared

ninja -C out/Release %{target_lib}
ninja -C out/Release %{target}


##############################################
## Install
##############################################

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_libdir}

rm -f ./out/Release/lib/*.tmp ./out/Release/lib/*.TOC
cp ./out/Release/lib/liblwnode.so* %{buildroot}%{_libdir}
cp ./out/Release/gen/escargot/libescargot.so %{buildroot}%{_libdir}

# for devel files
cp ./out/Release/%{target} %{buildroot}%{_bindir}

%clean
rm ./*.list

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
%{_libdir}/libescargot.so
%{_libdir}/liblwnode.so*
%license LICENSE.Apache-2.0 LICENSE.BOEHM-GC LICENSE.BSD-3-Clause LICENSE.MIT LICENSE.NodeJS

%files devel
%manifest packaging/%{name}.manifest
%{_bindir}/%{target}
