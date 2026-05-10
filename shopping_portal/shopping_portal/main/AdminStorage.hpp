// main/AdminStorage.hpp
#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "../templates/admin/main.h"

// =============================================================================
// AdminStorage
// -----------------------------------------------------------------------------
// File persistence for Admin accounts. Mirrors UserStorage but writes to a
// separate file (admins.txt) so the user and admin namespaces stay isolated.
//
// File format (pipe-separated, one admin per line):
//   username|password|type|balance|lifetimeSpent|paymentMethod
// =============================================================================
class AdminStorage {
public:
    static const std::string ADMIN_FILE;

    static bool saveAdmin(const Admin& a) {
        std::map<std::string, Admin> admins = loadAllAdmins();
        admins[a.getUsername()] = a;

        std::ofstream file(ADMIN_FILE);
        if (!file.is_open()) return false;
        for (const auto& pair : admins) {
            const Admin& x = pair.second;
            file << x.getUsername()      << "|"
                 << x.getPassword()      << "|"
                 << x.getType()          << "|"
                 << x.getBalance()       << "|"
                 << x.getLifetimeSpent() << "|"
                 << x.getPaymentMethod() << "\n";
        }
        return true;
    }

    static Admin loadAdmin(const std::string& username) {
        std::map<std::string, Admin> admins = loadAllAdmins();
        auto it = admins.find(username);
        if (it != admins.end()) return it->second;
        return Admin();
    }

    static std::map<std::string, Admin> loadAllAdmins() {
        std::map<std::string, Admin> out;
        std::ifstream file(ADMIN_FILE);
        if (!file.is_open()) return out;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string username, passStr, type, balStr, lifeStr, pay;
            if (std::getline(iss, username, '|') &&
                std::getline(iss, passStr,  '|') &&
                std::getline(iss, type,     '|') &&
                std::getline(iss, balStr,   '|') &&
                std::getline(iss, lifeStr,  '|') &&
                std::getline(iss, pay,      '|')) {
                try {
                    int    pwd  = std::stoi(passStr);
                    double bal  = std::stod(balStr);
                    double life = std::stod(lifeStr);
                    Admin a(username, pwd, type, bal, pay);
                    a.setLifetimeSpent(life);
                    out[username] = a;
                } catch (...) {
                    continue;
                }
            }
        }
        return out;
    }

    static bool adminExists(const std::string& username) {
        std::map<std::string, Admin> admins = loadAllAdmins();
        return admins.find(username) != admins.end();
    }
};

const std::string AdminStorage::ADMIN_FILE = "admins.txt";
