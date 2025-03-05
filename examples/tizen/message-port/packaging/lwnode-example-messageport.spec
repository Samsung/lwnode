Name:          lwnode-example-messageport
Summary:       -
Version:       1.0.0
Release:       1
Group:         System/Servers
License:       MIT
Source:        %{name}-%{version}.tar.gz
Source1001:    lwnode-example-messageport.manifest

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%if ("%{tizen_version}" >= "7.0")
BuildRequires:  sdk-core
BuildRequires:  sdk-rootstraps
%else
BuildRequires: builtin-java
BuildRequires: builtin-tizensdk, builtin-tizensdk-rootstraps
BuildRequires: app-signer, python-xml
%endif

BuildRequires: cmake, ninja
BuildRequires: rsync
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(capi-appfw-app-common)

%define _signer_url  http://<<url>>/kuep_net_signer.sh

%description
sample

# Configure

# Initialize the variables
%define _output ./out
%define _native_out ./out/%{name}
%define _jsroot res
%define _lib_dist ./out/project/%{_jsroot}/lib
%define _native_dist ./out/project/%{_jsroot}/lib/build
%define _project ./out/project
%define _certi ./packaging

%prep
%setup -q

%build
mkdir -p %{_native_out}
cmake . -B%{_native_out} -H. -G Ninja
ninja -v -C %{_native_out}

# project scaffolding
rm -rf %{_project}
mkdir -p %{_project}/%{_jsroot}

# copy lwnode binary
cp -rf bin %{_project}
cp -rf lib %{_project}

# copy JavaScript files
rsync -avm --include='*.js' -f 'hide,! */' ./%{_jsroot}/* %{_project}/%{_jsroot}
rsync -avm --include='*.node' -f 'hide,! */' ./%{_jsroot}/* %{_project}/%{_jsroot}
rsync -avm --include='*.json' -f 'hide,! */' ./%{_jsroot}/* %{_project}/%{_jsroot}
cp -rf tizen-manifest.xml %{_project}

# copy exec file
cp -f %{_native_out}/%{name} %{_project}/bin

# setup tizen certificate profile
tizen cli-config "profiles.path=%{_prefix}/tizen-studio-data/profile/profiles.xml"
tizen security-profiles add -n ABS -a %{_certi}/tizen_author.p12 -p \
    tizenauthor -d %{_certi}/tizen-distributor-partner-manufacturer-signer.p12 \
    -dp tizenpkcs12passfordsigner

# signing
curl -o ./kuep_net_signer.sh %{_signer_url} && chmod +x ./kuep_net_signer.sh
./kuep_net_signer.sh -s -tizen_major_ver %{?tizen_version_major} %{_project}/lib/liblwnode.*
./kuep_net_signer.sh -s -tizen_major_ver %{?tizen_version_major} %{_project}/lib/libescargot.*
./kuep_net_signer.sh -s -tizen_major_ver %{?tizen_version_major} %{_project}/bin/%{name}

# packaging
cd %{_project}
zip ../%{name}.tpk * -r
cd - > /dev/null
tizen package -t tpk -s ABS -- %{_output}/%{name}.tpk

%install

%clean
rm -f ./*.list
rm -f ./*.manifest
rm -rf %{_project}
rm -rf %{_native_out}
rm -f kuep_net_signer.sh

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
