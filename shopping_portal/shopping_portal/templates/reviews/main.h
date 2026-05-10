#ifndef REVIEWS_MAIN_H
#define REVIEWS_MAIN_H

#include <iostream>
using namespace std;

class Reviews{
    private:
    string author;
    string content;
    public: 
    Reviews(){
        this->author = " ";
        this->content = " ";
    }
    Reviews(string auth, string cont): content(cont), author(auth){};
    Reviews(const Reviews& other){
        this->author = other.author;
        this->content = other.content;
    }
    Reviews& operator =(const Reviews& other){
        if(this != &other){
            this->author = other.author;
            this->content = other.content;
        }
        return *this;
    }
    
    ~Reviews(){

    }

};

#endif