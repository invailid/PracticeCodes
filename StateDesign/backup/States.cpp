
#include<string>
#include <iostream>
#include <vector>
#include <typeinfo>
#include "States.h"
#include "Delivery.h"
//#include "AllStates.h"
using namespace std;
/*
virtual ~State();
        virtual void DisplayOptions();
        virtual void Execute();
        virtual void Setter();
        void setContext(Context *context);
*/
State::State(){}
State::~State(){ /*cout<<typeid(*(context->curState)).name()<<"state deleted"<<endl;*/}


void State::DisplayOptions(){
    cout<<"Invalid Transition from "/*<<typeid(*(context->curState)).name()*/<< "to DisplayOptions"<<endl;
}

void State::Execute(){
    cout<<"Invalid Transition from "/*<<typeid(*(context->curState)).name()*/<< "to Execute"<<endl;
}

void State::Setter(int quantity, int itemNum, double price){
    cout<<"Invalid Transition from "/*<<typeid(*(context->curState)).name()*/<< "to Setter"<<endl;
}



void State::setContext(Delivery *context){
    this->context = context;
    //cout<<"<context set  "/*<<typeid(*(context->curState)).name()*/<<endl;
}

/*STATES DECLARATION*/
// class Home_state:public State;
// class Browse_state:public State;
// class Checkout_state:public State;
// class Payment_state:public State;
// class Update_state:public State;

/*STATES DEFINITION*/

///HOME STATE

Home_state::~Home_state(){
    //cout<<"home state deleted"<<endl;
}
void Home_state::DisplayOptions(){
        cout<<"\n-----HOME-----\n\t 1. Shop Now\n\t 2. Add to Inventory\n\t 3. Exit"<<endl;
        
        Execute();     

}

void Home_state::Execute(){
            int opt;
            bool picked = false;
            while(!picked){
                cout<<"Enter your Choice : ";
                cin>>opt;
                switch(opt){
                    case 1:
                       { this->context->TransitionTo(new Browse_state); //
                        picked=true; return; }
                        break;
                    case 2:
                       { this->context->TransitionTo(new Update_state);
                        picked = true; return;}
                        break;
                    case 3:
                        {
                            context->setExit();
                            picked = true; return;
                        }
                        break;
                    default:
                        cout<<"Please select a valid option."<<endl;
                        break;
                }
            }
}

///BROWSE STATE


Browse_state::~Browse_state(){}
void Browse_state::DisplayOptions(){
            cout<<"\n-----BROWSE-----\n Items Available : "<<endl;
            //context->itemList->items;
            if(context->itemList->items.empty()){
                cout<<"\tInventory empty"<<endl;
                context->TransitionTo(new Home_state); //transitioning to home state
                return;
            }

            int counter=1;
            for(auto i:context->itemList->items){
                cout<<"\t"<<counter<<". ";
                i->DisplayAll();
                cout<<endl;
                counter++;
            }
            cout<<"\t"<<counter<<". Back"<<endl;
           Execute();            
}

void Browse_state::Execute(){
            int opt, qty;
            bool picked = false;
            while(!picked){
                cout<<"Enter item no. you wanna buy : ";
                cin>>opt;
                if(opt == context->itemList->items.size()+1)
                    {context->TransitionTo(new Home_state); picked = true; return;}

                else if(opt > context->itemList->items.size()){
                    cout<<"Please enter valid item id."<<endl;
                    continue;
                }
                cout<<"Enter quantity you wanna buy : ";
                cin>>qty;

                if(context->itemList->items[opt-1]->isAvailable(qty) && qty>0){
                    Delivery *tempo = context;
                    context->TransitionTo(new Checkout_state);
                    tempo->Checkout(opt, qty); //set option quantity of current state(checkout)
                    picked = true;
                    return;
                    
                    
                }
                else{
                    cout<<"<< Not enough stock available. Select valid quantity >>"<<endl;
                }
            }
}

///CHECKOUT STATE

void Checkout_state::DisplayOptions(){
            cout<<"-----CHECKOUT-----\nItem No. : "<<itemNum<<"\nQuantity : "<<quantity<<"\nTotal : "
            <<quantity*price<<endl;
            Execute();
}
Checkout_state::~Checkout_state(){}
void Checkout_state::Execute(){
            cout<<"\n---Confirm Order---"<<endl;
            cout<<"1. Make Payment\n2. Cancel";
            int opt;
            bool picked = false;
            while(!picked){
                cout<<"\nPick an option : "; cin>>opt;
                switch(opt){
                    case 1: {
                        Delivery *tempo = context;
                        context->TransitionTo(new Payment_state);
                        tempo->Make_payment(itemNum, quantity, price);
                        return;}
                        break;
                    case 2:
                        context->TransitionTo(new Browse_state);
                        return;
                        break;
                    default:
                        cout<<"please choose a valid option"<<endl;
                        break;
                }
            }
}

void Checkout_state::Setter(int quantity, int itemNum, double price){
            this->quantity = quantity;
            this->itemNum = itemNum;
            this->price = price;
}

///PAYMENT STATE

void Payment_state::DisplayOptions(){
            cout<<"Paying amount Rs."<<price*quantity<<" from your account.\n";
            cout<<"Paying..."<<endl;
            Execute();

}
Payment_state::~Payment_state(){}
void Payment_state::Execute(){
            cout<<"Rs. "<<quantity*price<<" has been deducted from your account.\n";
            Delivery *tempo = context;
            context->TransitionTo(new Update_state);
            tempo->Update_inventory(quantity, itemNum, price);
}

void Payment_state::Setter(int quantity, int itemNum, double price){
            this->quantity = quantity;
            this->itemNum = itemNum;
            this->price = price;
}

///UPDATE STATE
Update_state::Update_state(){
        this->itemNum =0;
}
void Update_state::DisplayOptions(){
             if(itemNum==0){
                int opt;
                do{
                    string name;
                    cout<<"\nEnter item Name : "; cin>>name;
                    cout<<"Enter item Quantity : "; cin>>quantity;
                    cout<<"Enter item Price : ";cin>>price;
                    context->Add_item(name, quantity, price);
                    cout<<"Item has been successfully added to Inventory\n";
                    
                    cout<<"Select an Option :\n\t1. Add More\n\t2. Back"<<endl;
                    cout<<"Enter your choice : "; cin>>opt;
                    while(opt!=1 && opt!=2){
                        cout<<"Enter a valid choice : "; cin>>opt;
                    }
                    
                }while(opt!=2);
                
                context->TransitionTo(new Home_state); //Transition to home state again
                
            }
            else
                Execute();
}

Update_state::~Update_state(){}

void Update_state::Execute(){
            int curQuantity = context->itemList->items[itemNum-1]->getQuantity();
            curQuantity-=quantity;
            if(curQuantity == 0){
                context->itemList->RemoveItem(itemNum);
            }
            else{
                context->itemList->items[itemNum-1]->UpdateQty(curQuantity);
            }
            cout<<"<Inventory Updated>"<<endl;
            context->TransitionTo(new Home_state);
}

void Update_state::Setter(int quantity, int itemNum, double price){
            this->quantity = quantity;
            this->itemNum = itemNum;
            this->price = price;
}



