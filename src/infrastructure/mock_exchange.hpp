#pragma once
#include "../interfaces.hpp"
#include <thread>
#include <atomic>
#include <random>
#include <iostream>
#include <map>

namespace Quant {

    class MockExchange : public IMarketData, public IOrderExecutor {
    private:
        std::map<std::string, TickHandler> subscribers_;
        std::atomic<bool> running_{false};
        
        // Ledger state
        double usd_balance_ = 10000.0;
        std::map<std::string, double> positions_;

    public:
        void subscribe(const std::string& symbol, TickHandler handler) override {
            subscribers_[symbol] = handler;
        }

        void execute_order(const Order& order) override {
            std::cout << "[MOCK EXECUTION] " 
                      << (order.side == Side::BUY ? "BUY " : (order.side == Side::SELL ? "SELL " : "CLOSE ")) 
                      << order.quantity << " " << order.symbol << std::endl;
            
            // Simple PnL/Position accounting
            if (order.side == Side::BUY) {
                usd_balance_ -= order.quantity * 100.0; // Hardcoded fills @ 100.0
                positions_[order.symbol] += order.quantity;
            } else if (order.side == Side::SELL) {
                usd_balance_ += order.quantity * 100.0;
                positions_[order.symbol] -= order.quantity;
            } else {
                positions_[order.symbol] = 0; // Flatten
            }
        }

        double get_position(const std::string& symbol) override {
            return positions_[symbol];
        }

        double get_balance() override { return usd_balance_; }

        void start() override {
            running_ = true;
            std::thread loop([this]() {
                // PRNG setup
                std::default_random_engine generator;
                std::normal_distribution<double> distribution(0.0, 1.0);

                // Initial sims (BTC/ETH pair)
                double btc_price = 50000.0;
                double eth_price = 3000.0; 
                
                while (running_) {
                    // Update BTC with basic drift
                    btc_price += distribution(generator) * 10.0; 
                    
                    // Synthetic mean-reversion logic to force trade signals 
                    // ETH pulls back to 0.06 ratio vs BTC
                    double target_ratio = 0.06;
                    double current_ratio = eth_price / btc_price;
                    double reversion = (target_ratio - current_ratio) * 0.1; 
                    
                    eth_price += (reversion * btc_price) + (distribution(generator) * 5.0);

                    // Push ticks to engine
                    long long now = std::chrono::system_clock::now().time_since_epoch().count();
                    
                    if (subscribers_.count("BTC")) subscribers_["BTC"]({"BTC", btc_price, now});
                    if (subscribers_.count("ETH")) subscribers_["ETH"]({"ETH", eth_price, now});

                    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10Hz loop
                }
            });
            loop.detach();
        }
    };
}