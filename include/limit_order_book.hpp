// An order book for managing Limit / Order objects in a continuous double auction.
//
// Copyright (c) 2020 Christian Kauten
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef LOB_LIMIT_ORDER_BOOK_HPP_
#define LOB_LIMIT_ORDER_BOOK_HPP_

#include "limit_tree.hpp"
#include <unordered_map>
#include <tuple>
#include <string>

/// stupid hack
static int64_t _top_of_book[6];

static int64_t _depth_of_book[4*500];

/// Logic for maintaining a continuous double auction via a limit-order book.
namespace LOB {

/// a map of order IDs to orders
typedef std::unordered_map<UID, Order> UIDOrderMap;

/// @brief An order book for managing Limit / Order objects in a continuous double auction.
/// @details
/// \image html img/limit-order-book.svg "Abstract Domain Model of the Limit Order Book"
/// \image latex img/limit-order-book.pdf "Abstract Domain Model of the Limit Order Book"
///
class LimitOrderBook {
 private:
    /// the tree of sell order in the book.
    LimitTree<Side::Sell> sells;
    /// the tree of buy orders in the book.
    LimitTree<Side::Buy> buys;
    /// a mapping of order IDs to orders (for cancellation).
    UIDOrderMap orders;

 public:
    /// @brief Initialize a new limit order book object.
    LimitOrderBook() : sells(), buys(), orders() { }

    /// @brief Clear all the orders in the book.
    inline void clear() {
        sells.clear();
        buys.clear();
        orders.clear();
    }

    /// @brief Add a new sell limit order to the book.
    ///
    /// @param order_id the ID for the order
    /// @param quantity the number of shares to sell
    /// @param price the limit price for the order
    ///
    inline void limit_sell(UID order_id, Quantity quantity, Price price) {
        // put the order into the map
        orders.emplace(std::piecewise_construct,
            std::forward_as_tuple(order_id),
            std::forward_as_tuple(order_id, Side::Sell, quantity, price)
        );
        if (buys.best != nullptr && price <= buys.best->key) {  // crosses
            // place a market order with the limit price
            buys.market(&orders.at(order_id), [&](UID uid) { orders.erase(uid); });
            if (orders.at(order_id).quantity == 0) {  // order filled
                orders.erase(order_id);
                return;
            }
        }
        sells.limit(&orders.at(order_id));
    }

    /// @brief Add a new buy limit order to the book.
    ///
    /// @param order_id the ID for the order
    /// @param quantity the number of shares to buy
    /// @param price the limit price for the order
    ///
    inline void limit_buy(UID order_id, Quantity quantity, Price price) {
        // put the order into the map
        orders.emplace(std::piecewise_construct,
            std::forward_as_tuple(order_id),
            std::forward_as_tuple(order_id, Side::Buy, quantity, price)
        );
        if (sells.best != nullptr && price >= sells.best->key) {  // crosses
            // place a market order with the limit price
            sells.market(&orders.at(order_id), [&](UID uid) { orders.erase(uid); });
            if (orders.at(order_id).quantity == 0) {  // order filled
                orders.erase(order_id);
                return;
            }
        }
        buys.limit(&orders.at(order_id));
    }

    /// @brief Add a new order to the book.
    ///
    /// @param side whether the order is a buy (true) or sell (false)
    /// @param order_id the ID for the order
    /// @param quantity the number of shares to buy
    /// @param price the limit/market price for the order
    ///
    inline void limit(Side side, UID order_id, Quantity quantity, Price price) {
        switch (side) {  // send the order to the appropriate side
            case Side::Sell: return limit_sell(order_id, quantity, price);
            case Side::Buy:  return limit_buy(order_id, quantity, price);
        }
    }

    /// @brief Return true if the book has an order with given ID, false otherwise.
    ///
    /// @param order_id the order ID of the order to check for existence of
    /// @returns true if the book has an order with given ID, false otherwise
    ///
    inline bool has(UID order_id) const { return orders.count(order_id); }

