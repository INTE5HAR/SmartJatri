#ifndef USER_H
#define USER_H

#include <cstddef>
#include <deque>
#include <memory>
#include <string>

#include "Date.h"

const long long kBaseFare = 10000;   // 100.00 Tk in paisa
const long long kLowBalance = 5000;  // 50.00 Tk

struct Transaction {
    Date date;
    int hour = 0;
    int minute = 0;
    std::string from;
    std::string to;
    long long fare = 0;   // paisa
};

enum class UserType { General, Student, Tourist, Child };

char typeCode(UserType t);         // 'G', 'S', 'T', 'C'
UserType typeFromCode(char code);  // throws on unknown code

class User {
public:
    static const std::size_t kMaxHistory = 10;

    User(std::string uid, std::string name, std::string blood,
         std::string contact, long long balance, Date dob);
    virtual ~User() {}

    static std::unique_ptr<User> create(UserType t, std::string uid, std::string name,
                                        std::string blood, std::string contact,
                                        long long balance, Date dob);

    virtual UserType type() const = 0;
    virtual std::string typeName() const = 0;
    virtual long long baseFare() const = 0;
    virtual Date issueValidity(const Date& issue) const = 0;

    bool isExpired(const Date& today) const;
    bool isNearExpiry(const Date& today, int warningDays = 7) const;
    bool isLowBalance() const { return balance_ < kLowBalance; }

    void recharge(long long paisa) { balance_ += paisa; }
    bool chargeTrip(const Transaction& tx);
    void addTransaction(const Transaction& tx);
    void setValidUntil(const Date& d) { validUntil_ = d; }

    const std::string& uid() const { return uid_; }
    const std::string& name() const { return name_; }
    const std::string& bloodGroup() const { return blood_; }
    const std::string& contact() const { return contact_; }
    long long balance() const { return balance_; }
    const Date& dob() const { return dob_; }
    const Date& validUntil() const { return validUntil_; }
    const std::deque<Transaction>& history() const { return history_; }

private:
    std::string uid_;
    std::string name_;
    std::string blood_;
    std::string contact_;
    long long balance_;
    Date dob_;
    Date validUntil_;
    std::deque<Transaction> history_;
};

#endif
