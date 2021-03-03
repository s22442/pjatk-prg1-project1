/*
console NBP currency converter

MIT License

Copyright (c) 2021 Kajetan Welc

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <currency_converter.h>


auto program_args_to_string(int argc, char* argv[]) -> std::string
{
    auto str = std::string{""};

    for (auto i = 1; i < argc; i++) {
        str += argv[i];

        if (i + 1 < argc) {
            str += " ";
        }
    }

    return str;
}


auto main(int argc, char* argv[]) -> int
{
    auto cc = currency_converter{};

    if (argc == 1) {
        cc.start();
    } else {
        auto const line = program_args_to_string(argc, argv);
        cc.read_command_line(line);
    }

    return 0;
}
