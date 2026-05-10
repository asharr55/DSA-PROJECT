#pragma once
#include <vector>
#include "../templates/products/main.h"

// =============================================================================
// InitDBFrame --- in-memory product "database".
// Each product now has a final `rating` argument (0.0 - 5.0).
// Ratings are illustrative defaults; in a real system they'd come from
// aggregated reviews. Used by MergeSort to demonstrate sort-by-rating.
// =============================================================================
inline std::vector<Products> InitDBFrame() {
    std::vector<Products> products;
products.push_back(Products(1001,"Adidas Ultraboost 23","Footwear",179.99,50,"Adidas","Ultraboost 23","White","NA",320,"Knit/Rubber","1 Year","2024-03-15","Vietnam","NA",0,"NA",0,0,"NA","Running Shoes, Boost Cushioning","images/adidas ultra boost.jpg",4.6));

products.push_back(Products(1002,"Apple Watch Series 9","Wearables",399.99,50,"Apple","Watch Series 9","Silver","44 x 38 x 10 mm",50,"Aluminum","1 Year","2025-03-01","USA","18 hours",1.78,"S9 SiP",0,0,"Bluetooth, WiFi","Fitness Tracking, ECG, GPS","../main/images/apple series 9.jpg",4.7));

products.push_back(Products(1003,"Bose SoundLink Revolve+","Audio",299.99,35,"Bose","SoundLink Revolve+","Silver","105 x 105 x 184 mm",900,"Aluminum","1 Year","2024-08-01","USA","12 hours",0,"NA",0,0,"Bluetooth","360 Sound, Waterproof","../main/images/bose soundlink.jpg",4.5));

products.push_back(Products(1004,"Canon EOS R6 Camera","Photography",2499.99,15,"Canon","EOS R6","Black","138 x 98 x 88 mm",680,"Magnesium Alloy","2 Years","2024-02-12","Japan","1,080 shots",3.0,"DIGIC X",0,0,"WiFi, Bluetooth","Full Frame, 4K Video","../main/images/canon EOS r6.jpg",4.8));

products.push_back(Products(1005,"Dell XPS 13 Laptop","Computers",999.99,25,"Dell","XPS 13","Silver","295 x 199 x 14 mm",1200,"Aluminum","1 Year","2025-03-20","USA","12 hours",13.4,"Intel i7-1360P",16,512,"WiFi 6E, Bluetooth 5.3","FHD Display, Fingerprint Reader","../main/images/Dell XPS.jpg",4.4));

products.push_back(Products(1006,"DJI Mini 4 Pro","Photography",759.99,18,"DJI","Mini 4 Pro","White","159 x 203 x 56 mm",249,"Plastic/Metal","1 Year","2024-12-01","China","34 minutes",0,"NA",0,0,"WiFi, GPS","Foldable, 4K Camera","../main/images/DJI mini 4.jpg",4.7));

products.push_back(Products(1007,"Dyson V15 Detect Vacuum","Home Appliances",699.99,28,"Dyson","V15 Detect","Purple","1260 x 250 x 220 mm",3000,"Plastic/Metal","2 Years","2024-10-10","UK","60 min",0,"NA",0,0,"Cordless","Laser Detect, HEPA Filter","../main/images/dyson vaccum.jpg",4.6));

products.push_back(Products(1008,"Amazon Echo Dot (5th Gen)","Smart Home",49.99,200,"Amazon","Echo Dot 5","Charcoal","100 x 100 x 50 mm",300,"Plastic","1 Year","2024-10-01","USA","12 hours",0,"NA",0,0,"WiFi, Bluetooth","Voice Assistant, Smart Home Control","../main/images/echo dot.jpg",4.3));

products.push_back(Products(1009,"Fitbit Charge 6","Wearables",149.99,80,"Fitbit","Charge 6","Black","43 x 22 x 12 mm",30,"Silicone","1 Year","2025-01-01","USA","7 days",1.0,"NA",0,0,"Bluetooth","Heart Rate, Sleep Tracking","../main/images/fitbit.jpg",4.2));

products.push_back(Products(1010,"Samsung Galaxy Buds 3","Audio",179.99,60,"Samsung","Galaxy Buds 3","White","22 x 20 x 24 mm",6.3,"Plastic","1 Year","2024-08-10","South Korea","8 hours",0,"NA",0,0,"Bluetooth 5.2","Noise Cancelling, Wireless Charging","../main/images/galaxy buds 3.jpg",4.4));

products.push_back(Products(1011,"Garmin Fenix 8","Wearables",699.99,45,"Garmin","Fenix 8","Black","47 x 47 x 14 mm",80,"Titanium/Plastic","1 Year","2025-02-01","USA","21 days",1.3,"NA",0,0,"GPS, Bluetooth","Multisport, Solar Charging","../main/images/garmin fenix.jpg",4.8));

products.push_back(Products(1012,"GoPro HERO12 Black","Photography",499.99,22,"GoPro","HERO12","Black","71 x 55 x 33 mm",153,"Plastic","1 Year","2025-01-05","USA","2 hours",0,"GP2",0,0,"WiFi, Bluetooth","4K Video, Waterproof","../main/images/gopro.jpg",4.5));

products.push_back(Products(1013,"HP Spectre x360","Computers",1299.99,20,"HP","Spectre x360","Silver","304 x 203 x 17 mm",1300,"Aluminum","1 Year","2025-01-10","USA","15 hours",13.3,"Intel i7-1360P",16,512,"WiFi 6, Bluetooth 5.3","Convertible, Touchscreen","../main/images/HP spectre.jpg",4.3));

products.push_back(Products(1014,"Apple iPad Pro 12.9","Electronics",1099.99,30,"Apple","iPad Pro 12.9","Silver","280.6 x 214.9 x 6.4 mm",682,"Aluminum/Glass","1 Year","2025-01-20","USA","12 hours",12.9,"Apple M2",16,512,"WiFi 6, 5G, Bluetooth","Liquid Retina XDR, Pencil Support","../main/images/ipad pro.jpg",4.6));

products.push_back(Products(1015,"Apple iPhone 15","Electronics",1199.99,50,"Apple","iPhone 15","Black","146.7 x 71.5 x 7.8 mm",174,"Aluminum/Glass","1 Year","2025-09-01","USA","20 hours",6.1,"A17 Bionic",8,256,"5G, WiFi 6, Bluetooth","Face ID, Dual Cameras, MagSafe","../main/images/iphone-15-pro.jpg",4.7));

products.push_back(Products(1016,"KitchenAid Artisan Mixer","Kitchen",379.99,35,"KitchenAid","Artisan 5QT","Red","357 x 218 x 356 mm",11000,"Metal","1 Year","2024-11-01","USA","NA",0,"NA",0,0,"Electric","Multiple Attachments, 5 QT Bowl","../main/images/kitchenaid artisan.jpg",4.8));

products.push_back(Products(1017,"Logitech MX Master 3 Mouse","Accessories",99.99,100,"Logitech","MX Master 3","Gray","124 x 84 x 51 mm",141,"Plastic","2 Years","2023-11-05","Switzerland","70 days",0,"NA",0,0,"Bluetooth, USB-C","Ergonomic, Customizable Buttons","../main/images/logitech mouse.jpg",4.7));

products.push_back(Products(1018,"Apple MacBook Air M2","Computers",1249.99,18,"Apple","MacBook Air M2","Silver","304 x 212 x 11 mm",1200,"Aluminum","1 Year","2024-07-20","USA","18 hours",13.6,"Apple M2",8,512,"WiFi 6, Bluetooth 5.3","Retina Display, Lightweight","../main/images/mackbook airm2.jpg",4.8));

products.push_back(Products(1019,"Nike Air Zoom Pegasus 40","Footwear",129.99,70,"Nike","Pegasus 40","Blue","NA",300,"Mesh/Rubber","1 Year","2024-07-10","Vietnam","NA",0,"NA",0,0,"NA","Running Shoes","../main/images/nike air zoom.jpg",4.5));

products.push_back(Products(1020,"Nintendo Switch OLED","Gaming",349.99,60,"Nintendo","Switch OLED","White","242 x 102 x 13.9 mm",320,"Plastic","1 Year","2023-09-15","Japan","9 hours",7.0,"Custom NVIDIA Tegra",0,0,"WiFi, Bluetooth","Dockable, OLED Screen","../main/images/nintendo switch.jpg",4.7));

products.push_back(Products(1021,"Google Pixel 8","Electronics",899.99,35,"Google","Pixel 8","Black","146 x 70 x 8 mm",172,"Aluminum/Glass","1 Year","2025-01-15","USA","24 hours",6.2,"Google Tensor G3",8,256,"5G, WiFi 6E, Bluetooth 5.3","Dual Camera, Google AI Features","../main/images/pixel 8.jpg",4.4));

products.push_back(Products(1022,"Instant Pot Duo 7-in-1","Kitchen",89.99,120,"Instant Pot","Duo 7","Silver","30 x 30 x 35 cm",6000,"Stainless Steel","1 Year","2024-05-10","China","NA",0,"NA",0,0,"Electric","Pressure Cooker, Slow Cooker","../main/images/pot duo 7.jpg",4.6));

products.push_back(Products(1023,"Razer Blade 16","Computers",2499.99,12,"Razer","Blade 16","Black","355 x 235 x 20 mm",2200,"Aluminum","1 Year","2025-03-01","USA","8 hours",16.0,"Intel i9-14900H",32,1024,"WiFi 6E, Bluetooth 5.3","Gaming Laptop, RTX 4080","../main/images/razor blade 16.jpg",4.5));

products.push_back(Products(1024,"Samsung Galaxy Tab S9","Electronics",699.99,40,"Samsung","Tab S9","Gray","253 x 165 x 6 mm",500,"Aluminum","1 Year","2025-02-10","South Korea","14 hours",11.0,"Snapdragon 8 Gen 3",8,256,"WiFi, 5G, Bluetooth","AMOLED Display, S Pen","../main/images/samsung galaxy tab s9.jpg",4.5));

products.push_back(Products(1025,"Samsung Galaxy S24","Electronics",1099.99,40,"Samsung","Galaxy S24","Silver","146.0 x 70.5 x 7.6 mm",168,"Aluminum/Glass","1 Year","2025-02-10","South Korea","22 hours",6.2,"Snapdragon 8 Gen 3",12,512,"5G, WiFi 6E, Bluetooth","Triple Camera, Fast Charging","../main/images/samsungs24.jpg",4.6));

products.push_back(Products(1026,"Sony Alpha a7 IV","Photography",2499.99,10,"Sony","Alpha a7 IV","Black","131 x 96 x 80 mm",658,"Magnesium Alloy","2 Years","2024-11-10","Japan","580 shots",3.0,"BIONZ XR",0,0,"WiFi, Bluetooth","Full Frame, 4K Video","../main/images/sony alpha 7 IV.jpg",4.7));

products.push_back(Products(1027,"Sony WH-1000XM5 Headphones","Audio",399.99,30,"Sony","WH-1000XM5","Black","250 x 235 x 50 mm",254,"Plastic/Metal","2 Years","2024-06-15","Japan","30 hours",0,"NA",0,0,"Bluetooth 5.2","Noise Cancelling, Touch Controls","../main/images/Sony-WH-1000XM5-9.jpg",4.8));

products.push_back(Products(1028,"Sony Bravia 65\" 4K TV","Electronics",1499.99,20,"Sony","Bravia 65X90","Black","1449 x 830 x 59 mm",25000,"Plastic/Metal","2 Years","2025-01-01","Japan","NA",65,"NA",0,0,"WiFi, HDMI, Bluetooth","4K HDR, Smart TV","../main/images/sonybravia.jpg",4.5));

products.push_back(Products(1029,"Xbox Series S","Gaming",499.99,30,"Microsoft","Series S","Black","301 x 151 x 151 mm",4200,"Plastic/Metal","1 Year","2023-12-01","USA","NA",0,"Custom AMD CPU",0,0,"WiFi, HDMI, Bluetooth","4K Gaming, Backward Compatible","../main/images/xbox S.jpg",4.4));

products.push_back(Products(1030,"Adidas Ultraboost 23 Variant","Footwear",179.99,50,"Adidas","Ultraboost 23","White","NA",320,"Knit/Rubber","1 Year","2024-03-15","Vietnam","NA",0,"NA",0,0,"NA","Running Shoes, Boost Cushioning","../main/images/adidas ultra boost.jpg",4.5));

    return products;
}
