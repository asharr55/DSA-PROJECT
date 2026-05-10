#ifndef ACCOUNT_MAIN_H
#define ACCOUNT_MAIN_H

#include <iostream>
#include <string>
using namespace std;

// =============================================================================
// Account
// -----------------------------------------------------------------------------
// CHANGE LOG (DSA upgrade):
//   - Made all getters `const` so `const User` can read them.
//   - Fixed `operator=` to return `Account&` (was returning by value -- bug).
// =============================================================================
class Account {
private:
    double totalbalance;
    double lifetimespent;
    string paymentmethods;

public:
    Account(double totalbalance, double lifetimespent, string paymentmethods)
        : totalbalance(totalbalance),
          lifetimespent(lifetimespent),
          paymentmethods(paymentmethods) {}

    Account(const Account& other) {
        this->lifetimespent  = other.lifetimespent;
        this->paymentmethods = other.paymentmethods;
        this->totalbalance   = other.totalbalance;
    }

    Account& operator=(const Account& other) {        // <-- now returns ref
        if (this != &other) {
            this->lifetimespent  = other.lifetimespent;
            this->paymentmethods = other.paymentmethods;
            this->totalbalance   = other.totalbalance;
        }
        return *this;
    }

    double gettotalbalence()   const { return this->totalbalance; }     // const
    double getlifetimespent()  const { return this->lifetimespent; }    // const
    string getpaymentmethods() const { return this->paymentmethods; }   // const

    void settotalbalence(double newbal)              { this->totalbalance   = newbal; }
    void setlifetimespent(double newlifetimespent)   { this->lifetimespent  = newlifetimespent; }
    void setpayement(const string& newpaymentmethod) { this->paymentmethods = newpaymentmethod; }

    ~Account() {}
};

#endif
