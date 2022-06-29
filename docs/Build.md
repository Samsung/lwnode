# How to build lwnode

## 1. Supported Platforms
* Ubuntu 16.04 and above
* Tizen 4.0 and above

## 2. Checkout code
```sh
$ git clone git@github.sec.samsung.net:lws/node-escargot.git
$ git submodule update --init --recursive
```

## 3. How to Compile: Ubuntu
### 3.1. Install required packages
```sh
$ sudo apt-get install -y build-essential cmake clang libicu-dev libglib2.0-dev
```

### 3.2. How to build and run
```sh
$ ./configure.py
$ ninja -C out/linux/Release lwnode
$ ./out/linux/Release/lwnode ./test/message/hello_world.js
```

### 3.3. How to run testcases
```sh
$ ./tools/test.sh
```

Known Issues:
* For systems enforcing higher security levels, (e.g., Ubuntu 20.04 and above) an error related to `ERR_SSL_EE_KEY_TOO_SMALL` may be thrown. In that case, update `openssl.cnf` as follows:

```sh
vi /etc/ssl/openssl.cnf

# Add the following line to the beginning of the config file
openssl_conf = default_conf

# Add the following lines to the end
[ default_conf ]
ssl_conf = ssl_sect

[ssl_sect]
system_default = system_default_sect

[system_default_sect]
MinProtocol = TLSv1.2
CipherString = DEFAULT:@SECLEVEL=1
```

## 4. How to Compile: Tizen
### 4.1. Prerequisite
Set up a gbs build environment on a Ubuntu machine.

### 4.2. Build with gbs command
The following command builds Tizen rpms.

```sh
$ gbs -c .github/gbs.conf build -A arm7l
```

If you are checking out a lwnode development branch from github repo (which includes submodules), add the following two options when building Tizen rpms on a local Linux machine.

```sh
$ gbs -c .github/gbs.conf build -A arm7l --include-all --incremental
```

### 4.3. Install a lwnode .so library
You can optionally install `lwnode-devel.rpm` to get  `liblwnode.so` library.
```sh
$ rpm -Uvh lwnode-devel.rpm
```
