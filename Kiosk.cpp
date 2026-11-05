#include "Kiosk.h"

#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Utils.h"

namespace {

Date readDate(const std::string& prompt) {
    while (true) {
        try {
            return Date::parse(util::readLine(prompt));
        } catch (const std::invalid_argument& e) {
            std::cout << e.what() << '\n';
        }
    }
}

std::string fitColumn(const std::string& s, std::size_t maxLen) {
    if (s.size() <= maxLen) return s;
    return maxLen > 3 ? s.substr(0, maxLen - 3) + "..." : s.substr(0, maxLen);
}

void printUser(const User& u) {
    std::cout << "\n--- " << u.typeName() << " card ---\n"
              << "UID:           " << u.uid() << '\n'
              << "Name:          " << u.name() << '\n'
              << "Blood group:   " << u.bloodGroup() << '\n'
              << "Contact:       " << u.contact() << '\n'
              << "Date of birth: " << u.dob() << '\n'
              << "Balance:       " << util::formatMoney(u.balance()) << '\n'
              << "Valid until:   " << u.validUntil() << '\n';
}

}  // namespace

Kiosk::Kiosk(UserRepository& repo, AdminAuth& auth) : repo_(repo), auth_(auth) {}

void Kiosk::run() {
    if (!auth_.exists()) {
        std::cout << "First run: please create an admin account.\n";
        registerAdmin();
    }

    while (true) {
        std::cout << "\n=====================================\n"
                  << "   Welcome to Digital Bus Card System\n"
                  << "=====================================\n"
                  << "  1. Tap card (passenger)\n"
                  << "  2. Admin login\n"
                  << "  0. Exit\n";

        const int choice = util::readInt("Choice: ", 0, 2);
        if (choice == 0) break;
        if (choice == 1) tapCard();
        else if (adminLogin()) adminMenu();
    }
    std::cout << "Goodbye!\n";
}

void Kiosk::tapCard() {
    const std::string uid = util::toUpper(util::readLine("Enter card UID (blank to cancel): "));
    if (uid.empty()) return;

    User* user = repo_.find(uid);
    if (!user) {
        std::cout << "Card not recognised.\n";
        return;
    }

    const std::time_t now = std::time(nullptr);
    const std::tm localNow = *std::localtime(&now);
    const Date today{localNow.tm_year + 1900, localNow.tm_mon + 1, localNow.tm_mday};

    if (user->isExpired(today)) {
        std::cout << "Card expired on " << user->validUntil() << ". Please renew at the admin desk.\n";
        return;
    }
    if (user->isNearExpiry(today)) {
        std::cout << "Notice: your card expires on " << user->validUntil() << ".\n";
    }

    const long long fare = user->baseFare();
    if (user->balance() < fare) {
        std::cout << "Insufficient balance. Fare is " << util::formatMoney(fare)
                  << ", you have " << util::formatMoney(user->balance()) << ".\n";
        return;
    }

    const std::string from = util::sanitize(util::readLine("From station: "));
    const std::string to = util::sanitize(util::readLine("To station: "));

    const Transaction tx{today, localNow.tm_hour, localNow.tm_min,
                         from.empty() ? "-" : from,
                         to.empty() ? "-" : to,
                         fare};

    user->chargeTrip(tx);
    repo_.save();

    std::cout << "\n*** Trip recorded ***\n"
              << "Fare:        " << util::formatMoney(fare) << '\n'
              << "New balance: " << util::formatMoney(user->balance()) << '\n';
    if (user->isLowBalance()) std::cout << "Low balance. Please recharge soon.\n";
}

void Kiosk::registerAdmin() {
    std::cout << "\n=== Create admin account ===\n";
    std::string username;
    do {
        username = util::sanitize(util::readLine("Admin username: "));
    } while (username.empty());

    std::string password;
    while (true) {
        password = util::readPassword("Password (min 4 characters): ");
        if (password.size() < 4) {
            std::cout << "Password too short.\n";
            continue;
        }
        if (password != util::readPassword("Confirm password: ")) {
            std::cout << "Passwords do not match.\n";
            continue;
        }
        break;
    }

    auth_.registerAdmin(username, password);
    std::cout << "Admin account created.\n";
}

bool Kiosk::adminLogin() {
    for (int attempt = 1; attempt <= 3; ++attempt) {
        std::cout << "\n=== Admin login (" << attempt << "/3) ===\n";
        const std::string username = util::sanitize(util::readLine("Username: "));
        const std::string password = util::readPassword("Password: ");
        if (auth_.verify(username, password)) return true;
        std::cout << "Incorrect username or password.\n";
    }
    std::cout << "Too many failed attempts.\n";
    util::pause(2000);
    return false;
}

