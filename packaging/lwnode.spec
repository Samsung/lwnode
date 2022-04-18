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
Summary:     Development files for Lightweight node.js
Group:       System/Servers
Requires:    %{name} = %{version}
%description devel
Development files for Lightweight node.js.

# Initialize the variables
%{!?target: %define target lwnode} #taget = [lwnode/v8/modules/apps/test]
%{!?lib_type: %define lib_type shared}
%{!?feature_mode: %define feature_mode production}
%{!?build_app: %define build_app 1}
%{!?app_name: %define app_name sqlite3}
%{!?build_test: %define build_test 0}

%description
Node.js on Escargot is a memory efficient node.js implementation,
which runs on top of Escargot, a memory optimized JavaScript Engine developed
by Samsung Research, instead of the default V8 JS engine.


# Add subpackage for apps
%if 0%{?build_app} == 1
%package %{app_name}
Summary: lwnode apps
%description %{app_name}
lwnode %{app_name} app

# variables related to app
%define project_path %{_builddir}/%{name}-%{version}
%define local_app_path %{project_path}/lwnode/apps/%{app_name}
%define app_out_path %{project_path}/out/apps/%{app_name}
%define app_files_path /tmp/%{app_name}/files
%define target_app_path /usr/apps/lwnode/apps/%{app_name}
%define app_variables BUILD_OUT_PATH=%{app_out_path} APP_PATH=%{local_app_path} FILES_PATH=%{buildroot}%{app_files_path}
%define app_post_variables APP_PATH=%{target_app_path} FILES_PATH=%{app_files_path}
%endif

# profile: test
%if 0%{?build_test} == 1
%package test
Summary:     Test files for Lightweight node.js
Group:       System/Servers
%description test
Test files for Lightweight node.js

%define target test
%endif

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
%define asan_config --enable-asan
%endif

%if "%{target}" == "lwnode"
%define target_lib liblwnode
%define target_src out/tizen/Release

%if %{?static_escargot:0}%{!?static_escargot:1}
  %define engine_config --without-bundled-v8 --engine escargot
%else
  %define engine_config --without-bundled-v8 --engine escargot --static-escargot
%endif
%endif

%if "%{target}" == "v8"
%define target node
%define target_src out/v8/Release
%endif

%if "%{lib_type}" == "shared"
%define lib_type_config --shared
%endif

%if (0%{?tizen_version_major} == 4) && (0%{?tizen_version_minor} == 0)
  %define libshared --shared-zlib --shared-cares
%else
  %define libshared --shared-zlib --shared-cares --shared-openssl --shared-nghttp2
%endif

%if "%{?feature_mode}" == "production"
  echo -e "\033[0;32m"production"\033[0m"
%else
  echo -e "\033[0;32m"development"\033[0m"
  %define extra_config --escargot-threading
%endif


echo "Build Target:" %{target}
echo $CFLAGS

%if "%{target}" == "modules"
./lwnode/build-modules.sh %{?modules_list} --os=tizen
%endif

%if "%{target}" == "lwnode" || "%{target}" == "v8"
# building liblwnode.so
./configure --tizen --without-npm \
            --without-inspector --without-node-code-cache --without-node-snapshot \
            --with-intl none %{?libshared} \
            --enable-reload-script --enable-external-builtin-script \
            --dest-os linux --dest-cpu '%{tizen_arch}' \
            --ninja %{?engine_config} %{?extra_config} %{?lib_type_config} %{?asan_config}

%if "%{target}" == "lwnode" && "%{lib_type}" == "shared"
  ninja -C %{target_src} %{target_lib}
%endif

# building a static lwnode executable
./configure --tizen --without-npm \
            --without-inspector --without-node-code-cache --without-node-snapshot \
            --with-intl none %{?libshared} \
            --enable-reload-script --enable-external-builtin-script \
            --dest-os linux --dest-cpu '%{tizen_arch}' \
            --ninja %{?engine_config} %{?extra_config} %{?asan_config}
ninja -C %{target_src} %{target}
%endif

%if 0%{?build_app} == 1
%{app_variables} %{local_app_path}/build/build.sh
%endif

# building the cctest executable
%if 0%{?build_test} == 1
%define test_exe cctest
%define local_test_exe %{project_path}/out/tizen/cctest/out/Release/%{test_exe}

./tools/gyp/gyp ./lwnode/code/escargotshim/test/cctest.gyp --depth=. -f ninja \
  --generator-output=./out/tizen/cctest -Dbuild_mode=release \
  -Descargot_lib_type=static_lib -Dtarget_arch=arm -Dtarget_os=tizen \
  -Denable_experimental=true -Descargot_threading=1 -Dinclude_node_bindings=false \
  -Descargot_debugger=0

ninja -C ./out/tizen/cctest/out/Release cctest
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

# for app files
%if 0%{?build_app} == 1
rm -rf %{buildroot}%{app_files_path}
mkdir -p %{buildroot}%{app_files_path}

%{app_variables} %{local_app_path}/build/install.sh
mkdir -p %{buildroot}%{target_app_path}/build
cp %{local_app_path}/build/post.sh %{buildroot}%{target_app_path}/build
cp %{local_app_path}/build/%{app_name}.manifest %{buildroot}%{target_app_path}

%endif

# for test files
%if 0%{?build_test} == 1
mkdir -p %{buildroot}%{_bindir}
cp %{local_test_exe} %{buildroot}%{_bindir}
%endif

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

%if 0%{?build_app} == 1
%post %{app_name}
%{app_post_variables} %{target_app_path}/build/post.sh

%files %{app_name}
%{target_app_path}
%{app_files_path}
%endif

%if 0%{?build_test} == 1
%files test
%{_bindir}/%{test_exe}
%endif
