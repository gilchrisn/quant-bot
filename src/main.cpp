#include <iostream>
#include <thread>
#include <chrono>
#include "types.hpp"
#include "infrastructure/mock_exchange.hpp"
#include "infrastructure/binance_connector.hpp"
#include "strategies/pairs_trading.hpp"

using namespace Quant;

int main() {
    std::cout << "=== Quant Bot (Paper Trading) ===" << std::endl;
    std::cout << "Target: Binance Live Feed" << std::endl;

    // Strategy parameters
    StrategyConfig config;
    config.asset_a = "ETH"; 
    config.asset_b = "BTC"; 
    
    // Window: 300 ticks to stabilize OU calibration against noise
    config.window_size = 300;  

    // Entry: 3.0 Z-Score to ensure spread exceeds transaction costs
    config.z_trigger = 3.0;   
    
    config.stop_loss_z = 5.0; 
    config.risk_per_trade = 0.1;

    // Infrastructure setup
    // MockExchange exchange; 
    BinanceConnector exchange;

    PairsTradingStrategy strategy(exchange, config);

    // Wire-up subscribers
    exchange.subscribe("ETH", [&](const Tick& t) { strategy.on_tick(t); });
    exchange.subscribe("BTC", [&](const Tick& t) { strategy.on_tick(t); });

    // Execution loop
    exchange.start();

    // Heartbeat loop
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}