#ifndef KIOSK_H
#define KIOSK_H

#include "AdminAuth.h"
#include "UserRepository.h"

class Kiosk {
public:
    Kiosk(UserRepository& repo, AdminAuth& auth);
    void run();

private:
    UserRepository& repo_;
    AdminAuth& auth_;

    // Passenger side
    void tapCard();

    // Admin side
    void registerAdmin();
    bool adminLogin();
    void adminMenu();
    void addUser();
    void deleteUser();
    void searchUser();
    void listUsers() const;
    void recharge();
    void showHistory();

    User* askForUser();
};

#endif
