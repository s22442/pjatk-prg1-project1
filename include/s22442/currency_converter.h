
#ifndef S22442_CURRENCY_CONVERTER_H
#define S22442_CURRENCY_CONVERTER_H

#include <cpr/cpr.h>  // https://github.com/whoshuu/cpr
#include <fort.hpp>   // https://github.com/seleznevae/libfort
#include <math.h>
#include <nlohmann/json.hpp>        // https://github.com/nlohmann/json
#include <termcolor/termcolor.hpp>  // https://github.com/ikalnytskyi/termcolor

#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::json;


namespace s22442 {

struct currency_converter {
  private:
    cpr::Url const NBP_URL =
        "api.nbp.pl/api/exchangerates/tables/a?format=json";
    std::map<std::string, cpr::Url> const CURRENCY_NAMES_URLS{
        {"EN", "openexchangerates.org/api/currencies.json"}};
    std::string const DEFAULT_LANGUAGE = "EN";

    std::map<std::string, float> exchange_rates{{"PLN", 1}};
    std::map<std::string, std::map<std::string, std::string>> currency_names;
    std::string rates_publication_date;
    std::vector<std::string> error_strings;

    enum class Text_color { red, yellow, cyan, blue, green, magenta, white };

    auto print(std::string const& str,
               Text_color const& color = Text_color::white) -> void
    {
        switch (color) {
        case Text_color::red:
            std::cout << termcolor::red;
            break;
        case Text_color::yellow:
            std::cout << termcolor::yellow;
            break;
        case Text_color::cyan:
            std::cout << termcolor::cyan;
            break;
        case Text_color::blue:
            std::cout << termcolor::blue;
            break;
        case Text_color::green:
            std::cout << termcolor::green;
            break;
        case Text_color::magenta:
            std::cout << termcolor::magenta;
            break;
        default:
            break;
        }

        std::cout << str << termcolor::white;
    }

    template<typename... Args>
    auto print(std::string const& str, Text_color const& color, Args... args)
        -> void
    {
        print(str, color);
        print(args...);
    }

    template<typename... Args>
    auto print(std::string const& str, std::string const& nextStr, Args... args)
        -> void
    {
        print(str);
        print(nextStr, args...);
    }

    auto string_to_vector(std::string const& str) -> std::vector<std::string>
    {
        std::stringstream tmp_ss(str);
        std::istream_iterator<std::string> const tmp_ss_begin(tmp_ss);
        std::istream_iterator<std::string> const tmp_ss_end;

        std::vector<std::string> v(tmp_ss_begin, tmp_ss_end);
        return v;
    }

    auto string_to_uppercase(std::string str) -> std::string
    {
        for (auto& character : str) {
            character = std::toupper(character);
        }

        return str;
    }

    template<typename T>
    auto vector_index_of(std::vector<T> const& v, T const element_to_find)
        -> int
    {
        auto i = int{0};
        for (auto const& each : v) {
            if (each == element_to_find) {
                return i;
            }

            i++;
        }
        return -1;
    }

    auto print_error_strings() -> void
    {
        print("Problems occurred: ", Text_color::red);

        for (auto const& e : error_strings) {
            print(e, Text_color::red);

            if (e != error_strings.back()) {
                print(", ", Text_color::red);
            }
        }

        print("\n");
    }

    auto parse_json(std::string const& str,
                    std::string const error_string = "JSON parse error") -> json
    {
        auto parsed_data = json{};

        try {
            parsed_data = json::parse(str);
        } catch (nlohmann::detail::parse_error const&) {
            error_strings.push_back(error_string);
        } catch (std::exception const& e) {
            error_strings.push_back(e.what());
        }

        return parsed_data;
    }

    auto set_currency_names(std::string const& language_code,
                            json const& names_obj) -> void
    {
        for (auto const& [currency, name] : names_obj.items()) {
            currency_names[language_code][currency] = name;
        }
    }

    auto set_exchange_rates(json const& nbp_json) -> void
    {
        rates_publication_date = nbp_json[0]["effectiveDate"];

        auto pl_currency_names = json::object();

        for (auto const& rate : nbp_json[0]["rates"]) {
            auto const code = rate["code"].get<std::string>();

            exchange_rates[code]    = rate["mid"].get<float>();
            pl_currency_names[code] = rate["currency"];
        }

        set_currency_names("PL", pl_currency_names);
    }

    auto fetch_data() -> void
    {
        cpr::Response nbp_response;
        std::map<std::string, cpr::Response> currency_names_responses;

        auto nbp_json = json::array();
        std::map<std::string, json> currency_names_jsons;

        auto nbp_thread =
            std::thread{[&] { nbp_response = cpr::Get(NBP_URL); }};
        std::vector<std::thread> currency_names_threads;
        for (auto const& [lang, url] : CURRENCY_NAMES_URLS) {
            auto const& l = lang;
            auto const& u = url;

            currency_names_threads.push_back(std::thread{
                [&] { currency_names_responses[l] = cpr::Get(u); }});
        }

        nbp_thread.join();
        for (auto& each : currency_names_threads) {
            each.join();
        }

        if (nbp_response.status_code >= 400) {
            error_strings.push_back("NBP HTTP request error");
        }
        for (auto const& [lang, response] : currency_names_responses) {
            if (response.status_code >= 400) {
                error_strings.push_back(lang
                                        + " currency names HTTP request error");
            }
        }

        if (!error_strings.empty()) {
            return;
        }

        nbp_json = parse_json(nbp_response.text, "NBP API parse error");
        for (auto const& [lang, response] : currency_names_responses) {
            currency_names_jsons[lang] = parse_json(
                response.text, lang + " currency names API parse error");
        }

        if (!error_strings.empty()) {
            return;
        }

        set_exchange_rates(nbp_json);

        for (auto const& [lang, obj] : currency_names_jsons) {
            set_currency_names(lang, obj);
        }
    }

