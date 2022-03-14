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

Name:          helloworld
Summary:       -
Version:       1.0.0
Release:       1
Group:         Application Framework/Service
License:       MIT
Source:        %{name}-%{version}.tar.gz
Source1:       %{name}.service
Source1001:    %{name}.manifest
BuildRequires: cmake, make, ninja, rsync, zip
BuildRequires: python, libgcc

%description

%{!?build_native: %define build_native false}
%define _jsdir /usr/apps/%{name}
%define _tmpdir /tmp
%define _outdir ./out
%define _prodir ./script
%define _nodedir ./deps/lwnode

%prep
%setup -q

%build
mkdir -p %{_outdir} 
mkdir -p %{_prodir}
%if "%{build_native}" == "true"
# Build native module
echo -e "\033[0;33mBuild native module\033[0m"
%define _module_outdir ./out/module
mkdir -p %{_module_outdir}
cmake . -B%{_module_outdir} -H. -G Ninja
ninja -v -C %{_module_outdir}

mkdir -p %{_prodir}/build
cp %{_module_outdir}/*.node %{_prodir}/build
rm -rf %{_module_outdir}
%endif

%install
mkdir -p %{buildroot}%{_libdir}/systemd/system
mkdir -p %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_libdir}/systemd/system/%{name}.service
ln -sf ../%{name}.service %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants/%{name}.service 

mkdir -p %{buildroot}%{_bindir}
cp -f ./bin/node %{buildroot}%{_bindir}/lwnode

rm -rf %{buildroot}%{_jsdir}
mkdir -p %{buildroot}%{_jsdir}

cp -rf ./lib/* %{_prodir}
cp -rf ./node_modules %{_prodir} 2>/dev/null || :
mkdir -p %{buildroot}/%{_tmpdir}
zip -r %{buildroot}/%{_tmpdir}/script.zip %{_prodir}

%clean
rm ./*.list
rm -rf %{_prodir}

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
    echo -e "\033[0;33mservice start\033[0m"
    systemctl start %{name}.service
fi

if [ $1 == 2 ]; then
    echo -e "\033[0;33mservice restart\033[0m"
    systemctl restart %{name}.service
fi

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
%manifest packaging/%{name}.manifest
%{_bindir}/lwnode
%{_tmpdir}/script.zip
%{_libdir}/systemd/system/%{name}.service
%{_libdir}/systemd/system/multi-user.target.wants/%{name}.service
