// =============================================================================
// DSAStructures.hpp
// -----------------------------------------------------------------------------
// Custom Data-Structure & Algorithm implementations for the Shopping Portal.
// Written from scratch (no STL containers used internally) so the team can
// clearly demonstrate DS understanding in the viva.
//
//   1. DSAStack<T>      Singly-linked stack (LIFO)
//                       Use cases: cart contents, navigation history,
//                                  undo/redo, recently viewed
//
//   2. CategoryTree     N-ary tree of products
//                       Use case : hierarchical product catalog
//                                  (Root -> Categories -> Products)
//
//   3. MergeSort        Classic divide-and-conquer sort, templated with a
//                       comparator so we can sort by any field.
//                       Use case : sorting products by price / name / rating
//
// Authors: Ashar Adnan (24K-0617), Umais Mustafa (24K-0693), Najam Nasr (24K-0754)
// Course : Data Structures (Sir Shafique Rehman)
// =============================================================================
#ifndef DSA_STRUCTURES_HPP
#define DSA_STRUCTURES_HPP

#include <string>
#include <vector>
#include <stdexcept>
// NOTE: We deliberately do NOT include products/main.h here.
// Products includes DSAStructures.hpp (for DSAStack<Reviews>), so an include
// in the other direction would be circular. CategoryTree::buildFrom is
// templated -- the caller's translation unit must have the Products
// definition visible, which is true at every call site.

// =============================================================================
// 1. DSAStack<T> --- Custom templated stack using a singly-linked list
// =============================================================================
//
// Why linked-list instead of array?
//   - Truly dynamic; never has to "resize and copy" like an array-based stack.
//   - All core operations are O(1): push, pop, top, size, empty.
//   - Demonstrates pointer manipulation (good for viva).
//
// LIFO semantics --- "Last In, First Out":
//      push(A); push(B); push(C);
//      pop()  -> C
//      pop()  -> B
//      pop()  -> A
//
// Memory layout (top points to the most recently pushed node):
//      top -> [C|next] -> [B|next] -> [A|nullptr]
// =============================================================================
template <typename T>
class DSAStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value, Node* nxt = nullptr) : data(value), next(nxt) {}
    };

    Node* topNode;       // pointer to the top of the stack
    int   stackSize;     // cached size for O(1) size() query

public:
    // ---------- Constructors / Destructor / Rule of three ------------------
    DSAStack() : topNode(nullptr), stackSize(0) {}

    // Deep-copy constructor: walks the source stack and rebuilds it
    DSAStack(const DSAStack& other) : topNode(nullptr), stackSize(0) {
        copyFrom(other);
    }

    // Copy-assignment: clear self, then deep-copy
    DSAStack& operator=(const DSAStack& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    ~DSAStack() { clear(); }

    // ---------- Core stack operations (O(1) each) --------------------------

    // push: insert at front of linked list
    void push(const T& value) {
        topNode = new Node(value, topNode);   // new node points to old top
        stackSize++;
    }

    // pop: remove front of linked list. Throws if empty.
    void pop() {
        if (isEmpty()) {
            throw std::runtime_error("DSAStack::pop() called on empty stack");
        }
        Node* old = topNode;
        topNode = topNode->next;
        delete old;
        stackSize--;
    }

    // top: peek at top element (read & write reference)
    T& top() {
        if (isEmpty()) {
            throw std::runtime_error("DSAStack::top() called on empty stack");
        }
        return topNode->data;
    }

    // const overload so const stacks can still call top()
    const T& top() const {
        if (isEmpty()) {
            throw std::runtime_error("DSAStack::top() called on empty stack");
        }
        return topNode->data;
    }

    bool isEmpty() const { return topNode == nullptr; }
    int  size()    const { return stackSize; }

    // STL-style alias so existing std::stack<T> call sites still compile
    bool empty()   const { return isEmpty(); }

    // ---------- Helpers used by the UI -------------------------------------

    // Convert stack contents to vector (top of stack -> vector index 0).
    // Useful for "snapshot the stack and iterate over it" without mutating.
    std::vector<T> toVector() const {
        std::vector<T> out;
        out.reserve(stackSize);
        for (Node* cur = topNode; cur != nullptr; cur = cur->next) {
            out.push_back(cur->data);
        }
        return out;
    }

    // Clear all elements (used by destructor & assignment).
    void clear() {
        while (!isEmpty()) {
            pop();
        }
    }

    // Capped-stack push: useful for "Recently Viewed" (max N items).
    // If pushing would exceed the cap, drops the OLDEST (bottom) element
    // before pushing the new one.
    void pushBounded(const T& value, int maxSize) {
        push(value);
        if (stackSize > maxSize) {
            removeBottom();
        }
    }

private:
    // Copy helper: builds a new linked list with the SAME top-to-bottom order
    // as 'other'. Naive copy reverses order, so we copy to a temp vector
    // first then push back in reverse.
    void copyFrom(const DSAStack& other) {
        std::vector<T> snap = other.toVector();        // top-first
        // Push from bottom to top so the resulting stack matches 'other'
        for (int i = (int)snap.size() - 1; i >= 0; --i) {
            push(snap[i]);
        }
    }

    // Drop the bottom (oldest) element. O(n) -- only used by pushBounded.
    void removeBottom() {
        if (isEmpty()) return;
        if (topNode->next == nullptr) {
            // Single element -- just pop it
            pop();
            return;
        }
        Node* cur = topNode;
        while (cur->next->next != nullptr) {
            cur = cur->next;
        }
        delete cur->next;
        cur->next = nullptr;
        stackSize--;
    }
};

