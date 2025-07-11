#include <array>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iosfwd>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

class FileAnalyzer {
    std::ifstream file_;
    std::string file_name_;

public:
    FileAnalyzer(const std::string &name)
        : file_{name, std::ios::binary}, file_name_(name) {
        if (!file_)
            throw std::runtime_error{file_name_ +
                                     ": No such file or directory"};
    }

    const std::string &get_file_name() const & { return file_name_; }
    size_t count_bytes();
    size_t count_chars();
    size_t count_lines();
    size_t count_words();
    size_t max_line_length();
};

size_t utf8_length(const char *str) {
    size_t len{};
    while (*str)
        len += (*(str++) & 0xc0) != 0x80;
    return len;
}

size_t FileAnalyzer::count_bytes() {
    file_.clear();
    std::streampos start_pos = file_.tellg();

    file_.seekg(0, std::ios::end);
    std::streampos end_pos = file_.tellg();

    file_.clear();
    file_.seekg(start_pos);
    return static_cast<size_t>(end_pos);
}

size_t FileAnalyzer::count_chars() {
    file_.clear();
    std::streampos start_pos = file_.tellg();

    size_t chars{};
    char byte;
    while (file_.get(byte))
        chars += (byte & 0xC0) != 0x80;

    file_.clear();
    file_.seekg(start_pos);
    return chars;
}

size_t FileAnalyzer::count_words() {
    file_.clear();
    std::streampos start_pos = file_.tellg();

    size_t words{};
    std::string word;
    while (file_ >> word)
        ++words;

    file_.clear();
    file_.seekg(start_pos);
    return words;
}

size_t FileAnalyzer::count_lines() {
    file_.clear();
    std::streampos start_pos = file_.tellg();

    size_t lines{};
    std::string line;
    while (std::getline(file_, line))
        ++lines;

    file_.clear();
    file_.seekg(start_pos);
    return lines;
}

size_t FileAnalyzer::max_line_length() {
    file_.clear();
    file_.seekg(0);
    std::streampos start_pos = file_.tellg();

    size_t max_length{};
    std::string line;
    while (std::getline(file_, line))
        max_length = std::max(max_length, utf8_length(line.data()));

    file_.clear();
    file_.seekg(start_pos);
    return max_length;
}

using FileAnalyzerMethod = size_t (FileAnalyzer::*)();

enum class Mode { LINES, WORDS, CHARS, BYTES, LINE_LENGTH };

constexpr std::array<
    std::tuple<std::string_view, char, Mode, FileAnalyzerMethod>, 5>
    options_list = {{{"lines", 'l', Mode::LINES, &FileAnalyzer::count_lines},
                     {"words", 'w', Mode::WORDS, &FileAnalyzer::count_words},
                     {"chars", 'm', Mode::CHARS, &FileAnalyzer::count_chars},
                     {"bytes", 'c', Mode::BYTES, &FileAnalyzer::count_bytes},
                     {"max-line-length", 'L', Mode::LINE_LENGTH,
                      &FileAnalyzer::max_line_length}}};
constexpr uint8_t default_options = 0b00001011;

constexpr auto get_method(const auto &entry) { return std::get<3>(entry); };

using FileStats =
    std::pair<std::array<size_t, options_list.size()>, std::string>;

template <size_t N>
void set_word_command(std::string_view option, std::bitset<N> &selected_options) {
    std::string_view option_name = option.substr(2, option.size() - 2);
    bool found = 0;
    for (auto &&[name, character, mode, method] : options_list) {
        if (option_name == name) {
            selected_options[static_cast<size_t>(mode)] = 1;
            found = 1;
        }
    }
    if (!found)
        throw std::runtime_error{"'" + std::string(option.data()) +
                                 "': invalid command"};
}

