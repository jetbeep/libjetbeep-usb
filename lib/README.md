[Back](../README.md)

# Downloads (pre-built binaries)

[Link](https://drive.google.com/open?id=1Ib5gs0VIJdjbKYZgtN78duxFT7A_cc05)

# Building from sources

#### Required 3rd-party libraries

* `libopenssl` - in order to make HTTPS requests to EasyPay backend
* `libcurl4-openssl`

#### Required tools

* cmake
* Mingw-w64 (Windows only)
* C++17 compiler support

#### Building for Windows
```bash
mkdir ../build
cd ../build

cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER="i686-w64-mingw32-gcc.exe" -DCMAKE_CXX_COMPILER="i686-w64-mingw32-g++.exe" -DCMAKE_MAKE_PROGRAM="mingw32-make.exe"

mingw32-make.exe
```

#### Building for Linux & Mac OS

```bash
mkdir ../build
cd ../build
cmake ..
make
```

# Example projects

#### AutoDevice

For full-code examples please refer to ../lib/examples/autodevice folder

[main.cpp](../examples/autodevice/main.cpp)

[cmd.cpp](../examples/autodevice/cmd.cpp)

#### EasyPayBackend

For full-code examples please refer to ../lib/examples/token-payment folder

[main.cpp](../examples/token-payment/main.cpp)