// =============================================================================
// 2. CategoryTree --- N-ary tree for hierarchical product catalog
// =============================================================================
//
//                        [ Root: All Products ]
//                       /         |         |
//             [Electronics]  [Footwear]  [Computers] ...
//                /    |
//        [iPhone15] [Pixel8] ...   <- product leaves
//
// Each non-leaf node holds a category name; each leaf holds a Products id
// (we store the id, not a copy of the whole product, to save memory).
//
// Key operations:
//   - insert(category, productId)        O(c) where c = number of categories
//   - getProductsInCategory(category)    O(c + k) where k = items in category
//   - getAllCategories()                 O(c)
//   - getAllProducts()                   O(n) (full tree traversal)
//
// In viva: we say "tree gives us hierarchical browsing in O(c+k) per category
// instead of scanning all n products with O(n) every time the user filters."
// =============================================================================
class CategoryTree {
public:
    struct TreeNode {
        std::string label;             // category name OR "" for root
        bool        isLeaf;            // true = product leaf
        int         productId;         // valid only if isLeaf
        std::vector<TreeNode*> children;

        TreeNode(const std::string& lbl, bool leaf = false, int pid = -1)
            : label(lbl), isLeaf(leaf), productId(pid) {}

        ~TreeNode() {
            for (TreeNode* c : children) delete c;
        }
    };

private:
    TreeNode* root;

    // Find a direct child of 'parent' whose label matches 'name'.
    // Returns nullptr if not present.
    TreeNode* findChild(TreeNode* parent, const std::string& name) const {
        for (TreeNode* c : parent->children) {
            if (!c->isLeaf && c->label == name) return c;
        }
        return nullptr;
    }

public:
    CategoryTree() {
        root = new TreeNode("All Products");
    }

    ~CategoryTree() {
        delete root;
    }

    // Disable copying for simplicity (we own raw pointers).
    CategoryTree(const CategoryTree&) = delete;
    CategoryTree& operator=(const CategoryTree&) = delete;

    // Insert a product under a category. Creates the category if missing.
    void insert(const std::string& category, int productId) {
        TreeNode* catNode = findChild(root, category);
        if (catNode == nullptr) {
            catNode = new TreeNode(category);
            root->children.push_back(catNode);
        }
        catNode->children.push_back(new TreeNode("", true, productId));
    }

    // Build the tree from a flat vector of products.
    // Templated so we don't need to include products/main.h here -- the call
    // site must have the full Products definition visible (it always does).
    template <typename ProductT>
    void buildFrom(const std::vector<ProductT>& products) {
        for (size_t i = 0; i < products.size(); ++i) {
            insert(products[i].getcategory(), products[i].getID());
        }
    }

