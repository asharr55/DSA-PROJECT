// test_dsa.cpp -- compile-only sanity check for the non-Qt parts of the project.
//
//   g++ -std=c++17 test_dsa.cpp -o test_dsa && ./test_dsa
//
// Tests:
//   1. DSAStack basics (push/pop/copy)
//   2. pushBounded -- capped recently-viewed
//   3. CART add/remove/undo/redo
//   4. CategoryTree build + lookup
//   5. MergeSort by price ascending
//   6. MergeSort by rating descending
//   7. Reviews integration (DSAStack<Reviews> in Products)
//   8. Admin class -- construct, getters, password check
//
// Does NOT test the Qt UI -- run the full app for that.

#include <iostream>
#include <vector>
#include <cassert>

#include "templates/products/main.h"
#include "templates/coupons/main.h"
#include "templates/account/main.h"
#include "templates/Cart/main.h"
#include "templates/userProfile/main.h"
#include "templates/admin/main.h"
#include "templates/reviews/main.h"
#include "templates/DSAStructures.hpp"
#include "data/DataArray.h"

int main() {
    std::cout << "===== Test 1: DSAStack basics =====\n";
    {
        DSAStack<int> s;
        assert(s.isEmpty());
        for (int i = 1; i <= 5; ++i) s.push(i);
        assert(s.size() == 5);
        assert(s.top() == 5);
        s.pop();
        assert(s.top() == 4);

        std::vector<int> snap = s.toVector();
        assert(snap.size() == 4);
        assert(snap[0] == 4 && snap[3] == 1);

        DSAStack<int> s2 = s;
        assert(s2.size() == 4 && s2.top() == 4);
        std::cout << "OK\n";
    }

    std::cout << "===== Test 2: pushBounded (capped recently-viewed) =====\n";
    {
        DSAStack<int> s;
        for (int i = 1; i <= 8; ++i) s.pushBounded(i, 5);
        assert(s.size() == 5);
        assert(s.top() == 8);
        std::vector<int> snap = s.toVector();
        assert(snap[0] == 8 && snap[4] == 4);
        std::cout << "OK\n";
    }

    std::cout << "===== Test 3: CART add/remove/undo/redo =====\n";
    {
        std::vector<Products> all = InitDBFrame();
        CART cart;
        Products p1 = all[0];
        Products p2 = all[1];
        Products p3 = all[2];
        cart.addToCart(p1);
        cart.addToCart(p2);
        cart.addToCart(p3);
        assert(cart.size() == 3);
        assert(cart.canUndo());

        cart.undo();
        assert(cart.size() == 2);
        assert(cart.canRedo());
        cart.redo();
        assert(cart.size() == 3);
        std::cout << "OK\n";
    }

    std::cout << "===== Test 4: CategoryTree =====\n";
    {
        std::vector<Products> all = InitDBFrame();
        CategoryTree tree;
        tree.buildFrom(all);
        std::vector<std::string> cats = tree.getAllCategories();
        std::cout << "Categories: " << cats.size() << "\n";
        std::cout << tree.toString();
        assert(cats.size() > 0);
        std::vector<int> photos = tree.getProductIdsInCategory("Photography");
        assert(!photos.empty());
        std::cout << "OK\n";
    }

    std::cout << "===== Test 5: MergeSort by price ASC =====\n";
    {
        std::vector<Products> all = InitDBFrame();
        MergeSort::sort(all, [](const Products& a, const Products& b) {
            return a.getprice() < b.getprice();
        });
        for (size_t i = 1; i < all.size(); ++i) {
            assert(all[i-1].getprice() <= all[i].getprice());
        }
        std::cout << "Cheapest: $" << all[0].getprice()
                  << "  " << all[0].getname() << "\nOK\n";
    }

    std::cout << "===== Test 6: MergeSort by rating DESC =====\n";
    {
        std::vector<Products> all = InitDBFrame();
        MergeSort::sort(all, [](const Products& a, const Products& b) {
            return a.getrating() > b.getrating();
        });
        for (size_t i = 1; i < all.size(); ++i) {
            assert(all[i-1].getrating() >= all[i].getrating());
        }
        std::cout << "Top-rated: " << all[0].getrating()
                  << "  " << all[0].getname() << "\nOK\n";
    }

    std::cout << "===== Test 7: Reviews -- DSAStack<Reviews> in Products =====\n";
    {
        std::vector<Products> all = InitDBFrame();
        Products p = all[0];
        assert(p.reviewCount() == 0);
        p.addReview(Reviews("alice", "Great running shoes!"));
        p.addReview(Reviews("bob",   "A bit narrow but comfortable."));
        p.addReview(Reviews("carol", "Best Adidas in years."));
        assert(p.reviewCount() == 3);
        // LIFO -- most recent on top
        DSAStack<Reviews> rs = p.getReviews();
        assert(rs.size() == 3);
        // Reviews don't expose getters, so just check it doesn't crash on copy
        DSAStack<Reviews> rs2 = rs;
        assert(rs2.size() == 3);
        std::cout << "OK\n";
    }

    std::cout << "===== Test 8: Admin class =====\n";
    {
        Admin a("rootuser", 9999, "admin", 5000.0, "Bank Transfer");
        assert(a.getUsername() == "rootuser");
        assert(a.getType() == "admin");
        assert(a.getBalance() == 5000.0);
        assert(a.checkPassword(9999));
        assert(!a.checkPassword(1234));
        // Make sure admin has a working cart too
        std::vector<Products> all = InitDBFrame();
        Products p = all[0];
        a.getShoppingCart().addToCart(p);
        assert(a.getShoppingCart().size() == 1);
        std::cout << "OK\n";
    }

    std::cout << "\nALL TESTS PASSED\n";
    return 0;
}
