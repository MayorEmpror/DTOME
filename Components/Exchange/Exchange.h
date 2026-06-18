#pragma once 

#include <string>
#include <unordered_map.h>

#include "Market.h"
#include "Order.h"


class Exchange {
        private :
            int exh_id;
            std::string name;
            unordered_map<int, Market*> Markets;
        
        public:
            void AddMarket(Market* mkt);
            void routeOrder(Order* order);
            Market* getMarket(int mkt_id);

};