    // Return all category labels (direct children of root).
    std::vector<std::string> getAllCategories() const {
        std::vector<std::string> out;
        for (TreeNode* c : root->children) {
            if (!c->isLeaf) out.push_back(c->label);
        }
        return out;
    }

    // Return product IDs that live under a given category. Empty if not found.
    std::vector<int> getProductIdsInCategory(const std::string& category) const {
        std::vector<int> out;
        TreeNode* catNode = findChild(root, category);
        if (catNode == nullptr) return out;
        for (TreeNode* leaf : catNode->children) {
            if (leaf->isLeaf) out.push_back(leaf->productId);
        }
        return out;
    }

    // Total products in the tree (sum across categories).
    int totalProductCount() const {
        int total = 0;
        for (TreeNode* c : root->children) {
            if (!c->isLeaf) total += (int)c->children.size();
        }
        return total;
    }

    // Pretty text dump (used in debugging / can be printed in viva demo).
    std::string toString() const {
        std::string out = root->label + "\n";
        for (TreeNode* c : root->children) {
            if (c->isLeaf) continue;
            out += "  |-- " + c->label
                +  " (" + std::to_string(c->children.size()) + " items)\n";
        }
        return out;
    }
};

// =============================================================================
// 3. MergeSort --- divide-and-conquer, templated with a comparator
// =============================================================================
//
// Why MergeSort (vs QuickSort / BubbleSort)?
//   - Guaranteed O(n log n) worst case (QuickSort is O(n^2) worst case).
//   - STABLE: equal elements keep their relative order.
//   - Easy to explain on the board: split, sort halves, merge.
//
// Algorithm:
//      mergeSort(arr, l, r):
//          if l < r:
//              m = (l + r) / 2
//              mergeSort(arr, l,   m)
//              mergeSort(arr, m+1, r)
//              merge(arr, l, m, r)
//
// Complexity:
//      Time : O(n log n)  (best, average, worst -- all the same)
//      Space: O(n)        (the temporary merge buffer)
// =============================================================================
class MergeSort {
public:
    // Sort 'arr' using a binary comparator 'cmp(a, b)' that returns true
    // if 'a' should come BEFORE 'b' in the sorted output.
    //
    // Templated on both the element type T and the comparator type Compare,
    // so we can pass lambdas without runtime overhead (no std::function).
    template <typename T, typename Compare>
    static void sort(std::vector<T>& arr, Compare cmp) {
        if (arr.size() < 2) return;     // already sorted
        mergeSortRec(arr, 0, (int)arr.size() - 1, cmp);
    }

private:
    template <typename T, typename Compare>
    static void mergeSortRec(std::vector<T>& arr, int left, int right, Compare cmp) {
        if (left >= right) return;
        int mid = left + (right - left) / 2;        // safer than (l+r)/2 for huge n
        mergeSortRec(arr, left,    mid,   cmp);
        mergeSortRec(arr, mid + 1, right, cmp);
        merge(arr, left, mid, right, cmp);
    }

    template <typename T, typename Compare>
    static void merge(std::vector<T>& arr, int left, int mid, int right, Compare cmp) {
        // Sizes of the two halves
        int n1 = mid - left + 1;
        int n2 = right - mid;

        // Temporary buffers
        std::vector<T> L; L.reserve(n1);
        std::vector<T> R; R.reserve(n2);
        for (int i = 0; i < n1; ++i) L.push_back(arr[left + i]);
        for (int j = 0; j < n2; ++j) R.push_back(arr[mid + 1 + j]);

        // Merge the buffers back into arr[left..right]
        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            if (cmp(L[i], R[j])) {
                arr[k++] = L[i++];
            } else {
                arr[k++] = R[j++];
            }
        }
        while (i < n1) arr[k++] = L[i++];   // drain leftovers
        while (j < n2) arr[k++] = R[j++];
    }
};

#endif // DSA_STRUCTURES_HPP
