
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <s22442/currency_converter.h>

#include <iostream>
#include <string>

using json = nlohmann::json;


auto main(int argc, char* argv[]) -> int
{
    auto cc = s22442::currency_converter{};

    if (argc == 1) {
        cc.start();
    } else {
        auto line = std::string{""};
        for (auto i = 1; i < argc; i++) {
            line += argv[i];
        }

        cc.read_command_line(line);
    }

    return 0;
}
