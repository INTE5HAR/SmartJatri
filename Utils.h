#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace util {
    std::string trim(const std::string& s);
    std::string toUpper(std::string s);
    std::string sanitize(std::string s);                 // removes '|' so it can't break the data file
    std::vector<std::string> split(const std::string& s, char sep);

    std::string readLine(const std::string& prompt);     // throws if input is closed
    std::string readPassword(const std::string& prompt); // masked with * on Windows
    int readInt(const std::string& prompt, int minValue, int maxValue);

    bool parseTaka(const std::string& text, long long& paisa, bool allowZero = true); // "99.50" -> 9950
    long long readTaka(const std::string& prompt, bool allowZero = true);

    std::string formatMoney(long long paisa);            // 9950 -> "Tk 99.50"
    void pause(int milliseconds);
}

#endif
