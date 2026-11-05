#ifndef DATE_H
#define DATE_H

#include <iosfwd>
#include <string>

struct Date {
    int year = 1970;
    int month = 1;
    int day = 1;

    static Date today();
    static Date parse(const std::string& text);   // "dd-mm-yyyy", throws std::invalid_argument

    long long toDays() const;                     // days since 1970-01-01
    long long daysUntil(const Date& other) const { return other.toDays() - toDays(); }
    Date addMonths(int n) const;
    Date addYears(int n) const;
    bool isValid() const;
    std::string toString() const;                 // "dd-mm-yyyy"
};

bool operator<(const Date& a, const Date& b);
bool operator==(const Date& a, const Date& b);
bool operator<=(const Date& a, const Date& b);
std::ostream& operator<<(std::ostream& os, const Date& d);

#endif
