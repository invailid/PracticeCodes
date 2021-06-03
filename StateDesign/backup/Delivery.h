#ifndef Delivery_H
#define Delivery_H
#include <string>
#include <iostream>
#include <thread>
using namespace std;
class State;
class Item{
    private:
        string name;
        int quantity;
        double price;
    public:
        ~Item(){}

        Item(string name, int quantity, double price){
            this->name = name;
            this->quantity = quantity;
            this->price = price;
        }

        void DisplayAll(){
            cout<<"ITEM NAME : "<<name<<"\n\t   QUANTITY AVAILABLE : "<<quantity<<"\n\t   PRICE : "<<price<<endl;
        }

        void UpdateQty(int quantity){
            this->quantity= quantity;
        }
        double getPrice(){
            return price;
        }
        string getName(){
            return name;
        }
        int getQuantity(){
            return quantity;
        }
        bool isAvailable(int to_buy){
            if(to_buy <= quantity) return true;
            return false;
        }
};

class ItemsList{ //singleton
    private:
        static ItemsList *instance;
        ItemsList(){}
        // ItemsList(const ItemsList&);
        // void operator=(ItemsList const&);
        
    public:
        vector <Item*> items;

        ItemsList(const ItemsList&)=delete;
        void operator=(ItemsList const&)= delete;

        static ItemsList* getInstance(){
            if(instance==NULL)
                instance = new ItemsList;
            return instance;
        }
        
        void RemoveItem(int itemNum){
            delete items[itemNum-1];
            items.erase(items.begin()+itemNum-1);
        }

        void addToList(Item *item){
            items.push_back(item);
        }


};

class Delivery{
    protected:
        State *curState;
        bool wantsExit =false;
       
    public:
        
        ItemsList *itemList;
        Delivery(State *state);
        virtual ~Delivery();
        void show_Options();
       
        void Add_item(string name, int quantity, double price);
        void Update_inventory(int quantity, int itemNum, double price);
        void Checkout(int opt, int qty);
        void Make_payment(int itemNum, int quantity, double price);
        void TransitionTo(State *state);
        bool ExitPrg();
        void setExit();
};

#endif