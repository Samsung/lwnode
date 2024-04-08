# Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

Name:          lwnode-modules
Summary:       -
Version:       1.0.0
Release:       1
Group:         System/Servers
License:       Apache-2.0 and MIT and BSD-2-Clause and BSD-3-Clause and BOEHM-GC and ICU and LGPL-2.1+ and Zlib
Source:        %{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: make
BuildRequires: ninja
BuildRequires: boost-devel
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(manifest-parser)
BuildRequires: pkgconfig(tpk-manifest-handlers)

##############################################
# Packages for profiles
##############################################

# Initialize the variables
%{!?modules_list: %define modules_list gmain-loop,device-api}
%define target_lwnode_path /usr/apps/lwnode
%define target_modules_path %{target_lwnode_path}/node_modules
%define local_modules_path modules/packages

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

%description
lwnode modules

%prep
%setup -q


##############################################
# Build
##############################################

%build
gcc --version
rpmbuild --version

##############################################
## Build rules for each profile
##############################################

echo "Build Modules:" %{modules_list}
echo $CFLAGS

./tools/build-modules.sh %{?modules_list} --os=tizen --arch=%{tizen_arch} --clean-after

##############################################
## Install
##############################################

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{target_modules_path}

cp -rf out/modules/tizen/%{tizen_arch}/* %{buildroot}%{target_modules_path}

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
%defattr(-,tizenglobalapp,root,-)
%manifest %{local_modules_path}/packaging/%{name}.manifest
%{target_lwnode_path}

%license LICENSE LICENSE.Apache-2.0 LICENSE.NodeJS LICENSE.MIT LICENSE.BSD-2-Clause LICENSE.BSD-3-Clause LICENSE.BOEHM-GC LICENSE.ICU LICENSE.LGPL-2.1+ LICENSE.Zlib
