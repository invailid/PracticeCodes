#ifndef States_H
#define States_H
#include <string>
#include <iostream>
using namespace std;
class Delivery;

class State{
    protected:
        int quantity;
        int itemNum;
        double price;
        Delivery *context;

    public:
        State();
        virtual ~State(); //deleting derived class from base class pointer results in undefined behaviour if destructor is non  virtual
        virtual void DisplayOptions();
        virtual void Execute();
        virtual void Setter(int quantity, int itemNum, double price);
        // int getQuantity();
        // int getItemNo();
        // double getPrice();
        void setContext(Delivery *context);
};

class Home_state:public State{

    public:
       // Home_state();
        ~Home_state();
        void DisplayOptions();
        void Execute();

};

class Browse_state:public State{

    public:
        //Browse_state(); 
        ~Browse_state();
        void DisplayOptions();
        void Execute();

};

class Update_state:public State{

    public:
        Update_state();
        ~Update_state();
        void DisplayOptions();
        void Execute();
        void Setter(int quantity, int itemNum, double price);

};

class Checkout_state:public State{

    public:
        ~Checkout_state();
        void DisplayOptions();
        void Execute();
        void Setter(int quantity, int itemNum, double price);

};

class Payment_state:public State{

    public:
        ~Payment_state();
        void DisplayOptions();
        void Execute();
        void Setter(int quantity, int itemNum, double price);
};



#endif 
