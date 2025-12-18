#pragma once
#include <string>
#include <chrono>

namespace Quant {

    enum class Side { BUY, SELL, HOLD };

    struct Tick {
        std::string symbol;
        double price;
        long long timestamp; // Unix ms
    };

    struct Order {
        std::string symbol;
        Side side;
        double quantity;
        double price; // 0.0 = MKT
        std::string id;
    };

    struct StrategyConfig {
        std::string asset_a;
        std::string asset_b;
        size_t window_size;    // Lookback for calibration
        double z_trigger;      // Entry threshold
        double stop_loss_z;    // Hard exit threshold
        double risk_per_trade; // Position sizing (Kelly/Fixed)
    };
}