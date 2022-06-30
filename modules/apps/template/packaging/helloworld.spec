#
# Copyright 2022-present Samsung Electronics Co., Ltd.
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
Summary:       LWNode.js Web Service Scaffolding
Version:       1.0.0
Release:       1
Group:         Application Framework/Service
License:       MIT
Source:        %{name}-%{version}.tar.gz
Source1:       %{name}.service
Source1001:    %{name}.manifest
BuildRequires: rsync, zip

%description

%define _jsdir /usr/apps/%{name}
%define _tmpdir /tmp
%define _prodir ./script

%prep
%setup -q

%install
mkdir -p %{buildroot}%{_libdir}/systemd/system
mkdir -p %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_libdir}/systemd/system/%{name}.service
ln -sf ../%{name}.service %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants/%{name}.service

rm -rf %{buildroot}%{_jsdir}
mkdir -p %{buildroot}%{_jsdir}
mkdir -p %{_prodir}

# Filter scripts ###############################
cp -rf lib/* %{_prodir}
cp -rf node_modules %{_prodir}
# ##############################################

mkdir -p %{buildroot}/%{_tmpdir}
zip -r --symlinks %{buildroot}/%{_tmpdir}/script.zip %{_prodir}

%clean
rm -f ./*.list
rm -f ./*.manifest
rm -rf %{_prodir}

%post
/sbin/ldconfig

echo -e "copy scripts"
rm -rf %{_jsdir}
unzip -q %{_tmpdir}/script.zip -d %{_jsdir}
if [ $? != 0 ]; then
    echo -e "failed to unzip scripts"
    exit 1
fi
rm %{_tmpdir}/script.zip

echo -e "start daemon"
systemctl daemon-reload
[[ $1 == 1 ]] && systemctl start %{name}.service
[[ $1 == 2 ]] && systemctl restart %{name}.service

echo -e "done"

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
%manifest packaging/%{name}.manifest
%{_tmpdir}/script.zip
%{_libdir}/systemd/system/%{name}.service
%{_libdir}/systemd/system/multi-user.target.wants/%{name}.service
