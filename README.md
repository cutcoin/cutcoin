# CUT coin

Copyright (c) 2018-2020 CUT coin.

Copyright (c) 2014-2017 The Monero Project.

Portions Copyright (c) 2012-2013 The Cryptonote developers.

## Content

  - [Development resources](#development-resources)
  - [Research](#research)
  - [Translations](#translations)
  - [Introduction](#introduction)
  - [About this project](#about-this-project)
  - [Supporting the project](#supporting-the-project)
  - [License](#license)
  - [Contributing](#contributing)
  - [Release staging schedule and protocol](#release-staging-schedule-and-protocol)
  - [Compiling Cutcoin from source](#compiling-monero-from-source)
    - [Dependencies](#dependencies)
  - [Internationalization](#Internationalization)
  - [Debugging](#Debugging)


## Development resources

- Web: [cutcoin.org](https://cutcoin.org)
- Forum: [https://bitcointalk.org/index.php?topic=5087550.msg48784779#msg48784779](https://bitcointalk.org/index.php?topic=5087550.msg48784779#msg48784779)
- Mail: [info@cutcoin.org](mailto:info@cutcoin.org)
- GitHub: [https://github.com/cutcoin](https://github.com/cutcoin)
- Documentation: [https://github.com/cutcoin/documentation](https://github.com/cutcoin/documentation)

## Research

Cutcoin Team has a solid conviction that the further advances in CryptoNote protocol development should be based on research work.
We do our own researches and utilize results that presented in respectable scientific sources.
If you have great ideas of how to make CryptoNote more reliable, secure and fast please contact us [info@cutcoin.org](mailto:info@cutcoin.org).

## Introduction

Concealed untraceable transactions coin that establishes a new level of privacy and usability.

## License

See our [LICENSE](LICENSE).

## Contributing

If you want to help out, see [CONTRIBUTING](CONTRIBUTING.md) for a set of guidelines. 
Our community's help is appreciated much and the most active members became Cutcoin Ambassadors, 
that gives different benefits.

## Compiling CUT coin from source

### Dependencies

The following table summarizes the tools and libraries required to build. A
few of the libraries are also included in this repository (marked as
"Vendored"). By default, the build uses the library installed on the system,
and ignores the vendored sources. However, if no library is found installed on
the system, then the vendored source will be built and used. The vendored
sources are also used for statically-linked builds because distribution
packages often include only shared library binaries (`.so`) but not static
library archives (`.a`).

| Dep          | Min. version  | Vendored | Debian/Ubuntu pkg  | Arch pkg     | Fedora            | Optional | Purpose        |
| ------------ | ------------- | -------- | ------------------ | ------------ | ----------------- | -------- | -------------- |
| GCC          | 4.7.3         | NO       | `build-essential`  | `base-devel` | `gcc`             | NO       |                |
| CMake        | 3.5           | NO       | `cmake`            | `cmake`      | `cmake`           | NO       |                |
| pkg-config   | any           | NO       | `pkg-config`       | `base-devel` | `pkgconf`         | NO       |                |
| Boost        | 1.58          | NO       | `libboost-all-dev` | `boost`      | `boost-devel`     | NO       | C++ libraries  |
| OpenSSL      | basically any | NO       | `libssl-dev`       | `openssl`    | `openssl-devel`   | NO       | sha256 sum     |
| libzmq       | 3.0.0         | NO       | `libzmq3-dev`      | `zeromq`     | `cppzmq-devel`    | NO       | ZeroMQ library |
| OpenPGM      | ?             | NO       | `libpgm-dev`       | `libpgm`     | `openpgm-devel`   | NO       | For ZeroMQ     |
| libunbound   | 1.4.16        | YES      | `libunbound-dev`   | `unbound`    | `unbound-devel`   | NO       | DNS resolver   |
| libsodium    | ?             | NO       | `libsodium-dev`    | `libsodium`  | `libsodium-devel` | NO       | cryptography   |
| libunwind    | any           | NO       | `libunwind8-dev`   | `libunwind`  | `libunwind-devel` | YES      | Stack traces   |
| liblzma      | any           | NO       | `liblzma-dev`      | `xz`         | `xz-devel`        | YES      | For libunwind  |
| libreadline  | 6.3.0         | NO       | `libreadline6-dev` | `readline`   | `readline-devel`  | YES      | Input editing  |
| ldns         | 1.6.17        | NO       | `libldns-dev`      | `ldns`       | `ldns-devel`      | YES      | SSL toolkit    |
| expat        | 1.1           | NO       | `libexpat1-dev`    | `expat`      | `expat-devel`     | YES      | XML parsing    |
| GTest        | 1.5           | YES      | `libgtest-dev`^    | `gtest`      | `gtest-devel`     | YES      | Test suite     |
| Doxygen      | any           | NO       | `doxygen`          | `doxygen`    | `doxygen`         | YES      | Documentation  |
| Graphviz     | any           | NO       | `graphviz`         | `graphviz`   | `graphviz`        | YES      | Documentation  |


[^] On Debian/Ubuntu `libgtest-dev` only includes sources and headers. You must
build the library binary manually. This can be done with the following command 

```sudo apt-get install libgtest-dev && cd /usr/src/gtest && sudo cmake . && sudo make && sudo mv libg* /usr/lib/ ```

Debian / Ubuntu one liner for all dependencies  

``` sudo apt update && sudo apt install git build-essential cmake pkg-config libboost-all-dev libssl-dev libzmq3-dev libunbound-dev libsodium-dev libunwind8-dev liblzma-dev libreadline6-dev libldns-dev libexpat1-dev doxygen graphviz libpgm-dev```

Install all dependencies at once on macOS with the provided Brewfile: ```brew update && brew bundle --file=contrib/brew/Brewfile```

FreeBSD one liner for required to build dependencies ```pkg install git gmake cmake pkgconf boost-libs cppzmq libsodium```

Debug mode build also requires QT4, on Debian / Ubuntu

``` sudo apt install qt4-default ```

### Cloning the repository

Clone recursively to pull-in needed submodule(s):

`$ git clone --recursive https://github.com/cutcoin/cutcoin`

If you already have a repo cloned, initialize and update:

`$ cd cutcoin && git submodule init && git submodule update`

### Build instructions

CUT coin uses the CMake build system and a top-level [Makefile](Makefile) that
invokes cmake commands as needed.

#### On Linux and OS X

* Install the dependencies
* Change to the root of the source code directory, change to the most recent release branch, and build:

        cd cutcoin
        git checkout master
        make

    *Optional*: If your machine has several cores and enough memory, enable
    parallel build by running `make -j<number of threads>` instead of `make`. For
    this to be worthwhile, the machine should have one core and about 2GB of RAM
    available per thread.

    *Note*: If cmake can not find zmq.hpp file on OS X, installing `zmq.hpp` from
    https://github.com/zeromq/cppzmq to `/usr/local/include` should fix that error.
    

* The resulting executables can be found in `build/release/bin`

* Add `PATH="$PATH:$HOME/cutcoin/build/release/bin"` to `.profile`

* Run Cutcoin with `cutcoind --detach`

* **Optional**: build and run the test suite to verify the binaries:

        make release-test

    *NOTE*: `core_tests` test may take a few hours to complete.

* **Optional**: to build binaries suitable for debugging:

         make debug

* **Optional**: to build statically-linked binaries:

         make release-static

Dependencies need to be built with -fPIC. Static libraries usually aren't, so you may have 
to build them yourself with -fPIC. Refer to their documentation for how to build them.

* **Optional**: build documentation in `doc/html` (omit `HAVE_DOT=YES` if `graphviz` is not installed):

        HAVE_DOT=YES doxygen Doxyfile

#### On Windows:

Binaries for Windows are built on Windows using the MinGW toolchain within
[MSYS2 environment](https://www.msys2.org). The MSYS2 environment emulates a
POSIX system. The toolchain runs within the environment and *cross-compiles*
binaries that can run outside of the environment as a regular Windows
application.

**Preparing the build environment**

* Download and install the [MSYS2 installer](https://www.msys2.org), either the 64-bit or the 32-bit package, depending on your system.
* Open the MSYS shell via the `MSYS2 Shell` shortcut
* Update packages using pacman:  

    ```bash
    pacman -Syu
    ```

* Exit the MSYS shell using Alt+F4  
* Edit the properties for the `MSYS2 Shell` shortcut changing "msys2_shell.bat" to "msys2_shell.cmd -mingw64" for 64-bit builds or "msys2_shell.cmd -mingw32" for 32-bit builds
* Restart MSYS shell via modified shortcut and update packages again using pacman:  

    ```bash
    pacman -Syu
    ```


* Install dependencies:

    To build for 64-bit Windows:

    ```bash
    pacman -S mingw-w64-x86_64-toolchain make mingw-w64-x86_64-cmake mingw-w64-x86_64-boost mingw-w64-x86_64-openssl mingw-w64-x86_64-zeromq mingw-w64-x86_64-libsodium mingw-w64-x86_64-hidapi git
    ```

    To build for 32-bit Windows:

    ```bash
    pacman -S mingw-w64-i686-toolchain make mingw-w64-i686-cmake mingw-w64-i686-boost mingw-w64-i686-openssl mingw-w64-i686-zeromq mingw-w64-i686-libsodium mingw-w64-i686-hidapi git
    ```

* Open the MingW shell via `MinGW-w64-Win64 Shell` shortcut on 64-bit Windows
  or `MinGW-w64-Win64 Shell` shortcut on 32-bit Windows. Note that if you are
  running 64-bit Windows, you will have both 64-bit and 32-bit MinGW shells.

**Cloning**

* To git clone, run:

    ```bash
    git clone --recursive https://github.com/cutcoin/cutcoin.git
    ```

**Building**

* Change to the cloned directory, run:

    ```bash
    cd cutcoin
    ```

* If you are on a 64-bit system, run:

    ```bash
    make release-static-win64
    ```

* If you are on a 32-bit system, run:

    ```bash
    make release-static-win32
    ```

* The resulting executables can be found in `build/release/bin`

* **Optional**: to build Windows binaries suitable for debugging on a 64-bit system, run:

    ```bash
    make debug-static-win64
    ```

* **Optional**: to build Windows binaries suitable for debugging on a 32-bit system, run:

    ```bash
    make debug-static-win32
    ```

* The resulting executables can be found in `build/debug/bin`

## Running cutcoind

The build places the binary in `bin/` sub-directory within the build directory
from which cmake was invoked (repository root by default). To run in
foreground:

    ./bin/cutcoind

To list all available options, run `./bin/cutcoind --help`.  Options can be
specified either on the command line or in a configuration file passed by the
`--config-file` argument.  To specify an option in the configuration file, add
a line with the syntax `argumentname=value`, where `argumentname` is the name
of the argument without the leading dashes, for example `log-level=1`.

To run in background:

    ./bin/cutcoind --log-file cutcoind.log --detach

To run as a systemd service, copy
[cutcoind.service](utils/systemd/cutcoind.service) to `/etc/systemd/system/` and
[cutcoind.conf](utils/conf/cutcoind.conf) to `/etc/`. The [example
service](utils/systemd/cutcoind.service) assumes that the user `cutcoin` exists
and its home is the data directory specified in the [example
config](utils/conf/cutcoind.conf).

If you're on Mac, you may need to add the `--max-concurrency 1` option to
cutcoin-wallet-cli, and possibly cutcoind, if you get crashes refreshing.

## Internationalization

See [README.i18n.md](README.i18n.md).

## Using Tor

While CUT coin isn't made to integrate with Tor, it can be used wrapped with torsocks,
by setting the following configuration parameters and environment variables:

* `--p2p-bind-ip 127.0.0.1` on the command line or `p2p-bind-ip=127.0.0.1` in
  cutcoind.conf to disable listening for connections on external interfaces.
* `--no-igd` on the command line or `no-igd=1` in cutcoind.conf to disable IGD
  (UPnP port forwarding negotiation), which is pointless with Tor.
* `DNS_PUBLIC=tcp` or `DNS_PUBLIC=tcp://x.x.x.x` where x.x.x.x is the IP of the
  desired DNS server, for DNS requests to go over TCP, so that they are routed
  through Tor. When IP is not specified, cutcoind uses the default list of
  servers defined in [src/common/dns_utils.cpp](src/common/dns_utils.cpp).
* `TORSOCKS_ALLOW_INBOUND=1` to tell torsocks to allow cutcoind to bind to interfaces
   to accept connections from the wallet. On some Linux systems, torsocks
   allows binding to localhost by default, so setting this variable is only
   necessary to allow binding to local LAN/VPN interfaces to allow wallets to
   connect from remote hosts. On other systems, it may be needed for local wallets
   as well.
* Do NOT pass `--detach` when running through torsocks with systemd, (see
  [utils/systemd/cutcoind.service](utils/systemd/cutcoind.service) for details).
* If you use the wallet with a Tor daemon via the loopback IP (eg, 127.0.0.1:9050),
  then use `--untrusted-daemon` unless it is your own hidden service.

Example command line to start cutcoind through Tor:

    DNS_PUBLIC=tcp torsocks cutcoind --p2p-bind-ip 127.0.0.1 --no-igd

### Using Tor on Tails

TAILS ships with a very restrictive set of firewall rules. Therefore, you need
to add a rule to allow this connection too, in addition to telling torsocks to
allow inbound connections. Full example:

    sudo iptables -I OUTPUT 2 -p tcp -d 127.0.0.1 -m tcp --dport 18081 -j ACCEPT
    DNS_PUBLIC=tcp torsocks ./cutcoind --p2p-bind-ip 127.0.0.1 --no-igd --rpc-bind-ip 127.0.0.1 \
        --data-dir /home/amnesia/Persistent/your/directory/to/the/blockchain

## Debugging

This section contains general instructions for debugging failed installs or problems 
encountered with CUT coin. First ensure you are running the latest version built from 
the Github repo.

### Obtaining stack traces and core dumps on Unix systems

We generally use the tool `gdb` (GNU debugger) to provide stack trace functionality, 
and `ulimit` to provide core dumps in builds which crash or segfault.

* To use gdb in order to obtain a stack trace for a build that has stalled:

Run the build.

Once it stalls, enter the following command:

```
gdb /path/to/cutcoind `pidof cutcoind` 
```

Type `thread apply all bt` within gdb in order to obtain the stack trace

* If however the core dumps or segfaults:

Enter `ulimit -c unlimited` on the command line to enable unlimited filesizes for core dumps

Enter `echo core | sudo tee /proc/sys/kernel/core_pattern` to stop cores from being hijacked 
by other tools

Run the build.

When it terminates with an output along the lines of "Segmentation fault (core dumped)", 
there should be a core dump file in the same directory as cutcoind. It may be named just 
`core`, or `core.xxxx` with numbers appended.

You can now analyse this core dump with `gdb` as follows:

`gdb /path/to/cutcoind /path/to/dumpfile`

Print the stack trace with `bt`

* To run cutcoin within gdb:

Type `gdb /path/to/cutcoind`

Pass command-line options with `--args` followed by the relevant arguments

Type `run` to run cutcoind

### Analysing memory corruption

There are two tools available:

* ASAN

Configure CUT coin with the -D SANITIZE=ON cmake flag, eg:

    cd build/debug && cmake -D SANITIZE=ON -D CMAKE_BUILD_TYPE=Debug ../..

You can then run the cutcoin tools normally. Performance will typically halve.

* valgrind

Install valgrind and run as `valgrind /path/to/cutcoind`. It will be very slow.

### LMDB

Instructions for debugging suspected blockchain corruption as per @HYC

There is an `mdb_stat` command in the LMDB source that can print statistics about 
the database but it's not routinely built. This can be built with the following command:

`cd ~/cutcoin/external/db_drivers/liblmdb && make`

The output of `mdb_stat -ea <path to blockchain dir>` will indicate inconsistencies 
in the blocks, block_heights and block_info table.

The output of `mdb_dump -s blocks <path to blockchain dir>` and 
`mdb_dump -s block_info <path to blockchain dir>` is useful for indicating whether 
blocks and block_info contain the same keys.

These records are dumped as hex data, where the first line is the key and the second 
line is the data.
