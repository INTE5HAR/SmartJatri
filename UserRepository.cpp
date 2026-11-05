#include "UserRepository.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "Utils.h"

namespace {

bool isValidUidFormat(const std::string& uid) {
    if (uid.size() < 2) return false;
    for (std::size_t i = 1; i < uid.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(uid[i]))) return false;
    }
    return true;
}

}  // namespace

UserRepository::UserRepository(std::string file) : file_(std::move(file)) {}

// File format:
//   U|uid|name|blood|contact|balance_paisa|dob|validUntil
//   T|date|hour|minute|from|to|fare_paisa      (trip lines follow their user line)
void UserRepository::load() {
    std::ifstream in(file_);
    if (!in) return;   // first run, nothing saved yet

    std::string line;
    User* current = nullptr;
    int lineNo = 0;

    while (std::getline(in, line)) {
        ++lineNo;
        if (util::trim(line).empty()) continue;
        const auto f = util::split(line, '|');
        if (f.empty()) continue;

        try {
            if (f[0] == "U") {
                current = nullptr;  // reset so corrupt U lines don't attach trips to previous user
                if (f.size() != 8) throw std::runtime_error("invalid user field count");

                const std::string uid = util::toUpper(util::trim(f[1]));
                if (!isValidUidFormat(uid)) throw std::runtime_error("invalid UID format");

                auto user = User::create(typeFromCode(uid[0]), uid, f[2], f[3], f[4],
                                         std::stoll(f[5]), Date::parse(f[6]));
                user->setValidUntil(Date::parse(f[7]));
                current = user.get();
                users_[uid] = std::move(user);
            } else if (f[0] == "T" && f.size() == 7 && current) {
                Transaction tx;
                tx.date = Date::parse(f[1]);
                tx.hour = std::stoi(f[2]);
                tx.minute = std::stoi(f[3]);
                if (tx.hour < 0 || tx.hour > 23 || tx.minute < 0 || tx.minute > 59) {
                    throw std::runtime_error("invalid transaction time");
                }
                tx.from = f[4];
                tx.to = f[5];
                tx.fare = std::stoll(f[6]);
                current->addTransaction(tx);
            } else {
                throw std::runtime_error("unrecognised or orphaned record");
            }
        } catch (const std::exception& e) {
            std::cerr << "Skipping corrupt line " << lineNo << " in " << file_
                      << " (" << e.what() << ")\n";
        }
    }
}

void UserRepository::save() const {
    std::ofstream out(file_, std::ios::trunc);
    if (!out) throw std::runtime_error("Cannot write " + file_);

    for (const auto& entry : users_) {
        const User& u = *entry.second;
        out << "U|" << u.uid() << '|' << u.name() << '|' << u.bloodGroup() << '|'
            << u.contact() << '|' << u.balance() << '|' << u.dob() << '|' << u.validUntil() << '\n';
        for (const Transaction& t : u.history()) {
            out << "T|" << t.date << '|' << t.hour << '|' << t.minute << '|'
                << t.from << '|' << t.to << '|' << t.fare << '\n';
        }
    }
}

User* UserRepository::find(const std::string& uid) const {
    const auto it = users_.find(uid);
    return it == users_.end() ? nullptr : it->second.get();
}

bool UserRepository::add(std::unique_ptr<User> user) {
    const std::string uid = user->uid();
    if (find(uid)) return false;
    users_[uid] = std::move(user);
    return true;
}

bool UserRepository::remove(const std::string& uid) {
    return users_.erase(uid) > 0;
}

std::string UserRepository::nextUid(UserType t) const {
    const char prefix = typeCode(t);
    int highest = 0;
    for (const auto& entry : users_) {
        const std::string& uid = entry.first;
        if (!uid.empty() && uid[0] == prefix && isValidUidFormat(uid)) {
            highest = std::max(highest, std::stoi(uid.substr(1)));
        }
    }
    std::ostringstream os;
    os << prefix << std::setw(4) << std::setfill('0') << highest + 1;
    return os.str();
}
