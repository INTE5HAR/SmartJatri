#include "AdminAuth.h"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "Utils.h"

namespace {

std::string makeSalt() {
    std::random_device rd;
    std::ostringstream os;
    for (int i = 0; i < 4; ++i) {
        os << std::hex << std::setw(8) << std::setfill('0') << rd();
    }
    return os.str();
}

}  // namespace

AdminAuth::AdminAuth(std::string file) : file_(std::move(file)) {}

bool AdminAuth::exists() const {
    std::ifstream in(file_);
    return in.good();
}

// Demo-level hashing (salted FNV-1a). Fine for a course project, but for real
// systems use a proper password hash such as bcrypt, scrypt, or Argon2.
std::string AdminAuth::hash(const std::string& salt, const std::string& password) {
    std::uint64_t h = 1469598103934665603ULL;   // FNV offset basis
    for (unsigned char c : salt + ":" + password) {
        h ^= c;
        h *= 1099511628211ULL;                  // FNV prime
    }
    std::ostringstream os;
    os << std::hex << std::setw(16) << std::setfill('0') << h;
    return os.str();
}

void AdminAuth::registerAdmin(const std::string& username, const std::string& password) {
    const std::string cleanUser = util::sanitize(username);
    if (cleanUser.empty()) throw std::invalid_argument("Username cannot be empty");

    const std::string salt = makeSalt();
    std::ofstream out(file_, std::ios::trunc);
    if (!out) throw std::runtime_error("Cannot write " + file_);
    out << cleanUser << '|' << salt << '|' << hash(salt, password) << '\n';
}

bool AdminAuth::verify(const std::string& username, const std::string& password) const {
    std::ifstream in(file_);
    std::string line;
    if (!in || !std::getline(in, line)) return false;

    const auto fields = util::split(line, '|');
    return fields.size() == 3 && fields[0] == username && fields[2] == hash(fields[1], password);
}
