#pragma once
#include "Database.hpp"
#include <string>

#pragma db object table("Users")
class User : public Database {
public:
    std::string name;
    std::string email;

    User() = default;
    User(std::string n, std::string e) : name(n), email(e) {}
};
