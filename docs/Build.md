# How to build lwnode

## 1. Supported Platforms
* Ubuntu 16.04 and above
* Tizen 4.0 and above

## 2. Checkout code
```sh
$ git clone git@github.com:Samsung/lwnode.git
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
$ ./out/linux/Release/lwnode ./deps/node/test/message/hello_world.js
```

```sh
# for debug build
$ ./configure.py --debug
$ ninja -C out/linux/Debug lwnode
$ ./out/linux/Debug/lwnode ./deps/node/test/message/hello_world.js
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
[Install GBS tools](https://docs.tizen.org/platform/developing/installing/) on a Linux machine.

### 4.2. Build with gbs command
The following command builds Tizen RPMs.

```sh
$ gbs -c .github/gbs.conf build -A arm7l
```

If you are checking out `lwnode` from the GitHub repo (which includes submodules), add the following two options when building Tizen RPMs on your Linux machine.

```sh
$ gbs -c .github/gbs.conf build -A arm7l --include-all --incremental
```

#### Build Options
Use option with `--define '<option_key> <option_value>'`.

For example, If you want to build to static type,
```sh
$ gbs -c .github/gbs.conf build -A arm7l --include-all --incremental --define 'lib_type static'
```

Options list:
`lib_type` : shared(default)|static

#### Building a package with a revision string

By default, lwnode binary contains revision information based on a git commit
id. This requires `git` in your build environment, but it is not available in
GBS. In this case, you can manually set revision information using the following
option.

```console
$ gbs ... --define 'revision $(git rev-parse --short HEAD)'
```

The following also indicates whether the git repository is modifed or not.
```console
$ gbs ... --define 'revision $(git rev-parse --short HEAD)$(git diff --quiet --exit-code || echo +)'
```

The revision information can also be found in `process.config`, as shown below.

```console
$ lwnode -e 'console.log(process.config.variables.lwnode_revision)'
```