template <size_t N>
void set_char_command(std::string_view option, std::bitset<N> &selected_options) {
    std::string_view option_characters = option.substr(1, option.size() - 1);
    for (char option_character : option_characters) {
        bool found = 0;
        for (auto &&[name, character, mode, method] : options_list) {
            if (option_character == character) {
                selected_options[static_cast<size_t>(mode)] = 1;
                found = 1;
            }
        }
        if (!found)
            throw std::runtime_error{"'-" + std::string(1, option_character) +
                                     "': invalid command"};
    }
}

void set_argument(std::string_view option, std::vector<FileAnalyzer> &files) {
    files.emplace_back(option.data());
}

template <size_t N>
void print_result(std::ostream &os, const std::vector<FileStats> &result,
                  std::bitset<N> &selected_options) {

    FileStats total;
    size_t max_setw = 1;
    for (auto &&file_stats : result) {
        for (size_t i = 0; i < options_list.size(); ++i) {
            if (selected_options[i]) {
                if (i == static_cast<size_t>(Mode::LINE_LENGTH))
                    total.first[i] = std::max(total.first[i], file_stats.first[i]);
                else
                    total.first[i] += file_stats.first[i];
                size_t digit_count =
                    file_stats.first[i]
                        ? std::ceil(std::log10(file_stats.first[i]))
                        : 1;
                max_setw = std::max(max_setw, digit_count);
            }
        }
    }

    for (size_t i = 0; i < options_list.size(); ++i) {
        if (selected_options[i]) {
            size_t digit_count =
                total.first[i] ? std::ceil(std::log10(total.first[i])) : 1;
            max_setw = std::max(max_setw, digit_count);
        }
    }

    os << std::right;
    for (auto &&file_stats : result) {
        for (size_t i = 0; i < options_list.size(); ++i) {
            if (selected_options[i])
                os << std::setw(max_setw) << file_stats.first[i] << " ";
        }
        os << std::setw(max_setw) << file_stats.second << '\n';
    }

    if (result.size() > 1) {
        for (size_t i = 0; i < options_list.size(); ++i)
            if (selected_options[i])
                os << std::setw(max_setw) << total.first[i] << " ";

        os << std::setw(max_setw) << "total" << '\n';
    }
}

void print_help() {
    std::cout << R"(Usage: wcpp [OPTION]... [FILE]...

Print newline, word, and byte counts for each FILE.

Options:
  -c, --bytes             print the number of bytes
  -m, --chars             print the number of characters (UTF-8 aware)
  -l, --lines             print the number of lines
  -w, --words             print the number of words
  -L, --max-line-length   print the length of the longest line
      --help              display this help and exit
      --version           output version information and exit
)";
}

void print_version() {
    std::cout << "wcpp version 1.1\n";
}

int main(int argc, char **argv) try {
    std::vector<std::string_view> options(argv + 1, argv + argc);
    std::vector<std::string_view> file_names;
    std::bitset<options_list.size()> selected_options;

    std::vector<FileAnalyzer> files;
    std::vector<FileStats> result;

    for (auto &&option : options) {
        if (option == "--help") {
            print_help();
            return EXIT_SUCCESS;
        } else if (option == "--version") {
            print_version();
            return EXIT_SUCCESS;
        } else if (option.starts_with("--"))
            set_word_command(option, selected_options);
        else if (option.starts_with("-"))
            set_char_command(option, selected_options);
        else
            file_names.emplace_back(option);
    }
    if (selected_options.none())
        selected_options = default_options;

    for (auto &&file_name : file_names)
        set_argument(file_name, files);

    for (auto it = files.begin(), ite = files.end(); it != ite; ++it) {
        FileStats file_stats;
        FileAnalyzer *ptr = &(*it);
        for (size_t i = 0; i < options_list.size(); ++i)
            if (selected_options[i])
                file_stats.first[i] = (ptr->*get_method(options_list[i]))();
        file_stats.second = it->get_file_name();
        result.push_back(std::move(file_stats));
    }
    print_result(std::cout, result, selected_options);
    return EXIT_SUCCESS;
} catch (std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
