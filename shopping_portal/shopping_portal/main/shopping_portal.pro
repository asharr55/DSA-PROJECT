QT += core gui widgets

CONFIG += c++17

# Source files
SOURCES += main.cpp

# Header files (so qmake tracks them for change detection)
HEADERS += \
    LoginWindow.hpp \
    UserStorage.hpp \
    AdminStorage.hpp \
    ../templates/DSAStructures.hpp \
    ../templates/products/main.h \
    ../templates/userProfile/main.h \
    ../templates/admin/main.h \
    ../templates/Cart/main.h \
    ../templates/account/main.h \
    ../templates/coupons/main.h \
    ../templates/reviews/main.h \
    ../data/DataArray.h

# Output binary name
TARGET = shopping_portal

# (Optional) keep build artifacts out of source dirs
DESTDIR     = $$PWD/build
OBJECTS_DIR = $$PWD/build/.obj
MOC_DIR     = $$PWD/build/.moc
RCC_DIR     = $$PWD/build/.rcc
UI_DIR      = $$PWD/build/.ui
