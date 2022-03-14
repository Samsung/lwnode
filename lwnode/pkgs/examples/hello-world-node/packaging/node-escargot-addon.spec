Name:          node-escargot-addon
Summary:       -
Version:       1.0.0
Release:       1
Group:         Development/Libraries
License:       Apache-2.0
Source:        %{name}-%{version}.tar.gz
Source1001:    %{name}.manifest
BuildRequires: cmake, ninja, libgcc

%description
This package provides a node-addon for Tizen platform.

%if 0%{?build_profile:1}
    %define profile_name %{build_profile}
%else
    %define profile_name all
%endif

%define _outdir out_tizen/%{profile_name}

%prep
%setup -q

%build
mkdir -p %{_outdir} && cd %{_outdir}
cmake ../.. -G Ninja && ninja

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
cp -fr %{_outdir}/*.node %{buildroot}%{_bindir}

%files 
%manifest packaging/%{name}.manifest
%{_bindir}/*.node

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
rm -f ./*.list
rm -f ./*.manifest
