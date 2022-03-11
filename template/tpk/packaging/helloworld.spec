Name:          helloworld
Summary:       -
Version:       1.0.0
Release:       1
Group:         System/Servers
License:       MIT
Source:        %{name}-%{version}.tar.gz
Source1001:    x.manifest

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: builtin-java
BuildRequires: builtin-tizensdk, builtin-tizensdk-rootstraps
BuildRequires: app-signer, python-xml

BuildRequires: cmake, ninja
BuildRequires: pkgconfig(dlog)

%define _signer_url http://10.40.68.214/kuep_net_signer.sh

%description
sample

# Configure
%{!?native_module: %define native_module false}

# Initialize the variables
%define _output ./out
%define _native_out ./out/%{name}
%define _native_dist ./out/project/res/lib/build
%define _project ./out/project
%define _certi ./packaging

%prep
%setup -q

%build
%if "%{native_module}" == "true"
mkdir -p %{_native_out}
cmake . -B%{_native_out} -H. -G Ninja
ninja -v -C %{_native_out}
%endif

# project scaffolding
rm -rf %{_project}
mkdir -p %{_project}/res

cp -rf bin %{_project}
cp -rf index.js %{_project}/res
cp -rf lib %{_project}/res
cp -rf node_modules %{_project}/res 2>/dev/null || :
cp -rf tizen-manifest.xml %{_project}

# build native-module
%if "%{native_module}" == "true"
mkdir -p %{_native_dist}
cp -f %{_native_out}/*.node %{_native_dist}
%endif

# setup tizen certificate profile
tizen cli-config "profiles.path=%{_prefix}/tizen-studio-data/profile/profiles.xml"
tizen security-profiles add -n ABS -a %{_certi}/tizen_author.p12 -p \
    tizenauthor -d %{_certi}/tizen-distributor-partner-manufacturer-signer.p12 \
    -dp tizenpkcs12passfordsigner

# signing
curl -o ./kuep_net_signer.sh %{_signer_url} && chmod +x ./kuep_net_signer.sh
./kuep_net_signer.sh -s -tizen_major_ver %{?tizen_version_major} %{_project}/bin/node
%if "%{native_module}" == "true"
find %{_native_dist} -name '*.node' -type f -printf '\n%kK %P\n' -exec ./kuep_net_signer.sh -s -tizen_major_ver %{?tizen_version_major} {} \;
%endif

# packaging
cd %{_project}
zip ../%{name}.tpk * -r
cd - > /dev/null
tizen package -t tpk -s ABS -- %{_output}/%{name}.tpk

%install

%clean
rm ./*.list
rm -rf %{_project}
%if "%{native_module}" == "true"
rm -rf %{_native_out}
%endif

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
