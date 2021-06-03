#include <bits/stdc++.h>
#include "Controller.h"
#include "Delivery.h"
#include "States.h"
#include <thread>
//#include "AllStates.h"
using namespace std;

// Controller::Controller( Controller const&)=delete;
// void Controller::operator=(Controller const&)=delete;

void Controller::Display(Delivery *OrderPlacement){
    OrderPlacement->show_Options();
}
void Controller::Launch(){
    cout<<"<---Launching Order Placement State Design Pattern System--->"<<endl;

    Delivery *OrderPlacement = new Delivery(new Home_state); //initially to home state
    while(!OrderPlacement->ExitPrg()){
       
        thread CurrentlyRunning(Display, ref(OrderPlacement));
        CurrentlyRunning.join();
    }
    delete OrderPlacement;
}

Controller* Controller::instance = NULL;

Controller* Controller::getInstance(){
    if(instance == NULL)
        instance = new Controller;
    return instance;
}