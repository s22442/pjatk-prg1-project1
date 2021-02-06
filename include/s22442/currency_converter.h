
#ifndef S22442_CURRENCY_CONVERTER_H
#define S22442_CURRENCY_CONVERTER_H

#include <cpr/cpr.h>
#include <fort.hpp>
#include <nlohmann/json.hpp>
#include <termcolor/termcolor.hpp>

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
    std::map<std::string, cpr::Url> CURRENCY_NAMES_URLS{
        {"en", "openexchangerates.org/api/currencies.json"}};

    json exchange_rates;
    json currency_names;
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

    auto set_currency_names(std::string const language_code,
                            json const names_obj) -> void
    {
        currency_names[language_code] = names_obj;
    }

    auto set_exchange_rates(json const& nbp_json) -> void
    {
        rates_publication_date = nbp_json[0]["effectiveDate"];

        auto rates             = json{{"PLN", 1}};
        auto pl_currency_names = json::object();

        for (auto const& rate : nbp_json[0]["rates"]) {
            std::string const code = rate["code"];

            rates[code]             = rate["mid"];
            pl_currency_names[code] = rate["currency"];
        }

        exchange_rates = rates;

        set_currency_names("pl", std::move(pl_currency_names));
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
            auto const l = std::ref(lang);
            auto const u = std::ref(url);

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

    auto print_help(std::vector<std::string> const& args) -> void
    {
        auto const has_no_args = args.empty();

        if (has_no_args) {
            print("TODO - help\n");
        }

        auto const i = vector_index_of(args, std::string{"qwe"});
        if (has_no_args || i != -1) {
            print("qwe\n");
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

    auto await_commands() -> void
    {
        while (error_strings.empty()) {
            auto line = std::string{};

            print("> ");
            std::getline(std::cin, line);

            if (line.empty()) {
                continue;
            }

            if (line == "exit") {
                print("Bye!\n");
                return;
            }

            read_command_line(line);
        }

        print_error_strings();
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

  public:
    currency_converter()
    {
        fetch_data();
    }

    auto read_command_line(std::string const& line) -> void
    {
        std::stringstream tmp_line_ss(line);
        std::istream_iterator<std::string> const tmp_line_ss_begin(tmp_line_ss);
        std::istream_iterator<std::string> const tmp_line_ss_end;

        std::vector<std::string> args(tmp_line_ss_begin, tmp_line_ss_end);

        if (args[0] == "help") {
            args.erase(args.begin());

            print_help(args);
            return;
        }
    }

    auto start() -> void
    {
        if (!error_strings.empty()) {
            print_error_strings();
            return;
        }

        print("NBP currency converter",
              Text_color::cyan,
              " by Kajetan Welc\n",
              Text_color::magenta);
        print("Type ",
              "help",
              Text_color::yellow,
              " to see the complete list of commands\n");

        await_commands();
    }
};
}  // namespace s22442

#endif