    /// @brief Get the order with given ID.
    ///
    /// @param order_id the order ID of the order to get
    /// @returns a pointer to the order with given order ID
    ///
    inline const Order& get(UID order_id) { return orders.at(order_id); }

    /// @brief Get the order quantity with given ID.
    ///
    /// @param order_id the order ID of the order to get
    /// @returns quantity of the order with given order ID
    ///
    inline const Quantity get_quantity(UID order_id) {
        auto order = &orders.at(order_id);
        return order->quantity;
    }

    /// @brief Get the order side with given ID.
    ///
    /// @param order_id the order ID of the order to get
    /// @returns side of the order with given order ID
    ///
    inline const Side get_side(UID order_id) {

        auto it = orders.find(order_id);
        
        if (it == orders.end())
            return Side::Sell;
        
        return it->second.side;
    }

    /// @brief Get the order side with given ID.
    ///
    /// @param order_id the order ID of the order to get
    /// @returns side of the order with given order ID
    ///
    inline const Price get_price(UID order_id) {
        auto it = orders.find(order_id);

        if (it == orders.end()) 
            return 0;
        
        return it->second.price;
    }

    /// @brief Cancel an existing order in the book.
    ///
    /// @param order_id the ID of the order to cancel
    ///
    inline int cancel(UID order_id) {

//        if (orders.find(order_id) == orders.end()) // safe cancel, but speed tradeoff?
//            return false;
//
//        auto order = &orders.at(order_id);

        auto it = orders.find(order_id);
        if (it == orders.end())
            return -1;

        auto order = &it->second;

        // find queue position of order
        auto limit = order->limit;
        DLL::Node* temp = (DLL::Node* )limit->order_tail;
        int position = 0;

        if (temp == nullptr) {
            position = -1;
        } else
            while (temp != order && temp != nullptr) {
                temp = temp->prev;
                position++;
            }

        switch (order->side) {  // remove the order from the appropriate side
            case Side::Sell: { sells.cancel(order); break; }
            case Side::Buy:  { buys.cancel(order); break; }
        }
        orders.erase(order_id);

        return position;

    }

    /// @brief Reduce the quantity of the order with given ID.
    ///
    /// @param order_id the id of the order to reduce the quantity of
    /// @param quantity the amount to remove from the order
    ///
    inline void reduce(UID order_id, Quantity quantity) {
        auto order = &orders.at(order_id);
        if (quantity > order->quantity) {  // trying to remove more than available
            throw "trying to remove " +
                std::to_string(quantity) +
                " from order with " +
                std::to_string(order->quantity) +
                " available!";
        }
        // remove the quantity from the order and limit
        order->quantity -= quantity;
        order->limit->volume -= quantity;
        switch (order->side) {  // remove the order from the appropriate side
            case Side::Sell: { sells.volume -= quantity; break; }
            case Side::Buy:  { buys.volume -= quantity; break; }
        }
        if (order->quantity == 0) {  // remove the order if there is no quantity left
            switch (order->side) {  // remove the order from the appropriate side
                case Side::Sell: { sells.cancel(order); break; }
                case Side::Buy:  { buys.cancel(order); break; }
            }
            orders.erase(order_id);
        }
    }

    /// @brief Modify an existing order in the book.
    ///
    /// @param order_id the ID of the order to modify
    /// @param side of the order to modify
    /// @param quantity of the order to modify
    /// @param price of the order to modify
    ///

    inline int64_t modify(UID order_id, Side side, Quantity quantity, Price price) {

//        if (orders.find(order_id) == orders.end()) // safe modify, but speed tradeoff?
//            return 0;
//
//        auto order = &orders.at(order_id);

        auto it = orders.find(order_id);
        if (it == orders.end())
            return 0;

        auto order = &it->second;

        Price orderprice = order->price;

        if (order->side != side || orderprice != price || quantity > order->quantity) // cancel and reinstate
        {
            cancel(order_id);
            limit(side, order_id, quantity, price);

            return (int64_t)((int64_t)price - (int64_t)orderprice);

        } else if (quantity < order->quantity)
        {
            reduce(order_id, order->quantity-quantity);
            return 0;
        }

        return 0;
    }

