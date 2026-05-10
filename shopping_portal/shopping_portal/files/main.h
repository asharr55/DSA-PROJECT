#ifndef CART_MAIN_H
#define CART_MAIN_H

#include <iostream>
#include <list>
#include "../products/main.h"
#include "../coupons/main.h"
#include "../DSAStructures.hpp"      // <-- our custom Stack lives here

using namespace std;

// =============================================================================
// CART
// -----------------------------------------------------------------------------
// CHANGE LOG (DSA upgrade):
//   - Replaced std::stack<Products> with our own DSAStack<Products>.
//   - Added undo/redo support backed by two more DSAStacks:
//        undoStack : history of cart actions
//        redoStack : actions that were undone (cleared on a fresh action)
//   - Wired applyCoupons() to actually use the percentage discount.
//   - Renamed `getItemsToBuy()` return type to DSAStack<Products>; existing
//     call sites still compile because DSAStack provides .empty() / .top() /
//     .pop() with the same signatures as std::stack.
// =============================================================================
//
// Why is the cart still a stack?
//   - The proposal mandates LIFO cart operations + undo/redo.
//   - Adding to the cart and undoing the last add map perfectly to push/pop.
//   - Removing an arbitrary item from the *middle* of the cart is the one
//     non-LIFO operation; we handle it with the classic "two-stack swap"
//     pattern (O(n) but uses ONLY stack operations).
//
// =============================================================================

// ---------------------------------------------------------------------------
// CartAction --- a single record in the undo/redo history.
// We store enough info to reverse the action.
// ---------------------------------------------------------------------------
struct CartAction {
    enum Type { ADD, REMOVE };
    Type     type;
    Products product;       // copy of the product involved

    CartAction() : type(ADD) {}
    CartAction(Type t, const Products& p) : type(t), product(p) {}
};

class CART {
private:
    DSAStack<Products>   ItemsToBuy;            // <-- our custom stack
    DSAStack<CartAction> undoStack;             // last N user actions
    DSAStack<CartAction> redoStack;             // actions that were undone
    list<Coupons>        AvailableCouponsList;
    double               totalprice;

    // Internal helpers --- DON'T touch undo/redo (used by undo()/redo() themselves)

    void rawPush(const Products& p) {
        ItemsToBuy.push(p);
        updateBill(p.getprice());
    }

    void rawRemoveById(int id) {
        // Two-stack swap to remove the (last-added) matching id.
        DSAStack<Products> temp;
        bool removed = false;
        while (!ItemsToBuy.isEmpty()) {
            Products topItem = ItemsToBuy.top();
            ItemsToBuy.pop();
            if (!removed && topItem.getID() == id) {
                updateBill(-topItem.getprice());
                removed = true;
            } else {
                temp.push(topItem);
            }
        }
        // Restore the surviving items (preserves original order)
        while (!temp.isEmpty()) {
            ItemsToBuy.push(temp.top());
            temp.pop();
        }
    }

public:
    // -------- Constructors / Rule of three ---------------------------------
    CART() : totalprice(0.0) {}

    CART(const CART& other)
        : ItemsToBuy(other.ItemsToBuy),
          undoStack(other.undoStack),
          redoStack(other.redoStack),
          AvailableCouponsList(other.AvailableCouponsList),
          totalprice(other.totalprice) {}

    CART& operator=(const CART& other) {
        if (this != &other) {
            ItemsToBuy           = other.ItemsToBuy;
            undoStack            = other.undoStack;
            redoStack            = other.redoStack;
            AvailableCouponsList = other.AvailableCouponsList;
            totalprice           = other.totalprice;
        }
        return *this;
    }

    ~CART() {}

    // -------- Getters ------------------------------------------------------
    DSAStack<Products> getItemsToBuy() const { return ItemsToBuy; }
    list<Coupons>      getAvailableCoupons() const { return AvailableCouponsList; }
    double             getTotalPrice() const { return totalprice; }

    int size()      const { return ItemsToBuy.size(); }
    bool canUndo()  const { return !undoStack.isEmpty(); }
    bool canRedo()  const { return !redoStack.isEmpty(); }

    // -------- Setters ------------------------------------------------------
    void setItemsToBuy(const DSAStack<Products>& items) { ItemsToBuy = items; }
    void setAvailableCoupons(const list<Coupons>& c)    { AvailableCouponsList = c; }
    void setTotalPrice(double p)                        { totalprice = p; }

    // -------- Public cart operations (these DO record undo history) --------

    // Add a product to the cart.
    void addToCart(Products& newProduct) {
        rawPush(newProduct);
        undoStack.push(CartAction(CartAction::ADD, newProduct));
        redoStack.clear();   // any new action invalidates the redo history
        cout << "[CART] Added: " << newProduct.getname() << endl;
    }

    // Remove a product from the cart.
    void removeFromCart(Products torem) {
        rawRemoveById(torem.getID());
        undoStack.push(CartAction(CartAction::REMOVE, torem));
        redoStack.clear();
        cout << "[CART] Removed: " << torem.getname() << endl;
    }

    // -------- Undo / Redo --------------------------------------------------

    // Undo the most recent action. Returns true if something was undone.
    bool undo() {
        if (undoStack.isEmpty()) return false;
        CartAction last = undoStack.top();
        undoStack.pop();

        // Reverse the action
        if (last.type == CartAction::ADD) {
            rawRemoveById(last.product.getID());
        } else { // REMOVE -> push it back
            rawPush(last.product);
        }
        redoStack.push(last);
        cout << "[CART] Undo " << (last.type == CartAction::ADD ? "ADD" : "REMOVE")
             << " of " << last.product.getname() << endl;
        return true;
    }

    // Redo the most recently-undone action. Returns true on success.
    bool redo() {
        if (redoStack.isEmpty()) return false;
        CartAction last = redoStack.top();
        redoStack.pop();

        // Re-apply the original action
        if (last.type == CartAction::ADD) {
            rawPush(last.product);
        } else {
            rawRemoveById(last.product.getID());
        }
        undoStack.push(last);
        cout << "[CART] Redo " << (last.type == CartAction::ADD ? "ADD" : "REMOVE")
             << " of " << last.product.getname() << endl;
        return true;
    }

    // -------- Coupons ------------------------------------------------------

    // Apply a coupon: returns the discounted total (does NOT mutate totalprice;
    // the UI passes the result to the checkout flow).
    double applyCoupon(const Coupons& coupon) const {
        double discount = (totalprice * coupon.getdiscount()) / 100.0;
        double finalTotal = totalprice - discount;
        if (finalTotal < 0) finalTotal = 0;
        return finalTotal;
    }

    // -------- Bill update --------------------------------------------------
    void updateBill(double amount) {
        totalprice += amount;
        if (totalprice < 0) totalprice = 0;
    }
};

#endif
