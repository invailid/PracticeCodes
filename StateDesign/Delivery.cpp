#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>
#include "Delivery.h"
#include "States.h"
//#include "AllStates.h"

using namespace std;

ItemsList* ItemsList::instance = NULL;

Delivery::Delivery(State *state){
    curState = nullptr;
    this->TransitionTo(state);
    itemList = ItemsList::getInstance(); //show home page on startup
    //show_Home();
}

Delivery::~Delivery(){}

void Delivery::TransitionTo(State *state){
    if(curState != nullptr)
        delete curState;
    curState = state;
    curState->setContext(this);
}

void Delivery::show_Options(){  //call the display options of the currently active state
    curState->DisplayOptions();
}


void Delivery::Add_item(string name, int quantity, double price){
    Item *p = new Item(name, quantity, price);
    itemList->items.push_back(p);
}

void Delivery::Update_inventory(int quantity, int itemNum, double price){
    curState->Setter(quantity, itemNum, price);
    //curState->DisplayOptions();
    
}

void Delivery::Checkout(int opt, int qty){
    double price = itemList->items[opt-1]->getPrice();
    curState->Setter(qty, opt, price );

}

void Delivery::Make_payment(int itemNum, int quantity, double price){
    //here
    curState->Setter(quantity, itemNum, price);
    //curState->DisplayOptions(); //payment state (hopefully!)

}

bool Delivery::ExitPrg(){
    return wantsExit;
}
void Delivery::setExit(){
    wantsExit = true;
}