    /// @brief Execute a sell market order.
    ///
    /// @param order_id the ID for the order
    /// @param quantity the quantity in the market order
    ///
    inline void market_sell(UID order_id, Quantity quantity) {
        Order order{order_id, Side::Sell, quantity, 0};
        buys.market(&order, [&](UID uid) { orders.erase(uid); });
    }

    /// @brief Execute a buy market order.
    ///
    /// @param order_id the ID for the order
    /// @param quantity the quantity of the market order
    ///
    inline void market_buy(UID order_id, Quantity quantity) {
        Order order{order_id, Side::Buy, quantity, 0};
        sells.market(&order, [&](UID uid) { orders.erase(uid); });
    }

    /// @brief Execute a market order.
    ///
    /// @param side whether the order is a sell or buy order
    /// @param order_id the ID for the order
    /// @param quantity the quantity of the market order
    ///
    inline void market(Side side, UID order_id, Quantity quantity) {
        switch (side) {  // send the market order to the appropriate side
            case Side::Sell: { market_sell(order_id, quantity); break; }
            case Side::Buy:  { market_buy(order_id, quantity); break; }
        }
    }

    /// @brief Return the best sell price.
    ///
    /// @returns the best ask price in the book
    ///
    inline Price best_sell() const {
        if (sells.best == nullptr)
            return 0;
        return sells.best->key;
    }

    /// @brief Return the best buy price.
    ///
    /// @returns the best bid price in the book
    ///
    inline Price best_buy() const {
        if (buys.best == nullptr)
            return 0;
        return buys.best->key;
    }

    /// @brief Return the best price for the given side.
    ///
    /// @param side the side to get the best price from
    /// @returns the best price on the given side of the book
    ///
    inline Price best(Side side) const {
        switch (side) {
            case Side::Sell: { return best_sell(); }
            case Side::Buy:  { return best_buy();  }
        }
    }

    /// @brief Return the current price of the asset.
    ///
    /// @returns the best price in the book (the midpoint price)
    ///
    inline Price price() const {
        if (sells.best == nullptr && buys.best == nullptr)  // empty book
            return 0;
        if (sells.best == nullptr)  // no sell orders
            return buys.best->key;
        if (buys.best == nullptr)  // no buy orders
            return sells.best->key;
        // return the midpoint
        return (sells.best->key + buys.best->key) / 2;
    }

    /// @brief Return the last best sell price.
    ///
    /// @returns the best bid price in the book
    ///
    inline Price last_best_sell() const { return sells.last_best_price; }

    /// @brief Return the last best buy price.
    ///
    /// @returns the best bid price in the book
    ///
    inline Price last_best_buy() const { return buys.last_best_price; }

    /// @brief Return the best price for the given side.
    ///
    /// @param side the side to get the best price from
    /// @returns the best price on the given side of the book
    ///
    inline Price last_best(Side side) const {
        switch (side) {
            case Side::Sell: { return last_best_sell(); }
            case Side::Buy:  { return last_best_buy();  }
        }
    }

    /// @brief Return the current price of the asset using last prices.
    ///
    /// @returns the last midpoint price
    ///
    inline Price last_price() const {
        return (sells.last_best_price + buys.last_best_price) / 2;
    }

    /// @brief Return the total volume for the sell side of the book.
    ///
    /// @param price the limit price to get the volume for
    /// @returns the volume for the given limit price
    ///
    inline Volume volume_sell(Price price) const {
        return sells.volume_at(price);
    }

