#ifndef ADMIN_MAIN_H
#define ADMIN_MAIN_H

#include <iostream>
#include <string>
#include "../Cart/main.h"
#include "../account/main.h"

using namespace std;

// =============================================================================
// Admin
// -----------------------------------------------------------------------------
// Separate class for store administrators.
//
// Bug fixes applied to the original draft:
//   - Renamed methods that collided with field names (gettype/getpassword
//     instead of type/password as method names).
//   - Made the constructor public (was inside private:).
//   - Removed the stray int main() that was at the bottom of a header file
//     and would have caused a linker conflict with the real main.cpp.
//   - Added forward decl for setshopping/setaccount (they took CART/Account
//     by value originally; kept that style here).
//
// Why a separate class instead of reusing User?
//   - Admins might gain extra functionality later (manage products, view
//     analytics, ban users, etc.) that doesn't belong on a regular User.
//   - Cleaner separation of concerns -- the type system enforces the role.
// =============================================================================
class Admin {
private:
    string  username;
    int     password;
    string  type;            // always "admin"; kept for symmetry with User
    CART    shoppingcart;    // admins can also browse + buy
    Account adminaccount;

public:
    // Default constructor (needed for AdminStorage::loadAdmin returning empty)
    Admin()
        : username(""),
          password(0),
          type("admin"),
          shoppingcart(),
          adminaccount(0.0, 0.0, "none") {}

    // Parameterized constructor
    Admin(const string& username,
          int            password,
          const string&  type,
          double         startingBalance,
          const string&  paymentMethod)
        : username(username),
          password(password),
          type(type),
          shoppingcart(),
          adminaccount(startingBalance, 0.0, paymentMethod) {}

    // Copy / assignment (auto-generated would work, but being explicit keeps
    // viva-friendly)
    Admin(const Admin& other) = default;
    Admin& operator=(const Admin& other) = default;
    ~Admin() {}

    // ---------- Getters ----------
    // Methods are renamed so they don't collide with the field names of the
    // same name. The original draft had `string type() { ... }` AND a `type`
    // field, which is a compile error.
    const string& getUsername()      const { return username; }
    int           getPassword()      const { return password; }
    const string& getType()          const { return type; }
    double        getBalance()       const { return adminaccount.gettotalbalence(); }
    double        getLifetimeSpent() const { return adminaccount.getlifetimespent(); }
    string        getPaymentMethod() const { return adminaccount.getpaymentmethods(); }

    CART&         getShoppingCart()         { return shoppingcart; }
    const CART&   getShoppingCart()  const  { return shoppingcart; }

    // ---------- Setters ----------
    void setUsername(const string& newname)       { username = newname; }
    void setType(const string& newtype)           { type = newtype; }
    void changePassword(int newpass)              { password = newpass; }
    void setShoppingCart(const CART& newshopping) { shoppingcart = newshopping; }
    void setAccount(const Account& newAcc)        { adminaccount = newAcc; }
    void setBalance(double amount)                { adminaccount.settotalbalence(amount); }
    void setLifetimeSpent(double amount)          { adminaccount.setlifetimespent(amount); }

    // Simple password check
    bool checkPassword(int entered) const {
        return entered == password;
    }
};

#endif
