## Building from sources

### Dependencies
* ```libopenssl```

Windows build:
```bash
cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER="i686-w64-mingw32-gcc.exe" -DCMAKE_CXX_COMPILER="i686-w64-mingw32-g++.exe" -DCMAKE_MAKE_PROGRAM="mingw32-make.exe"

mingw32-make.exe
```

Linux build:
```
```