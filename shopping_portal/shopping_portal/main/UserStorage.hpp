// main/UserStorage.hpp
#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "../templates/userProfile/main.h"

// =============================================================================
// UserStorage
// -----------------------------------------------------------------------------
// Simple flat-file persistence: one user per line, '|' separated fields.
// The shopping cart is NOT persisted on purpose -- carts are session-scoped,
// so each login starts empty (matches typical e-commerce sites). If you want
// to persist carts later, add a serializer that walks the DSAStack via
// toVector() and writes the products in order.
//
// CHANGE LOG (DSA upgrade):
//   - Removed const_cast hacks (User getters are const now).
// =============================================================================
class UserStorage {
public:
    static const std::string USER_FILE;

    static bool saveUser(const User& user) {
        std::map<std::string, User> users = loadAllUsers();
        users[user.getUsername()] = user;

        std::ofstream file(USER_FILE);
        if (!file.is_open()) return false;

        for (const auto& pair : users) {
            const User& u = pair.second;
            file << u.getUsername()      << "|"
                 << u.getPassword()      << "|"
                 << u.getType()          << "|"
                 << u.getBalance()       << "|"
                 << u.getLifetimeSpent() << "|"
                 << u.getPaymentMethod() << "\n";
        }
        file.close();
        return true;
    }

    static User loadUser(const std::string& username) {
        std::map<std::string, User> users = loadAllUsers();
        auto it = users.find(username);
        if (it != users.end()) return it->second;
        return User();
    }

    static std::map<std::string, User> loadAllUsers() {
        std::map<std::string, User> users;
        std::ifstream file(USER_FILE);
        if (!file.is_open()) return users;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string username, passwordStr, type, balanceStr, lifetimeStr, paymentMethod;

            if (std::getline(iss, username,      '|') &&
                std::getline(iss, passwordStr,   '|') &&
                std::getline(iss, type,          '|') &&
                std::getline(iss, balanceStr,    '|') &&
                std::getline(iss, lifetimeStr,   '|') &&
                std::getline(iss, paymentMethod, '|')) {

                try {
                    int    password = std::stoi(passwordStr);
                    double balance  = std::stod(balanceStr);
                    double lifetime = std::stod(lifetimeStr);

                    User u(username, password, type, balance, paymentMethod);
                    u.setLifetimeSpent(lifetime);
                    users[username] = u;
                } catch (...) {
                    continue;   // skip malformed rows
                }
            }
        }
        file.close();
        return users;
    }

    static bool userExists(const std::string& username) {
        std::map<std::string, User> users = loadAllUsers();
        return users.find(username) != users.end();
    }

    static bool deleteUser(const std::string& username) {
        std::map<std::string, User> users = loadAllUsers();
        auto it = users.find(username);
        if (it == users.end()) return false;
        users.erase(it);

        std::ofstream file(USER_FILE);
        if (!file.is_open()) return false;
        for (const auto& pair : users) {
            const User& u = pair.second;
            file << u.getUsername()      << "|"
                 << u.getPassword()      << "|"
                 << u.getType()          << "|"
                 << u.getBalance()       << "|"
                 << u.getLifetimeSpent() << "|"
                 << u.getPaymentMethod() << "\n";
        }
        file.close();
        return true;
    }
};

const std::string UserStorage::USER_FILE = "users.txt";
