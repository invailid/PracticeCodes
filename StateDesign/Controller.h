#ifndef CONTROLLER_H
#define CONTROLLER_H


class Delivery;

class Controller{
    private:
        static Controller *instance;
        Controller(){}
        // Controller(Controller const&);
        // void operator=(Controller const&);
    public:
        Controller( Controller const&)=delete;
        void operator=(Controller const&)=delete;
        static Controller *getInstance();
        void Launch();
        static void Display(Delivery *OrderPlacement);
};


#endif