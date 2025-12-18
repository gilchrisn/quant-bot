#pragma once
#include "types.hpp"
#include <functional>

namespace Quant {

    using TickHandler = std::function<void(const Tick&)>;

    class IMarketData {
    public:
        virtual ~IMarketData() = default;
        virtual void subscribe(const std::string& symbol, TickHandler handler) = 0;
        virtual void start() = 0; 
    };

    class IOrderExecutor {
    public:
        virtual ~IOrderExecutor() = default;
        virtual void execute_order(const Order& order) = 0;
        virtual double get_position(const std::string& symbol) = 0;
        virtual double get_balance() = 0;
    };

    class IStrategy {
    public:
        virtual ~IStrategy() = default;
        virtual void on_tick(const Tick& tick) = 0;
    };
}