[Back](../README.md)

# Downloads (pre-built binaries)

[Link](https://drive.google.com/open?id=1Ib5gs0VIJdjbKYZgtN78duxFT7A_cc05)

#### Supported platforms

* Win32
* Win64

#### Required tools

* cmake
* C++17 compiler support (Visual Studio C++)
* Delphi 10.3

#### Building project

32-bit version:
```bash
mkdir -p ../build
cd ../build
cmake -A Win32 ..
cmake --build . --config Release
```

64-bit version:
```bash
mkdir -p ../build
cd ../build
cmake ..
cmake --build . --config Release
```

#### After successful build

1. Depending on cmake build either jetbeep_i386.dll or jetbeep_x86_64.dll will be produced in this folder
2. Open jetbeep.groupproj in Embarcadero RAD Studio, choose correct architecture (32 or 64 bit)
3. Build and run!

#### Example documentation

[Link](example/README.md)