#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include <map>
#include <memory>
#include <string>

#include "User.h"

class UserRepository {
public:
    explicit UserRepository(std::string file);

    void load();
    void save() const;

    User* find(const std::string& uid) const;
    bool add(std::unique_ptr<User> user);     // false if the UID already exists
    bool remove(const std::string& uid);
    std::string nextUid(UserType t) const;    // e.g. "S0003"

    const std::map<std::string, std::unique_ptr<User>>& all() const { return users_; }

private:
    std::string file_;
    std::map<std::string, std::unique_ptr<User>> users_;
};

#endif
