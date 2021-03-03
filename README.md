# console NBP currency converter

Simple console currency converter for Linux/Windows made as a student project. <br />
API used by the application:

- [NBP Web API](http://api.nbp.pl/) (for exchange rates)
- [Open Exchange Rates - currencies.json](https://docs.openexchangerates.org/docs/currencies-json) (for english currency names)

## Build:

```
make build/main.bin
```

## Run:

```
./build/main.bin
```

#### or:

```
./build/main.bin COMMAND
```

#### e.g.:

```
./build/main.bin help
./build/main.bin 10 eur to usd
./build/main.bin table usd
```

## Libraries used:

- [C++ Requests](https://github.com/whoshuu/cpr)
- [libcurl](https://curl.se/libcurl/)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [Termcolor](https://github.com/ikalnytskyi/termcolor)
- [libfort](https://github.com/seleznevae/libfort)
