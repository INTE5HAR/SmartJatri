#include "Utils.h"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <chrono>
#include <thread>
#endif

namespace util {

std::string trim(const std::string& s) {
    const std::string::size_type first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const std::string::size_type last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

std::string toUpper(std::string s) {
    for (std::size_t i = 0; i < s.size(); ++i) {
        s[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[i])));
    }
    return s;
}

std::string sanitize(std::string s) {
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '|') s[i] = '/';
    }
    return trim(s);
}

std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    if (s.empty()) return out;

    std::string part;
    std::istringstream stream(s);
    while (std::getline(stream, part, sep)) {
        out.push_back(part);
    }
    if (s.back() == sep) {
        out.push_back("");
    }
    return out;
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) throw std::runtime_error("Input closed");
    return trim(line);
}

std::string readPassword(const std::string& prompt) {
    std::cout << prompt;
#ifdef _WIN32
    std::string password;
    while (true) {
        int ch = _getch();
        if (ch == 3 || ch == EOF) {       // Ctrl+C or EOF
            std::cout << '\n';
            throw std::runtime_error("Input interrupted");
        }
        if (ch == 0 || ch == 224) {       // arrow and function keys send two codes
            _getch();
            continue;
        }
        if (ch == '\r' || ch == '\n') break;
        if (ch == '\b') {
            if (!password.empty()) {
                password.erase(password.size() - 1);
                std::cout << "\b \b";
            }
        } else if (ch >= 32 && ch < 127) {
            password.push_back(static_cast<char>(ch));
            std::cout << '*';
        }
    }
    std::cout << '\n';
    return password;
#else
    return readLine("");   // non-Windows fallback: input is visible
#endif
}

int readInt(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        const std::string line = readLine(prompt);
        try {
            std::size_t used = 0;
            const int value = std::stoi(line, &used);
            if (used == line.size() && value >= minValue && value <= maxValue) return value;
        } catch (const std::exception&) {
            // not a number: fall through and ask again
        }
        std::cout << "Please enter a number from " << minValue << " to " << maxValue << ".\n";
    }
}

bool parseTaka(const std::string& text, long long& paisa, bool allowZero) {
    if (text.empty()) return false;
    try {
        std::size_t used = 0;
        const double value = std::stod(text, &used);
        if (used != text.size() || !std::isfinite(value)) return false;
        if (value < 0.0 || value > 10000000.0) return false; // max 1 Crore Tk
        const long long result = std::llround(value * 100.0);
        if (!allowZero && result <= 0) return false;
        paisa = result;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

long long readTaka(const std::string& prompt, bool allowZero) {
    while (true) {
        long long amount = 0;
        if (parseTaka(readLine(prompt), amount, allowZero)) return amount;
        std::cout << "Enter a valid amount" << (allowZero ? "" : " greater than 0")
                  << ", for example 250 or 99.50.\n";
    }
}

std::string formatMoney(long long paisa) {
    const long long absolute = paisa < 0 ? -paisa : paisa;
    std::ostringstream os;
    os << (paisa < 0 ? "-" : "") << "Tk " << absolute / 100 << '.'
       << std::setw(2) << std::setfill('0') << absolute % 100;
    return os.str();
}

void pause(int milliseconds) {
#ifdef _WIN32
    Sleep(static_cast<DWORD>(milliseconds));
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

}  // namespace util
