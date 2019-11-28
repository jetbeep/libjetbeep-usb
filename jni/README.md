[Back](../README.md)

# Downloads (pre-built binaries)

[Link](https://drive.google.com/open?id=1Ib5gs0VIJdjbKYZgtN78duxFT7A_cc05)

# Building from sources

#### Required 3rd-party libraries

* `libopenssl` - in order to make HTTPS requests to EasyPay backend
* `libcurl4-openssl` - optional, will fall-back to native implementation if not found

#### Required tools

* cmake
* Mingw-w64 (Windows only)
* C++17 compiler support
* Java Runtime
* mdpdf - for generating documentation (could be easily installed as: `npm i -g mdpdf`)

#### Building project

```bash
./gradlew build # use ./gradlew.bat on Windows
```

#### Running example (command-line) project

```bash
./gradlew run --console=plain # use ./gradlew.bat on Windows
```

#### Building distribution

```
./gradlew distZip # use ./gradlew.bat on Windows
```

Output files will be in `example/build/distributions/`

#### Example documentation

[Link](example/README.md)