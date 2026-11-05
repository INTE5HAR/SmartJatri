#include "User.h"

#include <stdexcept>
#include <utility>

char typeCode(UserType t) {
    switch (t) {
        case UserType::General: return 'G';
        case UserType::Student: return 'S';
        case UserType::Tourist: return 'T';
        case UserType::Child:   return 'C';
    }
    throw std::invalid_argument("unknown user type");
}

UserType typeFromCode(char code) {
    switch (code) {
        case 'G': return UserType::General;
        case 'S': return UserType::Student;
        case 'T': return UserType::Tourist;
        case 'C': return UserType::Child;
    }
    throw std::invalid_argument("unknown card code");
}

namespace {

class General : public User {
public:
    using User::User;   // inherit the constructor
    UserType type() const override { return UserType::General; }
    std::string typeName() const override { return "General"; }
    long long baseFare() const override { return kBaseFare; }
    Date issueValidity(const Date& issue) const override { return issue.addYears(5); }
};

class Student : public User {
public:
    using User::User;
    UserType type() const override { return UserType::Student; }
    std::string typeName() const override { return "Student"; }
    long long baseFare() const override { return kBaseFare / 2; }
    Date issueValidity(const Date& issue) const override { return issue.addYears(2); }
};

class Tourist : public User {
public:
    using User::User;
    UserType type() const override { return UserType::Tourist; }
    std::string typeName() const override { return "Tourist"; }
    long long baseFare() const override { return kBaseFare * 13 / 10; }   // 30% surcharge
    Date issueValidity(const Date& issue) const override { return issue.addMonths(1); }
};

class Child : public User {
public:
    using User::User;
    UserType type() const override { return UserType::Child; }
    std::string typeName() const override { return "Child"; }
    long long baseFare() const override { return kBaseFare / 2; }
    Date issueValidity(const Date& issue) const override { return issue.addYears(5); }
};

}  // namespace

User::User(std::string uid, std::string name, std::string blood,
           std::string contact, long long balance, Date dob)
    : uid_(std::move(uid)), name_(std::move(name)), blood_(std::move(blood)),
      contact_(std::move(contact)), balance_(balance), dob_(dob) {}

std::unique_ptr<User> User::create(UserType t, std::string uid, std::string name,
                                   std::string blood, std::string contact,
                                   long long balance, Date dob) {
    switch (t) {
        case UserType::General:
            return std::make_unique<General>(std::move(uid), std::move(name), std::move(blood),
                                             std::move(contact), balance, dob);
        case UserType::Student:
            return std::make_unique<Student>(std::move(uid), std::move(name), std::move(blood),
                                             std::move(contact), balance, dob);
        case UserType::Tourist:
            return std::make_unique<Tourist>(std::move(uid), std::move(name), std::move(blood),
                                             std::move(contact), balance, dob);
        case UserType::Child:
            return std::make_unique<Child>(std::move(uid), std::move(name), std::move(blood),
                                           std::move(contact), balance, dob);
    }
    throw std::invalid_argument("unknown user type");
}

bool User::isExpired(const Date& today) const {
    return validUntil_ < today;
}

bool User::isNearExpiry(const Date& today, int warningDays) const {
    const long long daysLeft = today.daysUntil(validUntil_);
    return daysLeft >= 0 && daysLeft <= warningDays;
}

bool User::chargeTrip(const Transaction& tx) {
    if (balance_ < tx.fare) return false;
    balance_ -= tx.fare;
    addTransaction(tx);
    return true;
}

void User::addTransaction(const Transaction& tx) {
    if (history_.size() >= kMaxHistory) history_.pop_front();   // keep only the last 10
    history_.push_back(tx);
}
