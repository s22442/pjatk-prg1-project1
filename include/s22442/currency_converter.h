// Author: Kajetan Welc - s22442

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

    std::map<std::string, json> const HELP_OBJECTS{
        {"AUTHOR",
         {{"template", "author"},
          {"description", "print the author of the program"}}},
        {"DATE",
         {{"template", "date"},
          {"description", "print publication date of the exchange rates"}}},
        {"EXIT", {{"template", "exit"}, {"description", "exit the program"}}},
        {"LOGO",
         {{"template", "logo"}, {"description", "print the program logo"}}},
        {"TABLE",
         {{"template", "table BASE_CURRENCY_CODE [OPTIONS...]"},
          {"description",
           "print exchange rate table for the selected base currency"},
          {"options",
           json::array(
               {json::object({{"template", "to TARGET_CURRENCY_CODES..."},
                              {"description",
                               "limit the table target currencies to the "
                               "selected ones"}}),
                json::object(
                    {{"template", "-n, --name-currencies [LANGUAGE_CODE]"},
                     {"description",
                      "add currency names of the selected language code to "
                      "the table. If no code is present, the default "
                      "language is used"}})})}}},
        {"TO",
         {{"template",
           "BASE_CURRENCY_CODES... to TARGET_CURRENCY_CODE [OPTIONS...]"},
          {"description",
           "print the sum of the base currencies in the target currency"},
          {"options",
           json::array(
               {json::object(
                    {{"template", "-n, --name-currencies [LANGUAGE_CODE]"},
                     {"description",
                      "add currency names of the selected language code to "
                      "the output. If no code is present, the default "
                      "language is used"}}),
                json::object({{"template", "-r, --result-only"},
                              {"description",
                               "print only the result value. Cannot be used "
                               "with option -n, --name-currencies"}})})}}},
        {"UPDATE",
         {{"template", "update [OPTIONS...]"},
          {"description", "update the entire currency converter database"},
          {"options",
           json::array({json::object({{"template", "-s, --silent-mode"},
                                      {"description", "print nothing"}})})}}}};

    std::string const DEFAULT_LANGUAGE      = "EN";
    int const DEFAULT_DECIMAL_POINTS_NUMBER = 4;

    bool awaits_commands = false;
    std::map<std::string, float> exchange_rates{{"PLN", 1}};
    std::map<std::string, std::map<std::string, std::string>> currency_names;
    std::string rates_publication_date;
    std::vector<std::string> error_strings;

    enum class color { blue, cyan, green, light, red, yellow };

    auto print(std::string const& str, color const& c = color::light) -> void
    {
#if defined(__APPLE__) || defined(__unix__) || defined(__unix)
        switch (c) {
        case color::blue:
            std::cout << termcolor::color<50, 180, 255>;
            break;
        case color::cyan:
            std::cout << termcolor::color<0, 255, 255>;
            break;
        case color::green:
            std::cout << termcolor::color<0, 255, 0>;
            break;
        case color::light:
            std::cout << termcolor::color<248, 249, 250>;
            break;
        case color::red:
            std::cout << termcolor::color<255, 60, 60>;
            break;
        case color::yellow:
            std::cout << termcolor::color<255, 255, 0>;
            break;
        default:
            break;
        }
#elif defined(_WIN32) || defined(_WIN64)
        switch (c) {
        case color::blue:
            std::cout << termcolor::blue;
            break;
        case color::cyan:
            std::cout << termcolor::cyan;
            break;
        case color::green:
            std::cout << termcolor::green;
            break;
        case color::light:
            std::cout << termcolor::white;
            break;
        case color::red:
            std::cout << termcolor::red;
            break;
        case color::yellow:
            std::cout << termcolor::yellow;
            break;
        default:
            break;
        }
#endif

        std::cout << str << termcolor::reset;
    }

    template<typename... Args>
    auto print(std::string const& str, color const& c, Args... args) -> void
    {
        print(str, c);
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

    auto string_capitalize_words(std::string str) -> std::string
    {
        auto capitalize_next_char = bool{true};

        for (auto& character : str) {
            if (capitalize_next_char) {
                character = std::toupper(character);

                capitalize_next_char = false;
                continue;
            }

            if (character == ' ') {
                capitalize_next_char = true;
            }
        }

        return str;
    }

    template<typename T>
    auto vector_index_of(std::vector<T> const& v, T const& element_to_find)
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

    auto float_to_fixed(float const& number, int const& decimal_points) -> float
    {
        auto const multiplier = (int)std::pow(10, decimal_points);

        auto result_number = float{};

        result_number = (int)(number * multiplier + .5);
        result_number = result_number / multiplier;

        return result_number;
    }

    auto float_to_fixed_to_string(float const& number,
                                  int const& decimal_points = 2) -> std::string
    {
        std::stringstream tmp_ss;
        tmp_ss << std::fixed << std::setprecision(decimal_points)
               << float_to_fixed(number, decimal_points);

        return tmp_ss.str();
    }

    auto parse_json(std::string const& str,
                    std::string const parse_error_string = "JSON parse error")
        -> json
    {
        auto parsed_data = json{};

        try {
            parsed_data = json::parse(str);
        } catch (nlohmann::detail::parse_error const&) {
            error_strings.push_back(parse_error_string);
        } catch (std::exception const& e) {
            error_strings.push_back(e.what());
        }

        return parsed_data;
    }

    auto print_error_strings() -> void
    {
        print("Problems occurred: ", color::red);

        auto const error_strings_size = (int)error_strings.size();
        auto error_string_index       = int{0};
        for (auto const& str : error_strings) {
            print(str, color::red);

            if (error_string_index < error_strings_size - 1) {
                print(", ", color::red);
            }

            error_string_index++;
        }

        print("\n");
    }

    auto print_incorrect_command_usage_string(std::string const& command)
        -> void
    {
        print("Incorrect usage of \"" + command + "\" command\n", color::red);
        print_help_entry(string_to_uppercase(command));
        print("\n");
    }

    auto set_currency_names(std::string const& language_code,
                            json const& names_obj) -> void
    {
        for (auto const& [currency, name] : names_obj.items()) {
            currency_names[language_code][currency] =
                string_capitalize_words(name.get<std::string>());
        }
    }

    auto set_exchange_rates(json const& nbp_json) -> void
    {
        rates_publication_date = nbp_json[0]["effectiveDate"];

        auto pl_currency_names = json{{"PLN", "Polski złoty"}};

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

    auto is_correct_language(std::string const& str) -> bool
    {
        return currency_names.count(str);
    }

    auto convert_currency(float const& input_value,
                          std::string const& input_currency,
                          std::string const& target_currency) -> float
    {
        auto const value_in_PLN = input_value * exchange_rates[input_currency];
        return value_in_PLN / exchange_rates[target_currency];
    }

    auto print_logo() -> void
    {
#if defined(__APPLE__) || defined(__unix__) || defined(__unix)
        std::cout << termcolor::on_color<0, 105, 95>     //
                  << termcolor::color<248, 249, 250>     //
                  << " NBP "                             // light on dark green
                  << termcolor::reset                    // reset
                  << termcolor::on_color<248, 249, 250>  //
                  << termcolor::color<0, 105, 95>        //
                  << " currency converter "              // dark green on light
                  << termcolor::color<0, 123, 255>       //
                  << "by Kajetan Welc "                  // dark blue on light
                  << termcolor::reset                    // reset
                  << "\n";
#elif defined(_WIN32) || defined(_WIN64)
        std::cout << termcolor::on_green << termcolor::white  //
                  << " NBP "                                  // white on green
                  << termcolor::reset                         // reset
                  << termcolor::on_white << termcolor::green  //
                  << " currency converter "                   // green on white
                  << termcolor::blue                          //
                  << "by Kajetan Welc "                       // blue on white
                  << termcolor::reset                         // reset
                  << "\n";
#endif
    }

    auto print_author() -> void
    {
        print("Kajetan Welc - s22442\n");
    }

    auto print_help_entry(std::string const& command) -> void
    {
        auto const& entry = HELP_OBJECTS.at(command);

        print("\n");
        print(entry["template"].get<std::string>() + "\n", color::yellow);
        print("  " + entry["description"].get<std::string>() + "\n");

        if (entry.contains("options")) {
            print("  Options:\n");
            for (auto const& each : entry["options"]) {
                print("  " + each["template"].get<std::string>() + "\n",
                      color::yellow);
                print("    " + each["description"].get<std::string>() + "\n");
            }
        }
    }

    auto print_help(std::vector<std::string> const& args) -> void
    {
        auto const has_no_args = bool{args.size() == 1};

        std::vector<std::string> help_args;
        if (has_no_args) {
            for (auto const& [command, obj] : HELP_OBJECTS) {
                if (command == "EXIT" && !awaits_commands) {
                    continue;
                }

                help_args.push_back(command);
            }
        } else {
            help_args = std::vector<std::string>(args.begin() + 1, args.end());
        }

        std::vector<std::string> unknown_commands;

        for (auto const& arg : help_args) {
            if (HELP_OBJECTS.count(arg)) {
                print_help_entry(arg);
            } else {
                unknown_commands.push_back(arg);
            }
        }

        if (help_args.size() != unknown_commands.size()) {
            print("\n");
        }

        if (!unknown_commands.empty()) {
            print("No help entries for: ", color::red);

            auto const unknown_commands_size = (int)unknown_commands.size();
            auto unknown_command_index       = int{0};
            for (auto const& each : unknown_commands) {
                print(each, color::red);

                if (unknown_command_index < unknown_commands_size - 1) {
                    print(", ", color::red);
                }

                unknown_command_index++;
            }
            print("\n");
        }
    }

    auto print_publication_date() -> void
    {
        print(rates_publication_date + "\n");
    }

    auto update_data(std::vector<std::string> const& args) -> void
    {
        auto silent_mode = bool{false};

        if (args.size() > 1) {
            if (args.size() > 2
                || (args[1] != "-S" && args[1] != "--SILENT-MODE")) {
                print_incorrect_command_usage_string("update");
                return;
            }
            silent_mode = true;
        }

        fetch_data();

        if (error_strings.empty()) {
            if (!silent_mode) {
                print("Data update successful!\n", color::green);
            }
            return;
        }

        if (!silent_mode) {
            print("Fetching data has failed!\n", color::red);
            print_error_strings();
            print("Please try again later...\n", color::red);
        }
        error_strings.clear();
    }

    auto print_currency_conversion(std::vector<std::string> const& args) -> void
    {
        auto const args_size = (int)args.size();

        auto const command_index = vector_index_of(args, std::string{"TO"});

        auto result_only_parameter_index =
            vector_index_of(args, std::string{"-R"});
        if (result_only_parameter_index == -1) {
            result_only_parameter_index =
                vector_index_of(args, std::string{"--RESULT-ONLY"});
        }

        auto name_currencies_index = vector_index_of(args, std::string{"-N"});
        if (name_currencies_index == -1) {
            name_currencies_index =
                vector_index_of(args, std::string{"--NAME-CURRENCIES"});
        }

        auto const print_result_only = bool{result_only_parameter_index != -1};
        auto const print_currency_names = bool{name_currencies_index != -1}; // TODO

        if (!command_index
            || (!print_result_only && command_index + 2 < args_size)
            || (print_result_only
                && result_only_parameter_index != args_size - 1)) {
            print_incorrect_command_usage_string("to");
            return;
        }

        std::vector<std::string> unknown_currency_codes;

        auto const& target_currency = print_result_only ? args[args_size - 2]
                                                        : args[args_size - 1];
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
                        print_incorrect_command_usage_string("to");
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
            print("Unknown currency codes: ", color::red);
            auto const unknown_currency_codes_size =
                (int)unknown_currency_codes.size();
            auto currency_index = int{0};
            for (auto const& currency : unknown_currency_codes) {
                print(currency, color::red);

                if (currency_index + 1 < unknown_currency_codes_size) {
                    print(", ", color::red);
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

        auto const result_value_string = float_to_fixed_to_string(
            result_value, DEFAULT_DECIMAL_POINTS_NUMBER);

        if (print_result_only) {
            print(result_value_string + "\n");
        } else {
            auto const input_currencies_size = (int)input_currencies.size();
            auto currency_index              = int{0};
            for (auto const& [currency, value] : input_currencies) {
                auto const value_string = float_to_fixed_to_string(
                    value, DEFAULT_DECIMAL_POINTS_NUMBER);

                print(value_string + " " + currency);

                if (currency_index == input_currencies_size - 1) {
                    print(" = ");
                } else {
                    print(" + ");
                }

                currency_index++;
            }
            print(result_value_string,
                  color::cyan,
                  " " + target_currency + "\n",
                  color::blue);
        }
    }

    auto make_currency_table(std::string const& base_currency,
                             std::vector<std::string> const& target_currencies,
                             std::string const& currency_names_language)
        -> fort::utf8_table
    {
        auto const show_currency_names = bool{!currency_names_language.empty()};

        fort::utf8_table table;
        table << fort::header;
        table << "Currency";
        if (show_currency_names) {
            table << "Name";
        }
        table << "Rate";
        table << fort::endr;

        for (auto const& currency : target_currencies) {
            if (currency == base_currency) {
                continue;
            }

            table << currency;
            if (show_currency_names) {
                if (currency_names[currency_names_language].count(currency)) {
                    table << currency_names[currency_names_language][currency];
                } else {
                    table << "";
                }
            }

            auto const rate = convert_currency(1, currency, base_currency);

            table << float_to_fixed_to_string(rate,
                                              DEFAULT_DECIMAL_POINTS_NUMBER);
            table << fort::endr;
        }

#if defined(__APPLE__) || defined(__unix__) || defined(__unix)
        table.set_border_style(FT_BOLD2_STYLE);
#elif defined(_WIN32) || defined(_WIN64)
        table.set_border_style(FT_BASIC2_STYLE);
#endif

        auto rates_column = int{1};
        if (show_currency_names) {
            rates_column = 2;
            table.column(1).set_cell_text_align(fort::text_align::left);
            table[0][1].set_cell_text_align(fort::text_align::center);
        }
        table.column(0).set_cell_text_align(fort::text_align::center);
        table.column(rates_column).set_cell_text_align(fort::text_align::left);
        table.column(rates_column).set_cell_left_padding(1);
        table.column(rates_column).set_cell_right_padding(1);
        table[0][rates_column].set_cell_text_align(fort::text_align::center);
#if defined(__APPLE__) || defined(__unix__) || defined(__unix)
        table.row(0).set_cell_content_fg_color(fort::color::cyan);
#elif defined(_WIN32) || defined(_WIN64)
        table.row(0).set_cell_content_fg_color(fort::color::light_blue);
#endif

        return table;
    }

    auto print_currency_table(std::vector<std::string> const& args) -> void
    {
        auto const args_size = (int)args.size();

        if (args_size == 1) {
            print_incorrect_command_usage_string("table");
            return;
        }

        auto const& base_currency = args[1];
        if (!is_correct_currency_code(base_currency)) {
            print("Unknown currency code: " + base_currency + "\n", color::red);
            return;
        }

        auto name_currencies_parameter_index =
            vector_index_of(args, std::string{"-N"});
        if (name_currencies_parameter_index == -1) {
            name_currencies_parameter_index =
                vector_index_of(args, std::string{"--NAME-CURRENCIES"});
        }

        auto currency_names_language = std::string{};

        auto to_parameter_index = vector_index_of(args, std::string{"TO"});

        if (args_size > 2 && name_currencies_parameter_index != 2
            && to_parameter_index != 2) {
            print_incorrect_command_usage_string("table");
            return;
        }

        // --NAME-CURRENCIES parameter
        if (name_currencies_parameter_index != -1) {
            if (to_parameter_index == name_currencies_parameter_index - 1) {
                print_incorrect_command_usage_string("table");
                return;
            }

            if (args_size - 1 == name_currencies_parameter_index
                || to_parameter_index - 1 == name_currencies_parameter_index) {
                currency_names_language = DEFAULT_LANGUAGE;
            } else {
                auto const lang_index = name_currencies_parameter_index + 1;

                if (is_correct_language(args[lang_index])) {
                    currency_names_language = args[lang_index];
                } else {
                    print("Unknown language: " + args[lang_index] + "\n",
                          color::red);
                    return;
                }

                if (lang_index < args_size - 1
                    && to_parameter_index < name_currencies_parameter_index) {
                    print_incorrect_command_usage_string("table");
                    return;
                }
            }
        }

        // TO parameter
        std::vector<std::string> target_currencies;
        if (to_parameter_index != -1) {
            if (to_parameter_index == args_size - 1) {
                print_incorrect_command_usage_string("table");
                return;
            }

            std::vector<std::string> unknown_currency_codes;
            auto const last_target_currency_index =
                int{to_parameter_index < name_currencies_parameter_index
                        ? name_currencies_parameter_index - 1
                        : args_size - 1};
            for (auto i = to_parameter_index + 1;
                 i <= last_target_currency_index;
                 i++) {
                if (is_correct_currency_code(args[i])) {
                    target_currencies.push_back(args[i]);
                } else {
                    unknown_currency_codes.push_back(args[i]);
                }
            }

            if (!unknown_currency_codes.empty()) {
                print("Unknown currency codes: ", color::red);
                auto const unknown_currency_codes_size =
                    (int)unknown_currency_codes.size();
                auto currency_index = int{0};
                for (auto const& currency : unknown_currency_codes) {
                    print(currency, color::red);

                    if (currency_index + 1 < unknown_currency_codes_size) {
                        print(", ", color::red);
                    }

                    currency_index++;
                }
                print("\n");
                return;
            }
        } else {
            for (auto const& [currency, rate] : exchange_rates) {
                target_currencies.push_back(currency);
            }
        }

        auto const table = make_currency_table(
            base_currency, target_currencies, currency_names_language);

        print(table.to_string() + "\n");
    }

    auto await_commands() -> void
    {
        awaits_commands = true;

        while (error_strings.empty() && awaits_commands) {
            auto line = std::string{};

            print("> ");
            std::getline(std::cin, line);

            if (line.empty()) {
                continue;
            }

            read_command_line(std::move(line));
        }
    }

  public:
    currency_converter()
    {
        fetch_data();
    }

    auto read_command_line(std::string line) -> void
    {
        line      = string_to_uppercase(line);
        auto args = string_to_vector(line);

        if (args[0] == "HELP") {
            print_help(args);
            return;
        }

        if (args[0] == "AUTHOR") {
            if (args.size() == 1) {
                print_author();
            } else {
                print_incorrect_command_usage_string("author");
            }
            return;
        }

        if (args[0] == "DATE") {
            if (args.size() == 1) {
                print_publication_date();
            } else {
                print_incorrect_command_usage_string("date");
            }
            return;
        }

        if (args[0] == "EXIT") {
            if (args.size() == 1) {
                stop();
                print("Bye!\n");
            } else {
                print_incorrect_command_usage_string("exit");
            }
            return;
        }

        if (args[0] == "LOGO") {
            if (args.size() == 1) {
                print_logo();
            } else {
                print_incorrect_command_usage_string("logo");
            }
            return;
        }

        if (args[0] == "TABLE") {
            print_currency_table(args);
            return;
        }

        if (vector_index_of(args, std::string{"TO"}) != -1) {
            print_currency_conversion(args);
            return;
        }

        if (args[0] == "UPDATE") {
            update_data(args);
            return;
        }

        print("Syntax error\n",
              color::red,
              "Type \"help\" to see the complete list of commands\n",
              color::red);
    }

    auto start() -> void
    {
        if (!error_strings.empty()) {
            print_error_strings();
            return;
        }

        if (awaits_commands) {
            print("The currency converter has already started\n", color::red);
            return;
        }

        print_logo();
        print("Type \"help\" to see the complete list of commands\n");

        await_commands();
    }

    auto stop() -> void
    {
        awaits_commands = false;
    }
};
}  // namespace s22442

#endif
