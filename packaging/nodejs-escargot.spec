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

Name:          nodejs-escargot
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

# Initialize the variables
%{!?build_mode: %define build_mode release}
%{!?build_target: %define build_target node}
%{!?node_engine: %define node_engine escargot}
%{!?binary_name: %define binary_name nodeescargot}
%{!?build_shared_lib: %define build_shared_lib false}
%{!?build_profile: %define build_profile none}


%description
Node.js on Escargot is a memory efficient node.js implementation,
which runs on top of Escargot, a memory optimized JavaScript Engine developed
by Samsung Research, instead of the default V8 JS engine.


%prep
%setup -q

%build
gcc --version

%if 0%{?asan} == 1
CFLAGS+="-fsanitize=address -fsanitize-recover=address -U_FORTIFY_SOURCE -fno-omit-frame-pointer -fno-common"
CXXFLAGS+="-fsanitize=address -fsanitize-recover=address -U_FORTIFY_SOURCE -fno-omit-frame-pointer -fno-common"
LDFLAGS+="-fsanitize=address"
%endif

echo "Build Configure"
echo %{build_target}

CONFIG="--without-npm --without-bundled-v8 --without-v8-platform \
    --without-inspector --without-node-code-cache --without-node-snapshot \
    --with-intl none  --shared-openssl --shared-zlib \
    --engine escargot \
    --ninja"

./configure $CONFIG
ninja -C out/Release node

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_libdir}

cp -fr ./out/Release/%{build_target} %{buildroot}%{_bindir}
#cp -fr ./out/Release/libescargot.so %{buildroot}%{_libdir}

%clean
rm ./*.list

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%manifest packaging/%{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/*
%{_bindir}/*
%license LICENSE LICENSE.Microsoft LICENSE.NodeJS
