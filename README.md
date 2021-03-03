# console NBP currency converter

Simple console currency converter for Linux/Windows made as a student project.

APIs used by the application:

- [NBP Web API](http://api.nbp.pl/) (for exchange rates)
- [Open Exchange Rates - currencies.json](https://docs.openexchangerates.org/docs/currencies-json) (for english currency names)

## Build:

```make
make build/main.bin
```

## Run:

```bash
./build/main.bin
```

#### or:

```bash
./build/main.bin COMMAND
```

## Example commands:

- print the complete list of commands

```text
help
```

- print publication date of the loaded exchange rates

```bash
date
# 2021-03-03
```

- print value of 1 EUR in USD

```bash
eur to usd
# 1.0000 EUR => 1.2102 USD
```

- print value of 10 EUR in USD

```bash
10 eur to usd
# 10.0000 EUR => 12.1019 USD
```

- print value of 10 EUR and 99 RUB in USD

```bash
10 eur 99 rub to usd
# 10.0000 EUR + 99.0000 RUB => 13.4453 USD
```

- print exchange rate table for the base currency of PLN and the target currencies of JPY, EUR, RUB, USD

```bash
table pln to jpy eur rub usd
# ┏━━━━━━━━━━┯━━━━━━━━┓
# ┃ Currency │  Rate  ┃
# ┣━━━━━━━━━━┿━━━━━━━━┫
# ┃   JPY    │ 0.0351 ┃
# ┠──────────┼────────┨
# ┃   EUR    │ 4.5393 ┃
# ┠──────────┼────────┨
# ┃   RUB    │ 0.0509 ┃
# ┠──────────┼────────┨
# ┃   USD    │ 3.7509 ┃
# ┗━━━━━━━━━━┷━━━━━━━━┛
```

## Libraries used:

- [C++ Requests](https://github.com/whoshuu/cpr)
- [libcurl](https://curl.se/libcurl/)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [Termcolor](https://github.com/ikalnytskyi/termcolor)
- [libfort](https://github.com/seleznevae/libfort)

## License

[MIT License](https://opensource.org/licenses/MIT)

Copyright (c) 2021 Kajetan Welc

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
