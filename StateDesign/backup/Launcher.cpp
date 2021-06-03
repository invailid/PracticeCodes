#include <iostream>
#include "Controller.h"


using namespace std;

int main(){
    Controller *controller = Controller::getInstance();
    controller->Launch();
}

// use smart pointers in other design patterns