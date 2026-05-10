// =============================================================================
// main.cpp
// -----------------------------------------------------------------------------
// Online Shopping Portal --- Qt6 UI driver.
//
// DSA UPGRADE (this revision):
//   - Catalog page: "Sort by" dropdown -> MergeSort with comparator lambdas
//   - Catalog page: Category sidebar built from CategoryTree
//   - Top bar    : "Back" button using a navigation-history DSAStack<int>
//   - Top bar    : "Recently Viewed" strip using a capped DSAStack<Products>
//   - Cart page  : Undo / Redo buttons backed by CART::undo() / redo()
//
// Authors: Ashar Adnan, Umais Mustafa, Najam Nasr
// =============================================================================
#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QPixmap>
#include <QStackedWidget>
#include <QStyleOption>
#include <QPainter>
#include <vector>
#include <QMessageBox>
#include <QSpacerItem>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDialog>
#include <algorithm>

#include "../templates/products/main.h"
#include "../templates/DSAStructures.hpp"
#include "../data/DataArray.h"
#include "./LoginWindow.hpp"
#include "./UserStorage.hpp"
#include "./AdminStorage.hpp"
#include "../templates/userProfile/main.h"
#include "../templates/admin/main.h"
#include "../templates/coupons/main.h"

// =============================================================================
// GLOBAL STATE
// =============================================================================

// Logged-in user (one of these is "active" depending on isLoggedInAsAdmin)
User  currentUser;
Admin currentAdmin;
bool  isLoggedInAsAdmin = false;

// Helper that returns the active username regardless of role
inline std::string activeUsername() {
    return isLoggedInAsAdmin ? currentAdmin.getUsername() : currentUser.getUsername();
}
inline double activeBalance() {
    return isLoggedInAsAdmin ? currentAdmin.getBalance() : currentUser.getBalance();
}
inline CART& activeCart() {
    return isLoggedInAsAdmin ? currentAdmin.getShoppingCart() : currentUser.getShoppingCart();
}
inline void activeAddToCart(Products& p) {
    if (isLoggedInAsAdmin) currentAdmin.getShoppingCart().addToCart(p);
    else                   currentUser.addToCart(p);
}
inline void activeRemoveFromCart(Products& p) {
    if (isLoggedInAsAdmin) currentAdmin.getShoppingCart().removeFromCart(p);
    else                   currentUser.removeFromCart(p);
}
inline void activeSetBalance(double b) {
    if (isLoggedInAsAdmin) currentAdmin.setBalance(b);
    else                   currentUser.setBalance(b);
}
inline double activeLifetimeSpent() {
    return isLoggedInAsAdmin ? currentAdmin.getLifetimeSpent() : currentUser.getLifetimeSpent();
}
inline void activeSetLifetimeSpent(double v) {
    if (isLoggedInAsAdmin) currentAdmin.setLifetimeSpent(v);
    else                   currentUser.setLifetimeSpent(v);
}
inline void activePersist() {
    if (isLoggedInAsAdmin) AdminStorage::saveAdmin(currentAdmin);
    else                   UserStorage::saveUser(currentUser);
}

// Sidebar labels we update from anywhere
QLabel *gUserLabel    = nullptr;
QLabel *gBalanceLabel = nullptr;

// Predefined coupons (unchanged from original)
std::vector<Coupons> availableCoupons = {
    Coupons("SAVE10",    "Get 10% off your purchase",    10),
    Coupons("SAVE20",    "Get 20% off your purchase",    20),
    Coupons("HALF50",    "Get 50% off your purchase",    50),
    Coupons("WELCOME15", "Welcome discount - 15% off",   15),
    Coupons("MEGA30",    "Mega sale - 30% off",          30)
};

// Master product list, loaded once at startup. We never mutate this --
// each catalog rebuild copies & sorts a snapshot.
std::vector<Products> gAllProducts;

// CategoryTree built from gAllProducts at startup
CategoryTree gCategoryTree;

// Currently active category filter ("" means "All Products")
std::string gActiveCategory = "";

// Currently active sort mode (drives the catalog rebuild)
enum SortMode {
    SORT_DEFAULT,        // insertion order (no sort)
    SORT_PRICE_ASC,
    SORT_PRICE_DESC,
    SORT_NAME_ASC,
    SORT_RATING_DESC
};
SortMode gSortMode = SORT_DEFAULT;

// Navigation history (for the Back button) -- stack of page indices
DSAStack<int> gNavHistory;

// Recently viewed products -- capped DSAStack, max 5 items.
// We push when the user clicks "Buy" (i.e. interacts with a product).
DSAStack<Products> gRecentlyViewed;
const int RECENTLY_VIEWED_CAP = 5;

// Forward declarations
QWidget* buildCatalogPage(QStackedWidget* sw = nullptr);
QWidget* buildCartPage(QStackedWidget* stackedWidget = nullptr);
QWidget* buildAdminPage(QStackedWidget* stackedWidget = nullptr);

// =============================================================================
// HELPER: switch to a page AND record nav history
// =============================================================================
//
// Wraps QStackedWidget::setCurrentIndex so every navigation gets pushed onto
// our DSAStack<int>. The Back button pops from this stack.
//
// We DON'T push if we're already on that page (avoids pointless history).
// =============================================================================
void navigateTo(QStackedWidget* sw, int newIndex) {
    if (sw == nullptr) return;
    int current = sw->currentIndex();
    if (current != newIndex) {
        gNavHistory.push(current);     // remember where we came from
        sw->setCurrentIndex(newIndex);
    }
}

