Linux build:
 Ubuntu:
 > sudo apt-get update
 > sudo apt-get install libssl-dev libcurl4-openssl-dev
Windows build:
install OpenSSL (32bit) https://slproweb.com/products/Win32OpenSSL.html 
> cd build
> cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER="i686-w64-mingw32-gcc.exe" -DCMAKE_CXX_COMPILER="i686-w64-mingw32-g++.exe" -DCMAKE_MAKE_PROGRAM="mingw32-make.exe"
> mingw32-make.exe