#include <exception>
#include <iostream>

#include "AdminAuth.h"
#include "Kiosk.h"
#include "UserRepository.h"

int main() {
    try {
        UserRepository repo("users.txt");
        AdminAuth auth("admin.txt");
        repo.load();

        Kiosk kiosk(repo, auth);
        kiosk.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