    /// @brief Return the total volume for the sell side of the book.
    ///
    /// @returns the total volume on the sell side of the book
    ///
    inline Volume volume_sell() const { return sells.volume; }

    /// @brief Return the volume at the best sell price.
    ///
    /// @returns the volume of sell orders at the best price
    ///
    inline Volume volume_sell_best() const {
        if (sells.best == nullptr) return 0;
        return sells.best->volume;
    }

    /// @brief Return the total volume for the buy side of the book.
    ///
    /// @param price the limit price to get the volume for
    /// @returns the volume for the given limit price
    ///
    inline Volume volume_buy(Price price) const {
        return buys.volume_at(price);
    }

    /// @brief Return the total volume for the buy side of the book.
    ///
    /// @returns the total volume on the buy side of the book
    ///
    inline Volume volume_buy() const { return buys.volume; }

    /// @brief Return the volume at the best buy price.
    ///
    /// @returns the volume of buy orders at the best price
    ///
    inline Volume volume_buy_best() const {
        if (buys.best == nullptr) return 0;
        return buys.best->volume;
    }

    /// @brief Return the count at the best buy price.
    ///
    /// @returns the count of buy orders at the best price
    ///
    inline Quantity count_buy_best() const {
        if (buys.best == nullptr) return 0;
        return buys.best->count;
    }
    
    /// @brief Return the count at the best sell price.
    ///
    /// @returns the count of sell orders at the best price
    ///
    inline Quantity count_sell_best() const {
        if (sells.best == nullptr) return 0;
        return sells.best->count;
    }


    inline int64_t *get_last_top_of_book() {
        LOB::Limit* best_bid = buys.best;
        if (best_bid == nullptr)
        {
            _top_of_book[0] = 0;
            _top_of_book[1] = 0;
            _top_of_book[2] = 0;
        } else
        {
            _top_of_book[0] = best_bid->key;
            _top_of_book[1] = best_bid->volume;
            _top_of_book[2] = best_bid->count;
        }

        LOB::Limit* best_ask = sells.best;
        if (best_ask == nullptr)
        {
            _top_of_book[3] = 0;
            _top_of_book[4] = 0;
            _top_of_book[5] = 0;
        }
        else
        {
            _top_of_book[3] = best_ask->key;
            _top_of_book[4] = best_ask->volume;
            _top_of_book[5] = best_ask->count;
        }
        return _top_of_book;
    }


//    inline uint64_t *get_depth_of_book(uint64_t step, uint64_t range) {
//        LOB::Limit* best_bid_limit = buys.best;
//        LOB::Limit* best_ask_limit = sells.best;
//
//        if (best_bid_limit == nullptr || best_ask_limit == nullptr)
//        {
//            return nullptr;
//        }
//
//        uint64_t best_bid = best_bid_limit->key;
//        uint64_t best_ask = best_ask_limit->key;
//        uint64_t curr_bid = best_bid;
//        uint64_t curr_ask = best_ask;
//
//        uint64_t i = 0;
//        while (curr_bid > best_bid - range)
//        {
//
//            curr_bid -= step;
//            curr_ask += step;
//
//            /*printf("i: %lld\n", (long long)i);
//            printf("range: %lld\n", (long long)range);
//            printf("best_bid: %lld\n", (long long)best_bid);
//            printf("best_ask: %lld\n", (long long)best_ask);
//            printf("step: %lld\n", (long long)step);
//            printf("curr_bid: %lld\n", (long long)curr_bid);
//            printf("curr_ask: %lld\n", (long long)curr_ask);*/
//
//            LOB::Limit* bid_limits = buys.limit_at(curr_bid);
//            LOB::Limit* ask_limits = sells.limit_at(curr_ask);
//
//            _depth_of_book[i] = curr_bid;
//            if (bid_limits == nullptr)
//            {
//                _depth_of_book[i+1] = 0;
//                _depth_of_book[i+2] = 0;
//            }
//            else
//            {
//                _depth_of_book[i+1] = bid_limits->volume;
//                _depth_of_book[i+2] = bid_limits->count;
//            }
//
//            _depth_of_book[i+3] = curr_ask;
//            if (ask_limits == nullptr)
//            {
//                _depth_of_book[i+4] = 0;
//                _depth_of_book[i+5] = 0;
//            }
//            else
//            {
//                _depth_of_book[i+4] = ask_limits->volume;
//                _depth_of_book[i+5] = ask_limits->count;
//            }
//
//            i += 6;
//        }
//        return _depth_of_book;
//    }

