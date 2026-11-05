#ifndef ADMIN_AUTH_H
#define ADMIN_AUTH_H

#include <string>

class AdminAuth {
public:
    explicit AdminAuth(std::string file);

    bool exists() const;
    void registerAdmin(const std::string& username, const std::string& password);
    bool verify(const std::string& username, const std::string& password) const;

private:
    std::string file_;
    static std::string hash(const std::string& salt, const std::string& password);
};

#endif