    auto is_correct_currency_code(std::string const& str) -> bool
    {
        return exchange_rates.count(str);
    }

    auto convert_currency(float const& input_value,
                          std::string const& input_currency,
                          std::string const& target_currency) -> float
    {
        auto const value_in_PLN = input_value * exchange_rates[input_currency];
        return value_in_PLN / exchange_rates[target_currency];
    }

    auto print_help(std::vector<std::string> const& args) -> void  // TODO
    {
        auto const has_no_args = args.empty();

        if (has_no_args) {
            print("TODO - help\n");
        }

        auto const i = vector_index_of(args, std::string{"TO"});
        if (has_no_args || i != -1) {
            print("'to' help\n");
        }
    }

    auto update_data() -> void
    {
        fetch_data();

        if (error_strings.empty()) {
            print("Data update successful!\n", Text_color::green);
            return;
        }

        print("Fetching data has failed!\n", Text_color::red);

        print_error_strings();

        error_strings.clear();

        print("Please try again later...\n", Text_color::red);
    }

    /*auto print_currency_table() -> const void {
        fort::utf8_table table;
        table << fort::header << "code" << "currency" << "mid" << fort::endr;
        table << exchange_rates[0]["code"] << exchange_rates[0]["currency"] <<
    exchange_rates[0]["mid"] << fort::endr; table << exchange_rates[1]["code"]
    << exchange_rates[1]["currency"] << exchange_rates[1]["mid"] << fort::endr;
        table << exchange_rates[2]["code"] << exchange_rates[2]["currency"] <<
    exchange_rates[2]["mid"] << fort::endr; table << exchange_rates[3]["code"]
    << exchange_rates[3]["currency"] << exchange_rates[3]["mid"] << fort::endr;
        table << exchange_rates[4]["code"] << exchange_rates[4]["currency"] <<
    exchange_rates[4]["mid"] << fort::endr; std::cout << table.to_string() <<
    std::endl;
    }*/

    auto print_currency_conversion_syntax_error_string() -> void
    {
        print("Incorrect usage of the ",
              Text_color::red,
              "TO",
              Text_color::yellow,
              " command\n",
              Text_color::red);
        print_help({"TO"});
    }

    auto print_currency_conversion(std::vector<std::string>& args) -> void
    {
        auto const command_index = vector_index_of(args, std::string{"TO"});

        if (!command_index || command_index + 2 < args.size()) {
            print_currency_conversion_syntax_error_string();
            return;
        }

        std::vector<std::string> unknown_currency_codes;

        auto const& target_currency = args.back();
        if (!is_correct_currency_code(target_currency)) {
            unknown_currency_codes.push_back(target_currency);
        }

        std::map<std::string, float> input_currencies;
        {
            auto currency_index = int{0};
            while (currency_index < command_index) {
                auto currency = std::string{};
                auto value    = float{1};

                if (!is_correct_currency_code(args[currency_index])) {
                    try {
                        value = std::stof(args[currency_index]);
                    } catch (...) {
                        unknown_currency_codes.push_back(args[currency_index]);

                        currency_index++;
                        continue;
                    }

                    if (currency_index + 1 >= command_index) {
                        print_currency_conversion_syntax_error_string();
                        return;
                    }

                    if (is_correct_currency_code(args[currency_index + 1])) {
                        currency = args[currency_index + 1];

                        currency_index += 2;
                    } else {
                        unknown_currency_codes.push_back(
                            args[currency_index + 1]);

                        currency_index += 2;
                        continue;
                    }
                } else {
                    currency = args[currency_index];

                    currency_index++;
                }

                if (input_currencies[currency]) {
                    input_currencies[currency] += value;
                } else {
                    input_currencies[currency] = value;
                }
            }
        }


        if (!unknown_currency_codes.empty()) {
            print("Unknown currency codes: ", Text_color::red);
            auto currency_index = int{0};
            for (auto const& currency : unknown_currency_codes) {
                print(currency, Text_color::red);

                if (currency_index + 1 < unknown_currency_codes.size()) {
                    print(", ", Text_color::red);
                }

                currency_index++;
            }
            print("\n");
            return;
        }

        auto result_value = float{0};
        for (auto const& [currency, value] : input_currencies) {
            result_value += convert_currency(value, currency, target_currency);
        }

        print(std::to_string(result_value) + "\n");
    }

    auto await_commands() -> void
    {
        while (error_strings.empty()) {
            auto line = std::string{};

            print("> ");
            std::getline(std::cin, line);

            if (line.empty()) {
                continue;
            }

            line = string_to_uppercase(line);

            if (line == "EXIT") {
                print("Bye!\n");
                return;
            }

            read_command_line(line);
        }

        print_error_strings();
    }

  public:
    currency_converter()
    {
        fetch_data();
    }

    auto read_command_line(std::string const& line) -> void
    {
        auto args = string_to_vector(line);

        if (args[0] == "HELP") {
            args.erase(args.begin());

            print_help(args);
            return;
        }

        if (args[0] == "TABLE") {
            args.erase(args.begin());

            print_help(args);
            return;
        }

        if (vector_index_of(args, std::string{"TO"}) != -1) {
            print_currency_conversion(args);
            return;
        }

        print("Syntax error\n", Text_color::red);
    }

    auto start() -> void
    {
        if (!error_strings.empty()) {
            print_error_strings();
            return;
        }

        print("NBP currency converter",
              Text_color::green,
              " by Kajetan Welc\n",
              Text_color::blue);
        print("Type ",
              "HELP",
              Text_color::yellow,
              " to see the complete list of commands\n");

        await_commands();
    }
};
}  // namespace s22442

#endif