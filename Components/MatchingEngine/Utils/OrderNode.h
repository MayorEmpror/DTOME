#pragma once 

class OrderNode{
    public:
        int orderid;
        int quantity;
        double price; 
        int remainingQ;

        OrderNode* prevnode;
        OrderNode* nextnode;

        OrderNde(int orderid, int quantity, int remq, double price ):
        orderid(orderid), quantity(quantity), remainingQ(remq), price(price){
            this->prevnode = nullptr;
            this->nextnode = nullptr;
        }
        
}