void Kiosk::adminMenu() {
    // Menu table: each entry is a label and a lambda that runs it.
    const std::vector<std::pair<std::string, std::function<void()>>> options = {
        {"Add user",          [this] { addUser(); }},
        {"Delete user",       [this] { deleteUser(); }},
        {"Search user",       [this] { searchUser(); }},
        {"List all users",    [this] { listUsers(); }},
        {"Recharge card",     [this] { recharge(); }},
        {"View trip history", [this] { showHistory(); }},
    };

    while (true) {
        std::cout << "\n=== Admin panel ===\n";
        for (std::size_t i = 0; i < options.size(); ++i) {
            std::cout << "  " << i + 1 << ". " << options[i].first << '\n';
        }
        std::cout << "  0. Logout\n";

        const int choice = util::readInt("Choice: ", 0, static_cast<int>(options.size()));
        if (choice == 0) return;
        options[choice - 1].second();
    }
}

void Kiosk::addUser() {
    std::cout << "\n=== New card ===\n"
              << "  0. General  (valid 5 years)\n"
              << "  1. Student  (valid 2 years, half fare)\n"
              << "  2. Tourist  (valid 1 month, 30% surcharge)\n"
              << "  3. Child    (valid 5 years, half fare)\n";

    const auto type = static_cast<UserType>(util::readInt("Card type: ", 0, 3));

    std::string name;
    do {
        name = util::sanitize(util::readLine("Full name: "));
    } while (name.empty());

    const std::string blood = util::sanitize(util::readLine("Blood group (e.g. B+): "));
    const std::string contact = util::sanitize(util::readLine("Emergency contact: "));

    const Date today = Date::today();
    Date dob;
    while (true) {
        dob = readDate("Date of birth (dd-mm-yyyy): ");
        if (dob <= today) break;
        std::cout << "Date of birth cannot be in the future.\n";
    }

    const long long balance = util::readTaka("Opening balance (Tk): ", true);

    const std::string uid = repo_.nextUid(type);
    auto user = User::create(type, uid, name, blood, contact, balance, dob);
    const Date validUntil = user->issueValidity(today);
    user->setValidUntil(validUntil);

    if (!repo_.add(std::move(user))) {
        std::cout << "Could not add user.\n";
        return;
    }
    repo_.save();

    std::cout << "Card issued. UID: " << uid
              << ", valid until " << validUntil << '\n';
}

void Kiosk::deleteUser() {
    const std::string uid = util::toUpper(util::readLine("UID to delete: "));
    if (util::readLine("Type YES to confirm: ") != "YES") {
        std::cout << "Cancelled.\n";
        return;
    }
    if (repo_.remove(uid)) {
        repo_.save();
        std::cout << "User deleted.\n";
    } else {
        std::cout << "No card with UID " << uid << ".\n";
    }
}

void Kiosk::searchUser() {
    if (User* u = askForUser()) printUser(*u);
}

void Kiosk::listUsers() const {
    const auto& users = repo_.all();
    if (users.empty()) {
        std::cout << "No users yet.\n";
        return;
    }
    std::cout << std::left << std::setw(8) << "UID" << std::setw(22) << "Name"
              << std::setw(10) << "Type" << std::setw(14) << "Balance" << "Valid until\n";

    for (const auto& entry : users) {
        const User& user = *entry.second;
        std::cout << std::left << std::setw(8) << entry.first
                  << std::setw(22) << fitColumn(user.name(), 20)
                  << std::setw(10) << user.typeName()
                  << std::setw(14) << util::formatMoney(user.balance())
                  << user.validUntil() << '\n';
    }
}

void Kiosk::recharge() {
    User* u = askForUser();
    if (!u) return;

    const long long amount = util::readTaka("Amount to add (Tk): ", false);
    u->recharge(amount);
    repo_.save();
    std::cout << "New balance: " << util::formatMoney(u->balance()) << '\n';
}

void Kiosk::showHistory() {
    User* u = askForUser();
    if (!u) return;

    if (u->history().empty()) {
        std::cout << "No trips recorded.\n";
        return;
    }

    std::cout << "\nRecent trips for " << u->name() << ":\n";
    for (const Transaction& t : u->history()) {
        std::cout << t.date << ' ' << std::setfill('0') << std::setw(2) << t.hour << ':'
                  << std::setw(2) << t.minute << std::setfill(' ') << "  "
                  << std::left << std::setw(15) << fitColumn(t.from, 14) << " -> "
                  << std::setw(15) << fitColumn(t.to, 14) << std::right
                  << util::formatMoney(t.fare) << '\n';
    }
}

User* Kiosk::askForUser() {
    const std::string uid = util::toUpper(util::readLine("Enter UID: "));
    User* u = repo_.find(uid);
    if (!u) std::cout << "No card with UID " << uid << ".\n";
    return u;
}
