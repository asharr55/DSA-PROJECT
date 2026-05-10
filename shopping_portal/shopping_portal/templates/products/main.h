#ifndef PRODUCTS_MAIN_H
#define PRODUCTS_MAIN_H

#include <iostream>
#include <string>
using namespace std;

// =============================================================================
// Products
// -----------------------------------------------------------------------------
// Entity class for an item on the store.
//
// CHANGE LOG (DSA upgrade):
//   - Replaced empty `Reviews` placeholder with the real Reviews class
//     defined in templates/reviews/main.h (built by the team).
//   - Reviews are stored in a DSAStack<Reviews> -- LIFO so the most recently
//     posted review appears first when iterating.
//   - Added a `rating` field (0.0 - 5.0) so MergeSort by rating works.
// =============================================================================
#include "../reviews/main.h"
#include "../DSAStructures.hpp"

class Products {
private:
    int    id;
    string name;
    string category;
    double price;
    int    stockQuantity;
    string brand;
    string model;
    string color;
    string dimensions;
    double weight;
    string material;
    string warranty;
    string releaseDate;
    string originCountry;
    string batteryLife;
    double screenSize;
    string processor;
    int    ramGB;
    int    storageGB;
    string connectivity;
    string features;
    string mainImageURL;
    double rating;          // average customer rating, 0.0 - 5.0
    DSAStack<Reviews> reviews;   // stack of customer reviews (most recent on top)

public:
    // ---- FULL CONSTRUCTOR (rating defaults to 0.0 if caller omits it) ----
    Products(int id,
             string name,
             string category,
             double price,
             int    stockQuantity,
             string brand,
             string model,
             string color,
             string dimensions,
             double weight,
             string material,
             string warranty,
             string releaseDate,
             string originCountry,
             string batteryLife,
             double screenSize,
             string processor,
             int    ramGB,
             int    storageGB,
             string connectivity,
             string features,
             string mainImageURL,
             double rating = 0.0)
    {
        this->id            = id;
        this->name          = name;
        this->category      = category;
        this->price         = price;
        this->stockQuantity = stockQuantity;
        this->brand         = brand;
        this->model         = model;
        this->color         = color;
        this->dimensions    = dimensions;
        this->weight        = weight;
        this->material      = material;
        this->warranty      = warranty;
        this->releaseDate   = releaseDate;
        this->originCountry = originCountry;
        this->batteryLife   = batteryLife;
        this->screenSize    = screenSize;
        this->processor     = processor;
        this->ramGB         = ramGB;
        this->storageGB     = storageGB;
        this->connectivity  = connectivity;
        this->features      = features;
        this->mainImageURL  = mainImageURL;
        this->rating        = rating;
    }

    // DEFAULT
    Products()
        : id(0), price(0), stockQuantity(0), weight(0),
          screenSize(0), ramGB(0), storageGB(0), rating(0.0) {}

    // COPY CONSTRUCTOR
    Products(const Products& other) {
        id            = other.id;
        name          = other.name;
        category      = other.category;
        price         = other.price;
        stockQuantity = other.stockQuantity;
        brand         = other.brand;
        model         = other.model;
        color         = other.color;
        dimensions    = other.dimensions;
        weight        = other.weight;
        material      = other.material;
        warranty      = other.warranty;
        releaseDate   = other.releaseDate;
        originCountry = other.originCountry;
        batteryLife   = other.batteryLife;
        screenSize    = other.screenSize;
        processor     = other.processor;
        ramGB         = other.ramGB;
        storageGB     = other.storageGB;
        connectivity  = other.connectivity;
        features      = other.features;
        mainImageURL  = other.mainImageURL;
        rating        = other.rating;
        reviews       = other.reviews;   // DSAStack has deep-copy semantics
    }

    // ASSIGNMENT OPERATOR
    Products& operator=(const Products& other) {
        if (this != &other) {
            id            = other.id;
            name          = other.name;
            category      = other.category;
            price         = other.price;
            stockQuantity = other.stockQuantity;
            brand         = other.brand;
            model         = other.model;
            color         = other.color;
            dimensions    = other.dimensions;
            weight        = other.weight;
            material      = other.material;
            warranty      = other.warranty;
            releaseDate   = other.releaseDate;
            originCountry = other.originCountry;
            batteryLife   = other.batteryLife;
            screenSize    = other.screenSize;
            processor     = other.processor;
            ramGB         = other.ramGB;
            storageGB     = other.storageGB;
            connectivity  = other.connectivity;
            features      = other.features;
            mainImageURL  = other.mainImageURL;
            rating        = other.rating;
            reviews       = other.reviews;
        }
        return *this;
    }

    // ---- GETTERS ----
    int    getID()           const { return id; }
    string getname()         const { return name; }
    string getcategory()     const { return category; }
    double getprice()        const { return price; }
    int    getStock()        const { return stockQuantity; }
    string getbrand()        const { return brand; }
    string getmodel()        const { return model; }
    string getcolor()        const { return color; }
    string getdimensions()   const { return dimensions; }
    double getweight()       const { return weight; }
    string getmaterial()     const { return material; }
    string getwarranty()     const { return warranty; }
    string getreleaseDate()  const { return releaseDate; }
    string getorigin()       const { return originCountry; }
    string getbattery()      const { return batteryLife; }
    double getscreensize()   const { return screenSize; }
    string getprocessor()    const { return processor; }
    int    getram()          const { return ramGB; }
    int    getstorage()      const { return storageGB; }
    string getconnectivity() const { return connectivity; }
    string getfeatures()     const { return features; }
    string getMainImage()    const { return mainImageURL; }
    double getrating()       const { return rating; }   // NEW

    // ---- Review operations (stack of Reviews, LIFO) ----
    void addReview(const Reviews& r) { reviews.push(r); }
    int  reviewCount() const         { return reviews.size(); }
    DSAStack<Reviews> getReviews() const { return reviews; }   // returns copy

    // ---- SETTERS ----
    void setprice(double p)     { price = p; }
    void setStock(int q)        { stockQuantity = q; }
    void setcolor(string c)     { color = c; }
    void setMainImage(string u) { mainImageURL = u; }
    void setrating(double r)    { rating = r; }         // NEW

    ~Products() {}
};

#endif