    inline int64_t *get_depth_of_book(uint64_t step, uint64_t range) {
        LOB::Limit* best_bid_limit = buys.best;
        LOB::Limit* best_ask_limit = sells.best;

        if (best_bid_limit == nullptr || best_ask_limit == nullptr)
        {
            return nullptr;
        }

        uint64_t best_bid = best_bid_limit->key;
        uint64_t best_ask = best_ask_limit->key;
        uint64_t curr_bid = best_bid;
        uint64_t curr_ask = best_ask;

        uint64_t i = 0;
        while (curr_bid > best_bid - range)
        {

            curr_bid -= step;
            curr_ask += step;

            /*printf("i: %lld\n", (long long)i);
            printf("range: %lld\n", (long long)range);
            printf("best_bid: %lld\n", (long long)best_bid);
            printf("best_ask: %lld\n", (long long)best_ask);
            printf("step: %lld\n", (long long)step);
            printf("curr_bid: %lld\n", (long long)curr_bid);
            printf("curr_ask: %lld\n", (long long)curr_ask);*/

            LOB::Limit* bid_limits = buys.limit_at(curr_bid);
            LOB::Limit* ask_limits = sells.limit_at(curr_ask);

            //_depth_of_book[i] = curr_bid;
            if (bid_limits == nullptr)
            {
                _depth_of_book[i] = 0;
                _depth_of_book[i+1] = 0;
            }
            else
            {
                _depth_of_book[i] = bid_limits->volume;
                _depth_of_book[i+1] = bid_limits->count;
            }

            //_depth_of_book[i+3] = curr_ask;
            if (ask_limits == nullptr)
            {
                _depth_of_book[i+2] = 0;
                _depth_of_book[i+3] = 0;
            }
            else
            {
                _depth_of_book[i+2] = ask_limits->volume;
                _depth_of_book[i+3] = ask_limits->count;
            }

            i += 4;
        }
        return _depth_of_book;
    }
    /// @brief Return the volume at the given limit price.
    ///
    /// @param price the limit price to get the volume for
    /// @returns the volume for the given limit price
    ///
    inline Volume volume(Price price) const {
        return buys.volume_at(price) + sells.volume_at(price);
    }

    /// @brief Return the total volume for the book.
    ///
    /// @returns the total volume of orders in the book (for both sides)
    ///
    inline Volume volume() const { return sells.volume + buys.volume; }

    /// @brief Return the total number of orders at the given limit price.
    ///
    /// @param price the limit price to get the volume for
    /// @returns the volume for the given limit price
    ///
    inline Count count_at(Price price) const {
        return buys.count_at(price) + sells.count_at(price);
    }

    /// @brief Return the total number of orders on the sell side of the book.
    ///
    /// @returns the total count of orders on the sell side of the book
    ///
    inline Count count_sell() const { return sells.count; }

    /// @brief Return the total number of orders on the buy side of the book.
    ///
    /// @returns the total count of orders on the buy side of the book
    ///
    inline Count count_buy() const { return buys.count; }

    /// @brief Return the total number of orders in the book.
    ///
    /// @returns the total count of orders in the book
    ///
    inline Count count() const { return sells.count + buys.count; }
};

}  // namespace LOB

#endif  // LOB_LIMIT_ORDER_BOOK_HPP_
