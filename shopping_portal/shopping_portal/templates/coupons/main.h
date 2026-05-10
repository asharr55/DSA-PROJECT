#ifndef COUPONS_MAIN_H
#define COUPONS_MAIN_H

#include <iostream>
using namespace std;

class Coupons {
private:
    string name;
    string description;
    int discount; // in %

public:
    Coupons(string name, string desc, int disc)
        : name(name), description(desc), discount(disc) {}

    Coupons(const Coupons& other) {
        this->description = other.description;
        this->name = other.name;
        this->discount = other.discount;
    }

    Coupons& operator=(const Coupons& other) {
        if (this != &other) {
            this->description = other.description;
            this->name = other.name;
            this->discount = other.discount;
        }
        return *this;
    }

    string getname() const { return this->name; }
    string getdesc() const { return this->description; }
    int getdiscount() const { return this->discount; }

    void setname(string newname) { this->name = newname; }
    void setdesc(string newdesc) { this->description = newdesc; }
    void setdiscount(int newdisc) { this->discount = newdisc; }

    ~Coupons() {}
};

#endif
