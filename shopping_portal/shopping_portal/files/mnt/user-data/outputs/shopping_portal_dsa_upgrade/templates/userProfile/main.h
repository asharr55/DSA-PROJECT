#pragma once

#include <iostream>
#include <string>
#include "../Cart/main.h"
#include "../account/main.h"

using namespace std;

// =============================================================================
// User
// -----------------------------------------------------------------------------
// Profile + Cart + Account.
//
// CHANGE LOG (DSA upgrade):
//   - getBalance() / getLifetimeSpent() / getPaymentMethod() are now const,
//     matching the const-correct Account getters.
//   - addToCart() forwards to CART::addToCart() which now records undo/redo.
// =============================================================================
class User {
private:
    string  username;
    int     password;      // simple demo password (int)
    string  type;          // "customer" or "admin"
    CART    shoppingcart;
    Account useraccount;

public:
    // Default constructor (needed for global currentUser)
    User()
        : username(""),
          password(0),
          type("customer"),
          shoppingcart(),
          useraccount(0.0, 0.0, "none") {}

    // Parameterized constructor
    User(const string& username,
         int password,
         const string& type,
         double startingBalance,
         const string& paymentMethod)
        : username(username),
          password(password),
          type(type),
          shoppingcart(),
          useraccount(startingBalance, 0.0, paymentMethod) {}

    // Getters (now all const)
    const string& getUsername()       const { return username; }
    const string& getType()           const { return type; }
    double        getBalance()        const { return useraccount.gettotalbalence(); }
    double        getLifetimeSpent()  const { return useraccount.getlifetimespent(); }
    int           getPassword()       const { return password; }
    string        getPaymentMethod()  const { return useraccount.getpaymentmethods(); }

    CART&         getShoppingCart()         { return shoppingcart; }
    const CART&   getShoppingCart()  const  { return shoppingcart; }

    // Setters / updates
    void setUsername(const string& newname) { username = newname; }
    void setType(const string& newtype)     { type     = newtype; }
    void changePassword(int newpass)        { password = newpass; }
    void setLifetimeSpent(double amount)    { useraccount.setlifetimespent(amount); }
    void setBalance(double amount)          { useraccount.settotalbalence(amount); }

    bool checkPassword(int entered) const {
        return entered == password;
    }

    // Cart operations (forward to CART, which records undo/redo)
    void addToCart(Products& newProduct) {
        shoppingcart.addToCart(newProduct);
    }

    void removeFromCart(Products& toRemProd) {
        shoppingcart.removeFromCart(toRemProd);
    }

    // Console-mode checkout (kept for completeness; UI uses its own flow)
    bool checkout() {
        double total = shoppingcart.getTotalPrice();
        double funds = useraccount.gettotalbalence();

        if (total == 0.0) {
            cout << "Cart is empty. Nothing to buy." << endl;
            return false;
        }
        if (funds < total) {
            cout << "Transaction unsuccessful, insufficient funds." << endl;
            return false;
        }
        useraccount.settotalbalence(funds - total);
        useraccount.setlifetimespent(useraccount.getlifetimespent() + total);
        cout << "Transaction successful, your delivery is on its way!" << endl;
        return true;
    }
};
