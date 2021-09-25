# libKitsunemimiHanamiMessaging

## Build

### Requirements

name | repository | version | task
--- | --- | --- | ---
g++ | g++ | >= 6.0 | Compiler for the C++ code.
make | make | >= 4.0 | process the make-file, which is created by qmake to build the programm with g++
qmake | qt5-qmake | >= 5.0 | This package provides the tool qmake, which is similar to cmake and create the make-file for compilation.
FLEX | flex | >= 2.6 | Build the lexer-code for all used parser.
GNU Bison | bison | 3.x | Build the parser-code together with the lexer-code.
ssl library | libssl-dev | >= 1.1 | encryption for tls connections


Installation on Ubuntu/Debian:

```bash
sudo apt-get install g++ make qt5-qmake flex bison  libssl-dev
```

IMPORTANT: All my projects are only tested on Linux. 

### Kitsunemimi-repositories

Repository-Name | Version-Tag | Download-Path
--- | --- | ---
libKitsunemimiCommon | v0.18.0 |  https://github.com/kitsudaiki/libKitsunemimiCommon.git
libKitsunemimiPersistence | v0.10.2 |  https://github.com/kitsudaiki/libKitsunemimiPersistence.git
libKitsunemimiJson | v0.10.6 |  https://github.com/kitsudaiki/libKitsunemimiJson.git
libKitsunemimiJinja2 | v0.8.1 |  https://github.com/kitsudaiki/libKitsunemimiJinja2.git
libKitsunemimiIni | v0.4.7 |  https://github.com/kitsudaiki/libKitsunemimiIni.git
libKitsunemimiConfig | v0.2.4 |  https://github.com/kitsudaiki/libKitsunemimiConfig.git
libKitsunemimiSakuraLang | v0.8.0 |  https://github.com/kitsudaiki/libKitsunemimiSakuraLang.git
libKitsunemimiNetwork | v0.6.6 |  https://github.com/kitsudaiki/libKitsunemimiNetwork.git
libKitsunemimiSakuraNetwork | v0.6.0 |  https://github.com/kitsudaiki/libKitsunemimiSakuraNetwork.git

HINT: These Kitsunemimi-Libraries will be downloaded and build automatically with the build-script below.

### build library

In all of my repositories you will find a `build.sh`. You only have to run this script. It doesn't required sudo, because you have to install required tool via apt, for example, by yourself. But if other projects from me are required, it download them from github and build them in the correct version too. This script is also use by the ci-pipeline, so its tested with every commit.


Run the following commands:

```
git clone https://github.com/kitsudaiki/libKitsunemimiHanamiMessaging.git
cd libKitsunemimiHanamiMessaging
./build.sh
cd ../result
```

It create automatic a `build` and `result` directory in the directory, where you have cloned the project. At first it build all into the `build`-directory and after all build-steps are finished, it copy the include directory from the cloned repository and the build library into the `result`-directory. So you have all in one single place.

Tested on Debian and Ubuntu. If you use Centos, Arch, etc and the build-script fails on your machine, then please write me a mail and I will try to fix the script.


## Usage

(sorry, docu comes later)


## Contributing

Please give me as many inputs as possible: Bugs, bad code style, bad documentation and so on.

## License

This project is licensed under the Apache License Version 2.0 - see the [LICENSE](LICENSE) file for details

