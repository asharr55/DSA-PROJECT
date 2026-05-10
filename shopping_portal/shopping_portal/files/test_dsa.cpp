// test_dsa.cpp -- compile-only sanity check for the non-Qt parts of the project.
// Builds: DSAStack, CategoryTree, MergeSort + the entity classes.
//
//   g++ -std=c++17 test_dsa.cpp -o test_dsa && ./test_dsa
//
// This does NOT use Qt, so it only proves the headers compile cleanly and the
// data structures behave correctly. The full app still needs Qt to build.

#include <iostream>
#include <vector>
#include <cassert>

#include "templates/products/main.h"
#include "templates/coupons/main.h"
#include "templates/account/main.h"
#include "templates/Cart/main.h"
#include "templates/userProfile/main.h"
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

        // Snapshot to vector (top -> bottom)
        std::vector<int> snap = s.toVector();
        assert(snap.size() == 4);
        assert(snap[0] == 4 && snap[3] == 1);

        // Copy constructor
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
        // Should be 8,7,6,5,4 (oldest 1,2,3 evicted)
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

        cart.undo();           // removes p3
        assert(cart.size() == 2);
        assert(cart.canRedo());
        cart.redo();           // re-adds p3
        assert(cart.size() == 3);
        std::cout << "OK\n";
    }

    std::cout << "===== Test 4: CategoryTree =====\n";
    {
        std::vector<Products> all = InitDBFrame();
        CategoryTree tree;
        tree.buildFrom(all);
        std::vector<std::string> cats = tree.getAllCategories();
        std::cout << "Categories found: " << cats.size() << "\n";
        std::cout << tree.toString();
        assert(cats.size() > 0);

        std::vector<int> photos = tree.getProductIdsInCategory("Photography");
        std::cout << "Photography product IDs: ";
        for (int id : photos) std::cout << id << " ";
        std::cout << "\n";
        assert(!photos.empty());
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
        std::cout << "Cheapest 3 after sort:\n";
        for (int i = 0; i < 3; ++i) {
            std::cout << "  $" << all[i].getprice() << "  " << all[i].getname() << "\n";
        }
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
        std::cout << "Top-3 rated:\n";
        for (int i = 0; i < 3; ++i) {
            std::cout << "  " << all[i].getrating() << "  " << all[i].getname() << "\n";
        }
    }

    std::cout << "\nALL TESTS PASSED\n";
    return 0;
}