// =============================================================================
// HELPER: rebuild the catalog page (called whenever the filter or sort changes)
// =============================================================================
//
// We keep a static reference to the QStackedWidget so the rebuild can splice
// the new page in at index 0.
// =============================================================================
void refreshCatalog(QStackedWidget* sw) {
    if (sw == nullptr) return;
    QWidget* oldCatalog = sw->widget(0);
    if (oldCatalog) {
        sw->removeWidget(oldCatalog);
        delete oldCatalog;
    }
    QWidget* newCatalog = buildCatalogPage(sw);
    sw->insertWidget(0, newCatalog);
    sw->setCurrentIndex(0);
}

// =============================================================================
// DarkWidget --- background painter helper (unchanged)
// =============================================================================
class DarkWidget : public QWidget {
public:
    using QWidget::QWidget;
protected:
    void paintEvent(QPaintEvent *) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
};

// =============================================================================
// Product Card (one card per product on the catalog grid)
// =============================================================================
QFrame* createProductCard(const Products& product) {
    QFrame *card = new QFrame();
    card->setFixedSize(220, 360);     // slightly taller to fit the rating
    card->setStyleSheet(R"(
        QFrame {
            background-color: #1c1c1c;
            border-radius: 12px;
            padding: 10px;
        }
        QFrame:hover {
            background-color: #2a2a2a;
        }
        QLabel {
            color: #ffffff;
        }
        QPushButton {
            background-color: #007bff;
            border-radius: 8px;
            padding: 6px;
            color: white;
        }
        QPushButton:hover {
            background-color: #2690ff;
        }
    )");

    QVBoxLayout *vbox = new QVBoxLayout(card);
    vbox->setAlignment(Qt::AlignTop);

    // Image
    QLabel *img = new QLabel();
    img->setFixedSize(150, 150);
    img->setAlignment(Qt::AlignCenter);
    QPixmap pix(QString::fromStdString(product.getMainImage()));
    if (!pix.isNull()) {
        img->setPixmap(pix.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        img->setStyleSheet("background-color: #333333; border-radius: 10px;");
        img->setText("[image]");
    }

    // Name
    QLabel *name = new QLabel(QString::fromStdString(product.getname()));
    name->setWordWrap(true);
    name->setStyleSheet("font-size: 14px;");

    // Price
    QLabel *price = new QLabel("$" + QString::number(product.getprice(), 'f', 2));
    price->setStyleSheet("font-weight:bold; color:#00ff00;");

    // Rating (NEW)
    QLabel *ratingLabel = new QLabel(
        QString("Rating: %1 / 5.0").arg(product.getrating(), 0, 'f', 1));
    ratingLabel->setStyleSheet("font-size: 11px; color:#ffd700;");

    // Buy Button
    QPushButton *buyBtn = new QPushButton("Buy");
    QObject::connect(buyBtn, &QPushButton::clicked, [product]() {
        // Add to active cart (works for both User and Admin)
        Products &nonConst = const_cast<Products&>(product);
        activeAddToCart(nonConst);

        // Track recently-viewed (capped at 5)
        gRecentlyViewed.pushBounded(product, RECENTLY_VIEWED_CAP);

        QMessageBox::information(nullptr, "Cart",
            QString("'%1' added to cart.\nUse the Cart page to undo.")
                .arg(QString::fromStdString(product.getname())));
    });

    vbox->addWidget(img, 0, Qt::AlignCenter);
    vbox->addSpacing(8);
    vbox->addWidget(name, 0, Qt::AlignCenter);
    vbox->addWidget(price, 0, Qt::AlignCenter);
    vbox->addWidget(ratingLabel, 0, Qt::AlignCenter);
    vbox->addStretch(1);
    vbox->addWidget(buyBtn, 0, Qt::AlignCenter);
    return card;
}

// =============================================================================
// applySortAndFilter: produce the final list to render on the catalog grid.
//
// 1. Filter by gActiveCategory (using CategoryTree).
// 2. Sort using MergeSort with the appropriate comparator lambda.
// =============================================================================
std::vector<Products> applySortAndFilter() {
    // ---- 1. Filter ----
    std::vector<Products> filtered;
    if (gActiveCategory.empty()) {
        filtered = gAllProducts;       // no filter -> all products
    } else {
        // Look up product IDs in the active category via the tree
        std::vector<int> ids = gCategoryTree.getProductIdsInCategory(gActiveCategory);
        for (int id : ids) {
            // Find the product with this id in gAllProducts (linear, n is tiny)
            for (const Products& p : gAllProducts) {
                if (p.getID() == id) { filtered.push_back(p); break; }
            }
        }
    }

    // ---- 2. Sort using our custom MergeSort ----
    switch (gSortMode) {
        case SORT_PRICE_ASC:
            MergeSort::sort(filtered, [](const Products& a, const Products& b) {
                return a.getprice() < b.getprice();
            });
            break;
        case SORT_PRICE_DESC:
            MergeSort::sort(filtered, [](const Products& a, const Products& b) {
                return a.getprice() > b.getprice();
            });
            break;
        case SORT_NAME_ASC:
            MergeSort::sort(filtered, [](const Products& a, const Products& b) {
                return a.getname() < b.getname();
            });
            break;
        case SORT_RATING_DESC:
            MergeSort::sort(filtered, [](const Products& a, const Products& b) {
                return a.getrating() > b.getrating();
            });
            break;
        case SORT_DEFAULT:
        default:
            // leave in insertion order
            break;
    }
    return filtered;
}

// =============================================================================
// Build Catalog Page  (now with: sort dropdown, category sidebar, recently viewed)
// =============================================================================
QWidget* buildCatalogPage(QStackedWidget* sw) {
    QWidget *root = new QWidget();
    root->setStyleSheet("background-color: #0a0a0a;");
    QVBoxLayout *outer = new QVBoxLayout(root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // ---------- TOP BAR: Back button + Sort dropdown + filter label ----------
    QWidget *topBar = new QWidget();
    topBar->setStyleSheet("background-color: #111111;");
    topBar->setFixedHeight(56);
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 8, 20, 8);
    topBarLayout->setSpacing(15);

    // Back button (uses gNavHistory stack)
    QPushButton *backBtn = new QPushButton("< Back");
    backBtn->setFixedHeight(36);
    backBtn->setStyleSheet(
        "QPushButton { background-color: #292929; color: white; "
        "padding: 6px 14px; border-radius: 6px; font-size: 13px; }"
        "QPushButton:hover { background-color: #3a3a3a; }"
        "QPushButton:disabled { color: #555555; }"
    );
    backBtn->setEnabled(!gNavHistory.isEmpty());
    QObject::connect(backBtn, &QPushButton::clicked, [sw]() {
        if (gNavHistory.isEmpty()) return;
        int prevIdx = gNavHistory.top();
        gNavHistory.pop();
        if (sw) sw->setCurrentIndex(prevIdx);
    });
    topBarLayout->addWidget(backBtn);

    // Active filter label
    QString filterText = gActiveCategory.empty()
        ? "Showing: All Products"
        : QString("Showing: %1").arg(QString::fromStdString(gActiveCategory));
    QLabel *filterLabel = new QLabel(filterText);
    filterLabel->setStyleSheet("color: #cccccc; font-size: 14px;");
    topBarLayout->addWidget(filterLabel);

    topBarLayout->addStretch();

    // Sort dropdown (drives MergeSort)
    QLabel *sortLbl = new QLabel("Sort by:");
    sortLbl->setStyleSheet("color: #888888; font-size: 13px;");
    topBarLayout->addWidget(sortLbl);

    QComboBox *sortBox = new QComboBox();
    sortBox->addItem("Default");
    sortBox->addItem("Price: Low to High");
    sortBox->addItem("Price: High to Low");
    sortBox->addItem("Name (A-Z)");
    sortBox->addItem("Rating (High to Low)");
    sortBox->setCurrentIndex((int)gSortMode);
    sortBox->setStyleSheet(
        "QComboBox { background-color: #1c1c1c; color: white; "
        "padding: 6px 10px; border: 1px solid #333; border-radius: 6px; "
        "min-width: 180px; font-size: 13px; }"
    );
    QObject::connect(sortBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [sw](int idx) {
            gSortMode = (SortMode)idx;
            refreshCatalog(sw);
        });
    topBarLayout->addWidget(sortBox);

    outer->addWidget(topBar);

    // ---------- RECENTLY VIEWED STRIP -----------------------------------------
    if (!gRecentlyViewed.isEmpty()) {
        QWidget *rvBar = new QWidget();
        rvBar->setStyleSheet("background-color: #161616;");
        QHBoxLayout *rvLayout = new QHBoxLayout(rvBar);
        rvLayout->setContentsMargins(20, 10, 20, 10);
        rvLayout->setSpacing(10);

        QLabel *rvTitle = new QLabel("Recently Viewed:");
        rvTitle->setStyleSheet("color: #888; font-size: 12px;");
        rvLayout->addWidget(rvTitle);

        // Snapshot the stack as a vector (top->bottom = most recent first)
        std::vector<Products> recents = gRecentlyViewed.toVector();
        for (const Products& p : recents) {
            QLabel *chip = new QLabel(QString::fromStdString(p.getname()));
            chip->setStyleSheet(
                "background-color: #292929; color: #ddd; "
                "padding: 4px 10px; border-radius: 10px; font-size: 11px;"
            );
            rvLayout->addWidget(chip);
        }
        rvLayout->addStretch();
        outer->addWidget(rvBar);
    }

    // ---------- BODY: category sidebar + product grid ------------------------
    QWidget *bodyWidget = new QWidget();
    QHBoxLayout *bodyLayout = new QHBoxLayout(bodyWidget);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    // ----- LEFT: Category sidebar (built from CategoryTree) ------------------
    QWidget *catSidebar = new QWidget();
    catSidebar->setFixedWidth(180);
    catSidebar->setStyleSheet("background-color: #141414;");
    QVBoxLayout *catLayout = new QVBoxLayout(catSidebar);
    catLayout->setContentsMargins(15, 15, 15, 15);
    catLayout->setSpacing(8);

    QLabel *catHeader = new QLabel("Categories");
    catHeader->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    catLayout->addWidget(catHeader);

    auto makeCatBtn = [&](const QString& label, const std::string& filter) {
        QPushButton *b = new QPushButton(label);
        b->setStyleSheet(
            "QPushButton { background-color: transparent; color: #ccc; "
            "text-align: left; padding: 8px 10px; border-radius: 6px; "
            "font-size: 13px; }"
            "QPushButton:hover { background-color: #222; color: white; }"
        );
        QObject::connect(b, &QPushButton::clicked, [sw, filter]() {
            gActiveCategory = filter;
            refreshCatalog(sw);
        });
        return b;
    };

    catLayout->addWidget(makeCatBtn("All Products", ""));

    // Get categories from the tree (this is the part the proposal promised)
    std::vector<std::string> cats = gCategoryTree.getAllCategories();
    std::sort(cats.begin(), cats.end());
    for (const std::string& c : cats) {
        catLayout->addWidget(makeCatBtn(QString::fromStdString(c), c));
    }
    catLayout->addStretch();
    bodyLayout->addWidget(catSidebar);

    // ----- RIGHT: Product grid -----------------------------------------------
    QWidget *catalog = new QWidget();
    QGridLayout *grid = new QGridLayout(catalog);
    grid->setSpacing(20);
    grid->setContentsMargins(20, 20, 20, 20);

    std::vector<Products> displayed = applySortAndFilter();
    if (displayed.empty()) {
        QLabel *emptyLbl = new QLabel("No products in this category.");
        emptyLbl->setStyleSheet("color: #666; font-size: 14px;");
        grid->addWidget(emptyLbl, 0, 0);
    } else {
        for (size_t i = 0; i < displayed.size(); ++i) {
            QFrame *card = createProductCard(displayed[i]);
            grid->addWidget(card, static_cast<int>(i / 3), static_cast<int>(i % 3));
        }
    }

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(catalog);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");
    bodyLayout->addWidget(scrollArea, 1);

    outer->addWidget(bodyWidget, 1);
    return root;
}

// =============================================================================
// Cart Page (now with Undo / Redo buttons)
// =============================================================================
QWidget* buildCartPage(QStackedWidget *stackedWidget) {
    QWidget *cartPage = new QWidget();
    cartPage->setStyleSheet("background-color: #0a0a0a;");

    QHBoxLayout *mainLayout = new QHBoxLayout(cartPage);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(30);

    // ============= LEFT SIDE: Cart Items + Undo/Redo =========================
    QWidget *itemsContainer = new QWidget();
    itemsContainer->setStyleSheet("background-color: transparent;");
    QVBoxLayout *itemsLayout = new QVBoxLayout(itemsContainer);
    itemsLayout->setContentsMargins(0, 0, 0, 0);
    itemsLayout->setSpacing(20);

    // Title row with undo/redo buttons
    QHBoxLayout *titleRow = new QHBoxLayout();
    QLabel *title = new QLabel("Your Cart");
    title->setStyleSheet("color: #ffffff; font-size: 28px; font-weight: bold;");
    titleRow->addWidget(title);
    titleRow->addStretch();

    CART &cart = activeCart();

    QPushButton *undoBtn = new QPushButton("Undo");
    QPushButton *redoBtn = new QPushButton("Redo");
    QString undoStyle =
        "QPushButton { background-color: #2a2a2a; color: white; "
        "padding: 8px 16px; border-radius: 6px; font-size: 13px; "
        "border: 1px solid #444; }"
        "QPushButton:hover { background-color: #3a3a3a; }"
        "QPushButton:disabled { color: #555; border-color: #2a2a2a; }";
    undoBtn->setStyleSheet(undoStyle);
    redoBtn->setStyleSheet(undoStyle);
    undoBtn->setEnabled(cart.canUndo());
    redoBtn->setEnabled(cart.canRedo());
    titleRow->addWidget(undoBtn);
    titleRow->addWidget(redoBtn);
    itemsLayout->addLayout(titleRow);

    QObject::connect(undoBtn, &QPushButton::clicked, [stackedWidget]() {
        if (activeCart().undo()) {
            activePersist();
            // Refresh the cart page so the UI reflects the new state
            if (stackedWidget) {
                QWidget *oldCart = stackedWidget->widget(1);
                if (oldCart) {
                    stackedWidget->removeWidget(oldCart);
                    delete oldCart;
                }
                QWidget *newCartPage = buildCartPage(stackedWidget);
                stackedWidget->insertWidget(1, newCartPage);
                stackedWidget->setCurrentIndex(1);
            }
        }
    });
    QObject::connect(redoBtn, &QPushButton::clicked, [stackedWidget]() {
        if (activeCart().redo()) {
            activePersist();
            if (stackedWidget) {
                QWidget *oldCart = stackedWidget->widget(1);
                if (oldCart) {
                    stackedWidget->removeWidget(oldCart);
                    delete oldCart;
                }
                QWidget *newCartPage = buildCartPage(stackedWidget);
                stackedWidget->insertWidget(1, newCartPage);
                stackedWidget->setCurrentIndex(1);
            }
        }
    });

    // Scroll area for cart items
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(R"(
        QScrollArea { border: none; background-color: transparent; }
        QScrollBar:vertical { background-color: #1a1a1a; width: 8px; border-radius: 4px; }
        QScrollBar::handle:vertical { background-color: #333333; border-radius: 4px; min-height: 20px; }
        QScrollBar::handle:vertical:hover { background-color: #444444; }
    )");
    QWidget *container = new QWidget();
    QVBoxLayout *listLayout = new QVBoxLayout(container);
    listLayout->setSpacing(15);
    listLayout->setContentsMargins(0, 0, 0, 0);

    // Snapshot the cart's DSAStack as a vector for iteration
    DSAStack<Products> stackCopy = cart.getItemsToBuy();
    std::vector<Products> productsList = stackCopy.toVector();

    // Render one row per item
    for (size_t i = 0; i < productsList.size(); ++i) {
        Products p = productsList[i];

        QFrame *itemFrame = new QFrame();
        itemFrame->setStyleSheet(R"(
            QFrame {
                background-color: #1a1a1a;
                border: 1px solid #2a2a2a;
                border-radius: 12px;
                padding: 0px;
            }
            QFrame:hover { border-color: #333333; }
        )");
        itemFrame->setMinimumHeight(120);

        QHBoxLayout *h = new QHBoxLayout(itemFrame);
        h->setContentsMargins(15, 15, 15, 15);
        h->setSpacing(15);

        QCheckBox *cb = new QCheckBox();
        cb->setChecked(true);
        cb->setStyleSheet(R"(
            QCheckBox::indicator {
                width: 20px; height: 20px; border: 2px solid #444;
                border-radius: 4px; background-color: #1a1a1a;
            }
            QCheckBox::indicator:checked {
                background-color: #4a9eff; border-color: #4a9eff;
            }
        )");
        h->addWidget(cb);

        QLabel *img = new QLabel();
        img->setFixedSize(90, 90);
        img->setStyleSheet("background-color: #0a0a0a; border-radius: 8px; border: 1px solid #2a2a2a;");
        QPixmap pix(QString::fromStdString(p.getMainImage()));
        if (!pix.isNull()) {
            img->setPixmap(pix.scaled(90, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            img->setText("[img]");
            img->setAlignment(Qt::AlignCenter);
        }
        h->addWidget(img);

        QVBoxLayout *infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(6);
        infoLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *name = new QLabel(QString::fromStdString(p.getname()));
        name->setStyleSheet("color: white; font-size: 16px; font-weight: 500;");
        name->setWordWrap(true);
        infoLayout->addWidget(name);

        QLabel *features = new QLabel(QString::fromStdString(p.getfeatures()));
        features->setStyleSheet("color: #888; font-size: 12px;");
        features->setWordWrap(true);
        infoLayout->addWidget(features);

        QLabel *price = new QLabel("$" + QString::number(p.getprice(), 'f', 2));
        price->setStyleSheet("color: #00ff88; font-size: 18px; font-weight: bold;");
        infoLayout->addWidget(price);
        infoLayout->addStretch();
        h->addLayout(infoLayout, 1);

        QPushButton *removeBtn = new QPushButton("X");
        removeBtn->setFixedSize(32, 32);
        removeBtn->setStyleSheet(R"(
            QPushButton { background-color: transparent; border: 1px solid #333;
                          border-radius: 6px; color: #888; font-weight: bold; }
            QPushButton:hover { background-color: #2a2a2a; border-color: #ff4444; color: #ff4444; }
        )");
        Products productToRemove = p;
        QObject::connect(removeBtn, &QPushButton::clicked, [productToRemove, stackedWidget]() {
            Products copy = productToRemove;
            activeRemoveFromCart(copy);
            activePersist();
            if (stackedWidget) {
                QWidget *oldCart = stackedWidget->widget(1);
                if (oldCart) {
                    stackedWidget->removeWidget(oldCart);
                    delete oldCart;
                }
                QWidget *newCartPage = buildCartPage(stackedWidget);
                stackedWidget->insertWidget(1, newCartPage);
                stackedWidget->setCurrentIndex(1);
            }
        });
        h->addWidget(removeBtn);
        listLayout->addWidget(itemFrame);
    }

    if (productsList.empty()) {
        QLabel *emptyLabel = new QLabel("Your cart is empty");
        emptyLabel->setStyleSheet("color: #666666; font-size: 16px; padding: 40px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        listLayout->addWidget(emptyLabel);
    }

    listLayout->addStretch(1);
    scroll->setWidget(container);
    itemsLayout->addWidget(scroll, 1);

    // ============= RIGHT SIDE: Summary (price + coupon + checkout) ===========
    QWidget *summaryContainer = new QWidget();
    summaryContainer->setFixedWidth(320);
    QVBoxLayout *summaryLayout = new QVBoxLayout(summaryContainer);
    summaryLayout->setContentsMargins(0, 0, 0, 0);
    summaryLayout->setSpacing(20);

    QFrame *summaryCard = new QFrame();
    summaryCard->setStyleSheet(R"(
        QFrame {
            background-color: #1a1a1a;
            border: 1px solid #2a2a2a;
            border-radius: 12px;
            padding: 20px;
        }
    )");
    QVBoxLayout *cardLayout = new QVBoxLayout(summaryCard);
    cardLayout->setSpacing(15);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *priceTitle = new QLabel("Price Details");
    priceTitle->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    cardLayout->addWidget(priceTitle);

    // Calculate subtotal by walking the cart's DSAStack
    double subtotal = 0;
    DSAStack<Products> walk = cart.getItemsToBuy();
    while (!walk.isEmpty()) {
        subtotal += walk.top().getprice();
        walk.pop();
    }

    QHBoxLayout *itemsRow = new QHBoxLayout();
    QLabel *itemsLabel = new QLabel(
        QString("%1 item%2").arg(productsList.size())
                            .arg(productsList.size() != 1 ? "s" : ""));
    itemsLabel->setStyleSheet("color: #888; font-size: 14px;");
    QLabel *itemsPrice = new QLabel("$" + QString::number(subtotal, 'f', 2));
    itemsPrice->setStyleSheet("color: white; font-size: 14px;");
    itemsRow->addWidget(itemsLabel);
    itemsRow->addStretch();
    itemsRow->addWidget(itemsPrice);
    cardLayout->addLayout(itemsRow);

    QHBoxLayout *deliveryRow = new QHBoxLayout();
    QLabel *deliveryLabel = new QLabel("Delivery Charges");
    deliveryLabel->setStyleSheet("color: #888; font-size: 14px;");
    QLabel *deliveryPrice = new QLabel("Free Delivery");
    deliveryPrice->setStyleSheet("color: #00ff88; font-size: 14px; font-weight: bold;");
    deliveryRow->addWidget(deliveryLabel);
    deliveryRow->addStretch();
    deliveryRow->addWidget(deliveryPrice);
    cardLayout->addLayout(deliveryRow);

    cardLayout->addSpacing(10);
    QLabel *couponLabel = new QLabel("Have a coupon?");
    couponLabel->setStyleSheet("color: #888; font-size: 13px; font-weight: bold;");
    cardLayout->addWidget(couponLabel);

    QHBoxLayout *couponInputRow = new QHBoxLayout();
    couponInputRow->setSpacing(8);
    QLineEdit *couponInput = new QLineEdit();
    couponInput->setPlaceholderText("Enter coupon code");
    couponInput->setStyleSheet(
        "QLineEdit { background-color: #0a0a0a; border: 1px solid #444; "
        "border-radius: 6px; padding: 8px; color: white; font-size: 13px; }"
        "QLineEdit:focus { border-color: #4a9eff; }"
    );
    couponInputRow->addWidget(couponInput);

    QPushButton *applyBtn = new QPushButton("Apply");
    applyBtn->setFixedWidth(70);
    applyBtn->setStyleSheet(
        "QPushButton { background-color: #4a9eff; color: white; "
        "padding: 8px; border-radius: 6px; font-weight: bold; }"
        "QPushButton:hover { background-color: #3a8eef; }"
    );
    couponInputRow->addWidget(applyBtn);
    cardLayout->addLayout(couponInputRow);

    QHBoxLayout *discountRow = new QHBoxLayout();
    QLabel *discountLabel = new QLabel("Discount");
    discountLabel->setStyleSheet("color: #888; font-size: 14px;");
    QLabel *discountAmount = new QLabel("- $0.00");
    discountAmount->setStyleSheet("color: #ff6b6b; font-size: 14px; font-weight: bold;");
    discountRow->addWidget(discountLabel);
    discountRow->addStretch();
    discountRow->addWidget(discountAmount);
    discountLabel->setVisible(false);
    discountAmount->setVisible(false);
    cardLayout->addLayout(discountRow);

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("QFrame { background-color: #2a2a2a; max-height: 1px; }");
    cardLayout->addWidget(separator);

    QHBoxLayout *totalRow = new QHBoxLayout();
    QLabel *totalLabel = new QLabel("Total Amount");
    totalLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    QLabel *totalPrice = new QLabel("$" + QString::number(subtotal, 'f', 2));
    totalPrice->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    totalRow->addWidget(totalLabel);
    totalRow->addStretch();
    totalRow->addWidget(totalPrice);
    cardLayout->addLayout(totalRow);

    summaryLayout->addWidget(summaryCard);

    totalPrice->setProperty("appliedDiscount", 0.0);
    totalPrice->setProperty("originalSubtotal", subtotal);

    QObject::connect(applyBtn, &QPushButton::clicked,
        [couponInput, applyBtn, discountLabel, discountAmount, totalPrice, subtotal]() {
        QString code = couponInput->text().trimmed().toUpper();
        if (code.isEmpty()) {
            QMessageBox::warning(nullptr, "Invalid Coupon", "Please enter a coupon code.");
            return;
        }
        bool valid = false;
        int pct = 0;
        for (const auto& c : availableCoupons) {
            if (QString::fromStdString(c.getname()).toUpper() == code) {
                valid = true; pct = c.getdiscount(); break;
            }
        }
        if (valid) {
            double disc = (subtotal * pct) / 100.0;
            double finalTotal = subtotal - disc;
            totalPrice->setProperty("appliedDiscount", disc);
            discountLabel->setVisible(true);
            discountAmount->setVisible(true);
            discountAmount->setText("- $" + QString::number(disc, 'f', 2)
                                  + QString(" (%1%)").arg(pct));
            totalPrice->setText("$" + QString::number(finalTotal, 'f', 2));
            couponInput->setEnabled(false);
            applyBtn->setEnabled(false);
            applyBtn->setText("Applied");
            QMessageBox::information(nullptr, "Coupon Applied",
                QString("Coupon '%1' applied! You saved $%2 (%3%% off).")
                    .arg(code).arg(disc, 0, 'f', 2).arg(pct));
        } else {
            QMessageBox::warning(nullptr, "Invalid Coupon",
                QString("Coupon code '%1' is not valid.\n\nTry: SAVE10, SAVE20, "
                        "HALF50, WELCOME15, MEGA30").arg(code));
        }
    });

    QPushButton *checkoutBtn = new QPushButton("Checkout");
    checkoutBtn->setMinimumHeight(52);
    checkoutBtn->setStyleSheet(
        "QPushButton { background-color: #4a9eff; color: white; "
        "font-size: 16px; font-weight: bold; padding: 14px; border-radius: 8px; }"
        "QPushButton:hover { background-color: #3a8eef; }"
    );
    summaryLayout->addWidget(checkoutBtn);
    summaryLayout->addStretch();

    QObject::connect(checkoutBtn, &QPushButton::clicked, [totalPrice, stackedWidget]() {
        double originalTotal = activeCart().getTotalPrice();
        double appliedDiscount = totalPrice->property("appliedDiscount").toDouble();
        double finalTotal = originalTotal - appliedDiscount;

        if (originalTotal == 0.0) {
            QMessageBox::warning(nullptr, "Empty Cart", "Your cart is empty.");
            return;
        }
        double bal = activeBalance();
        if (bal < finalTotal) {
            QMessageBox::warning(nullptr, "Insufficient funds",
                QString("Total: $%1\nYour Balance: $%2")
                    .arg(finalTotal, 0, 'f', 2).arg(bal, 0, 'f', 2));
            return;
        }

        activeSetBalance(bal - finalTotal);
        activeSetLifetimeSpent(activeLifetimeSpent() + finalTotal);

        // Clear the cart by repeatedly removing the top
        CART &cart = activeCart();
        while (!cart.getItemsToBuy().isEmpty()) {
            DSAStack<Products> snap = cart.getItemsToBuy();
            Products p = snap.top();
            activeRemoveFromCart(p);
        }
        activePersist();

        if (gBalanceLabel) {
            gBalanceLabel->setText(
                QString("Balance: $%1").arg(activeBalance(), 0, 'f', 2));
        }
        QString message = "Payment successful! Your order is on its way.";
        if (appliedDiscount > 0) {
            message += QString("\n\nYou saved $%1 with your coupon!")
                          .arg(appliedDiscount, 0, 'f', 2);
        }
        QMessageBox::information(nullptr, "Success", message);

        if (stackedWidget) {
            QWidget *oldCart = stackedWidget->widget(1);
            if (oldCart) {
                stackedWidget->removeWidget(oldCart);
                delete oldCart;
            }
            QWidget *newCartPage = buildCartPage(stackedWidget);
            stackedWidget->insertWidget(1, newCartPage);
            stackedWidget->setCurrentIndex(1);
        }
    });

    mainLayout->addWidget(itemsContainer, 2);
    mainLayout->addWidget(summaryContainer, 1);
    return cartPage;
}

// =============================================================================
// Admin Page (unchanged from original)
// =============================================================================
QWidget* buildAdminPage(QStackedWidget *stackedWidget) {
    QWidget *adminPage = new QWidget();
    adminPage->setStyleSheet("background-color: #0a0a0a;");

    QVBoxLayout *mainLayout = new QVBoxLayout(adminPage);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    QLabel *title = new QLabel("Admin Panel - User Management");
    title->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    mainLayout->addWidget(title);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { border: none; background-color: transparent; }"
        "QScrollBar:vertical { background-color: #1a1a1a; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background-color: #333; border-radius: 4px; min-height: 20px; }"
    );

    QWidget *container = new QWidget();
    QVBoxLayout *listLayout = new QVBoxLayout(container);
    listLayout->setSpacing(15);
    listLayout->setContentsMargins(0, 0, 0, 0);

    std::map<std::string, User> allUsers = UserStorage::loadAllUsers();

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame { background-color: #1a1a1a; border: 1px solid #2a2a2a; "
        "border-radius: 8px; padding: 0px; }"
    );
    headerFrame->setMinimumHeight(50);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    headerLayout->setSpacing(15);
    auto makeHeader = [&](const QString& t, int w) {
        QLabel *l = new QLabel(t);
        l->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
        l->setFixedWidth(w);
        return l;
    };
    headerLayout->addWidget(makeHeader("Username", 200));
    headerLayout->addWidget(makeHeader("Type", 120));
    headerLayout->addWidget(makeHeader("Balance", 150));
    headerLayout->addWidget(makeHeader("Lifetime Spent", 150));
    headerLayout->addStretch();
    headerLayout->addWidget(makeHeader("Actions", 100));
    listLayout->addWidget(headerFrame);

    for (const auto& pair : allUsers) {
        const User& user = pair.second;
        std::string username = user.getUsername();

        QFrame *userFrame = new QFrame();
        userFrame->setStyleSheet(
            "QFrame { background-color: #1a1a1a; border: 1px solid #2a2a2a; "
            "border-radius: 8px; padding: 0px; }"
            "QFrame:hover { border-color: #333; background-color: #1f1f1f; }"
        );
        userFrame->setMinimumHeight(70);
        QHBoxLayout *h = new QHBoxLayout(userFrame);
        h->setContentsMargins(20, 15, 20, 15);
        h->setSpacing(15);

        QLabel *un = new QLabel(QString::fromStdString(username));
        un->setStyleSheet("color: white; font-size: 14px; font-weight: 500;");
        un->setFixedWidth(200);
        h->addWidget(un);

        QLabel *tp = new QLabel(QString::fromStdString(user.getType()));
        tp->setStyleSheet(user.getType() == "admin"
            ? "color: #ff9900; font-size: 14px; font-weight: bold;"
            : "color: #4a9eff; font-size: 14px;");
        tp->setFixedWidth(120);
        h->addWidget(tp);

        QLabel *bl = new QLabel("$" + QString::number(user.getBalance(), 'f', 2));
        bl->setStyleSheet("color: #00ff88; font-size: 14px; font-weight: 500;");
        bl->setFixedWidth(150);
        h->addWidget(bl);

        QLabel *sp = new QLabel("$" + QString::number(user.getLifetimeSpent(), 'f', 2));
        sp->setStyleSheet("color: #888; font-size: 14px;");
        sp->setFixedWidth(150);
        h->addWidget(sp);

        h->addStretch();

        QPushButton *deleteBtn = new QPushButton("Delete");
        deleteBtn->setFixedSize(80, 36);
        deleteBtn->setStyleSheet(
            "QPushButton { background-color: #2a2a2a; border: 1px solid #444; "
            "border-radius: 6px; color: #ff4444; font-weight: bold; }"
            "QPushButton:hover { background-color: #ff4444; color: white; }"
            "QPushButton:disabled { background-color: #1a1a1a; color: #555; }"
        );
        if (username == activeUsername()) {
            deleteBtn->setEnabled(false);
            deleteBtn->setToolTip("Cannot delete your own account");
        }
        QObject::connect(deleteBtn, &QPushButton::clicked, [username, stackedWidget]() {
            auto reply = QMessageBox::question(nullptr, "Confirm Deletion",
                QString("Delete user '%1'? This cannot be undone.")
                    .arg(QString::fromStdString(username)),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                if (UserStorage::deleteUser(username)) {
                    QMessageBox::information(nullptr, "Success",
                        QString("User '%1' deleted.")
                            .arg(QString::fromStdString(username)));
                    if (stackedWidget) {
                        QWidget *oldAdmin = stackedWidget->widget(2);
                        if (oldAdmin) {
                            stackedWidget->removeWidget(oldAdmin);
                            delete oldAdmin;
                        }
                        QWidget *newAdminPage = buildAdminPage(stackedWidget);
                        stackedWidget->insertWidget(2, newAdminPage);
                        stackedWidget->setCurrentIndex(2);
                    }
                } else {
                    QMessageBox::warning(nullptr, "Error",
                        QString("Failed to delete user '%1'.")
                            .arg(QString::fromStdString(username)));
                }
            }
        });
        h->addWidget(deleteBtn);
        listLayout->addWidget(userFrame);
    }

    if (allUsers.empty()) {
        QLabel *emptyLabel = new QLabel("No users found");
        emptyLabel->setStyleSheet("color: #666; font-size: 16px; padding: 40px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        listLayout->addWidget(emptyLabel);
    }

    listLayout->addStretch(1);
    scroll->setWidget(container);
    mainLayout->addWidget(scroll, 1);

    QLabel *infoLabel = new QLabel(QString("Total Users: %1").arg(allUsers.size()));
    infoLabel->setStyleSheet("color: #888; font-size: 14px; padding: 10px;");
    mainLayout->addWidget(infoLabel);
    return adminPage;
}

// =============================================================================
// MAIN
// =============================================================================
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Login
    LoginWindow loginWindow;
    if (loginWindow.exec() != QDialog::Accepted) return 1;

    isLoggedInAsAdmin = loginWindow.isAdminUser();
    if (isLoggedInAsAdmin) {
        currentAdmin = loginWindow.getCurrentAdmin();
        if (currentAdmin.getUsername().empty()) {
            QMessageBox::warning(nullptr, "Error", "Failed to load admin.");
            return 1;
        }
    } else {
        currentUser = loginWindow.getCurrentUser();
        if (currentUser.getUsername().empty()) {
            QMessageBox::warning(nullptr, "Error", "Failed to load user.");
            return 1;
        }
    }

    // -------- Load product database & build CategoryTree (one-time setup) ----
    gAllProducts = InitDBFrame();
    gCategoryTree.buildFrom(gAllProducts);

    // Optional debug print -- great for showing the tree in viva
    std::cout << "[CategoryTree built]\n" << gCategoryTree.toString() << std::endl;

    // -------- Main window layout ---------------------------------------------
    QWidget *mainWindow = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWindow);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    QVBoxLayout *sidebarLayout = new QVBoxLayout();
    sidebarLayout->setContentsMargins(20, 20, 20, 20);
    sidebarLayout->setSpacing(20);

    QLabel *menuLabel = new QLabel("Menu");
    menuLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    sidebarLayout->addWidget(menuLabel);

    QPushButton *catalogBtn = new QPushButton("Catalog");
    catalogBtn->setStyleSheet("background-color: #292929; color: white; padding: 10px; border-radius: 8px;");
    sidebarLayout->addWidget(catalogBtn);

    QPushButton *cartBtn = new QPushButton("Cart");
    cartBtn->setStyleSheet("background-color: #292929; color: white; padding: 10px; border-radius: 8px;");
    sidebarLayout->addWidget(cartBtn);

    QPushButton *adminBtn = nullptr;
    if (isLoggedInAsAdmin) {
        adminBtn = new QPushButton("Admin Panel");
        adminBtn->setStyleSheet(
            "background-color: #ff9900; color: white; padding: 10px; "
            "border-radius: 8px; font-weight: bold;"
        );
        sidebarLayout->addWidget(adminBtn);
    }

    sidebarLayout->addStretch(1);

    QString userLabelText = isLoggedInAsAdmin
        ? QString("Admin: %1").arg(QString::fromStdString(currentAdmin.getUsername()))
        : QString("User: %1").arg(QString::fromStdString(currentUser.getUsername()));
    gUserLabel = new QLabel(userLabelText);
    gUserLabel->setStyleSheet(
        QString("color: %1; font-size: 13px;")
            .arg(isLoggedInAsAdmin ? "#ff9900" : "#bbb")
    );
    sidebarLayout->addWidget(gUserLabel);

    gBalanceLabel = new QLabel(
        QString("Balance: $%1").arg(activeBalance(), 0, 'f', 2));
    gBalanceLabel->setStyleSheet("color: #bbb; font-size: 13px;");
    sidebarLayout->addWidget(gBalanceLabel);

    QPushButton *signOutBtn = new QPushButton("Sign Out");
    signOutBtn->setStyleSheet("background-color: #444; color: white; padding: 8px; border-radius: 6px;");
    sidebarLayout->addWidget(signOutBtn);

    QWidget *sidebar = new QWidget();
    sidebar->setLayout(sidebarLayout);
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet("background-color: #181818;");

    // Stacked widget
    QStackedWidget *stackedWidget = new QStackedWidget();
    QWidget *catalogPage = buildCatalogPage(stackedWidget);
    QWidget *cartPage    = buildCartPage(stackedWidget);
    stackedWidget->addWidget(catalogPage);   // index 0
    stackedWidget->addWidget(cartPage);      // index 1

    if (isLoggedInAsAdmin) {
        QWidget *adminPage = buildAdminPage(stackedWidget);
        stackedWidget->addWidget(adminPage); // index 2
    }

    QObject::connect(catalogBtn, &QPushButton::clicked, [stackedWidget]() {
        navigateTo(stackedWidget, 0);
    });
    QObject::connect(cartBtn, &QPushButton::clicked, [stackedWidget]() {
        // Rebuild so undo/redo button states match latest cart
        QWidget *oldCart = stackedWidget->widget(1);
        if (oldCart) {
            stackedWidget->removeWidget(oldCart);
            delete oldCart;
        }
        QWidget *newCartPage = buildCartPage(stackedWidget);
        stackedWidget->insertWidget(1, newCartPage);
        navigateTo(stackedWidget, 1);
    });
    if (adminBtn) {
        QObject::connect(adminBtn, &QPushButton::clicked, [stackedWidget]() {
            QWidget *oldAdmin = stackedWidget->widget(2);
            if (oldAdmin) {
                stackedWidget->removeWidget(oldAdmin);
                delete oldAdmin;
            }
            QWidget *newAdminPage = buildAdminPage(stackedWidget);
            stackedWidget->insertWidget(2, newAdminPage);
            navigateTo(stackedWidget, 2);
        });
    }

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(stackedWidget);
    mainWindow->resize(1280, 800);
    mainWindow->show();

    QObject::connect(signOutBtn, &QPushButton::clicked, &app, &QApplication::quit);
    return app.exec();
}
