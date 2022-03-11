#
# Copyright (c) 2020-present Samsung Electronics Co., Ltd
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA

Name:          async-work
Summary:       -
Version:       1.0.0
Release:       1
Group:         System/Servers
License:       MIT
Source:        %{name}-%{version}.tar.gz
Source1001:    %{name}.manifest

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake, ninja, libgcc

%description

# Initialize the variables
%define _outdir build/%{name}

%prep
%setup -q

%build
mkdir -p %{_outdir} && cd %{_outdir}
cmake ../.. -DTARGET_OS_TIZEN=1 -G Ninja && ninja

%install
mv %{_outdir}/*.node %{_outdir}/../

%clean
rm ./*.list

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
