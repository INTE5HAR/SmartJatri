#include "Date.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace {

// Converts a calendar date to a day count (Howard Hinnant's algorithm).
long long daysFromCivil(long long y, unsigned m, unsigned d) {
    y -= m <= 2;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long long>(doe) - 719468;
}

int daysInMonth(int year, int month) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        return leap ? 29 : 28;
    }
    return days[month - 1];
}

}  // namespace

Date Date::today() {
    const std::time_t now = std::time(nullptr);
    const std::tm* t = std::localtime(&now);
    return Date{t->tm_year + 1900, t->tm_mon + 1, t->tm_mday};
}

Date Date::parse(const std::string& text) {
    for (unsigned char c : text) {
        if (std::isspace(c)) {
            throw std::invalid_argument("Invalid date. Use dd-mm-yyyy, for example 15-08-2001.");
        }
    }
    Date d;
    char extra = 0;
    if (std::sscanf(text.c_str(), "%d-%d-%d%c", &d.day, &d.month, &d.year, &extra) != 3 || !d.isValid()) {
        throw std::invalid_argument("Invalid date. Use dd-mm-yyyy, for example 15-08-2001.");
    }
    return d;
}

long long Date::toDays() const {
    return daysFromCivil(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
}

Date Date::addMonths(int n) const {
    const int total = year * 12 + (month - 1) + n;
    int newYear = total / 12;
    int newMonth = total % 12;
    if (newMonth < 0) {
        newMonth += 12;
        --newYear;
    }
    Date result{newYear, newMonth + 1, day};
    result.day = std::min(day, daysInMonth(result.year, result.month));
    return result;
}

Date Date::addYears(int n) const {
    return addMonths(12 * n);
}

bool Date::isValid() const {
    return year >= 1900 && year <= 9999 &&
           month >= 1 && month <= 12 &&
           day >= 1 && day <= daysInMonth(year, month);
}

std::string Date::toString() const {
    std::ostringstream os;
    os << std::setfill('0') << std::setw(2) << day << '-'
       << std::setw(2) << month << '-' << std::setw(4) << year;
    return os.str();
}

bool operator<(const Date& a, const Date& b) {
    return a.toDays() < b.toDays();
}

bool operator==(const Date& a, const Date& b) {
    return a.year == b.year && a.month == b.month && a.day == b.day;
}

bool operator<=(const Date& a, const Date& b) {
    return a < b || a == b;
}

std::ostream& operator<<(std::ostream& os, const Date& d) {
    return os << d.toString();
}
