#pragma once
#include "../interfaces.hpp"
#include "../math/ou_model.hpp"
#include <iostream>
#include <fstream>
#include <map>
#include <iomanip>

namespace Quant {

    class PairsTradingStrategy : public IStrategy {
    private:
        IOrderExecutor& executor_;
        StrategyConfig config_;
        OUModel model_;
        
        // Internal state
        double price_a_ = 0.0;
        double price_b_ = 0.0;
        bool in_position_ = false;
        Side current_side_ = Side::HOLD;

        std::ofstream log_file_;

    public:
        PairsTradingStrategy(IOrderExecutor& executor, StrategyConfig config)
            : executor_(executor), config_(config), model_(config.window_size) {
            
            log_file_.open("trade_log.csv");
            log_file_ << "Timestamp,PriceA,PriceB,Ratio,Mu,Theta,ZScore,Action\n";
        }

        ~PairsTradingStrategy() {
            if (log_file_.is_open()) log_file_.close();
        }

        void on_tick(const Tick& tick) override {
            // Sync price state
            if (tick.symbol == config_.asset_a) price_a_ = tick.price;
            else if (tick.symbol == config_.asset_b) price_b_ = tick.price;

            // Guard: need prices for both legs
            if (price_a_ == 0 || price_b_ == 0) return;

            // Synthetic spread ratio
            double ratio = price_a_ / price_b_;

            model_.update_price(ratio);

            double z_score = model_.get_z_score(ratio);
            
            // Console status line
            if (model_.is_ready()) {
                std::cout << "[STRATEGY] Ratio: " << std::fixed << std::setprecision(5) << ratio 
                          << " | Mean: " << model_.get_mu() 
                          << " | Z: " << std::setprecision(2) << z_score << "\r" << std::flush;
            }

            check_signals(z_score, ratio, tick.timestamp);
        }

    private:
        void check_signals(double z, double ratio, long long timestamp) {
            std::string action = "HOLD";

            // Entry: Short Leg A, Long Leg B (Ratio > Upper Bound)
            if (!in_position_ && z > config_.z_trigger && model_.is_ready()) {
                std::cout << "\n>>> OPEN SHORT SPREAD (Short " << config_.asset_a << ", Long " << config_.asset_b << ")" << std::endl;
                
                double qty = config_.risk_per_trade; 
                executor_.execute_order({config_.asset_a, Side::SELL, qty, 0, "1"});
                executor_.execute_order({config_.asset_b, Side::BUY, qty * ratio, 0, "2"});
                
                in_position_ = true;
                current_side_ = Side::SELL;
                action = "OPEN_SHORT";
            }
            // Entry: Long Leg A, Short Leg B (Ratio < Lower Bound)
            else if (!in_position_ && z < -config_.z_trigger && model_.is_ready()) {
                std::cout << "\n>>> OPEN LONG SPREAD (Long " << config_.asset_a << ", Short " << config_.asset_b << ")" << std::endl;
                
                double qty = config_.risk_per_trade;
                executor_.execute_order({config_.asset_a, Side::BUY, qty, 0, "3"});
                executor_.execute_order({config_.asset_b, Side::SELL, qty * ratio, 0, "4"});
                
                in_position_ = true;
                current_side_ = Side::BUY;
                action = "OPEN_LONG";
            }
            // Exit logic: Mean-crossing or hard Stop-Loss
            else if (in_position_ && model_.is_ready()) {
                bool should_close = false;
                
                if (current_side_ == Side::SELL && z <= 0) should_close = true;
                if (current_side_ == Side::BUY && z >= 0) should_close = true;
                if (std::abs(z) > config_.stop_loss_z) {
                     std::cout << "\n!!! STOP LOSS TRIGGERED !!!" << std::endl;
                     should_close = true;
                     action = "STOP_LOSS";
                }

                if (should_close && action == "HOLD") {
                    std::cout << "\n>>> CLOSING POSITIONS (Mean Reversion Achieved)" << std::endl;
                    executor_.execute_order({config_.asset_a, Side::HOLD, 0, 0, "CLOSE"}); 
                    executor_.execute_order({config_.asset_b, Side::HOLD, 0, 0, "CLOSE"});
                    in_position_ = false;
                    action = "CLOSE";
                }
            }

            // Persistence
            if (log_file_.is_open()) {
                log_file_ << timestamp << "," 
                          << price_a_ << "," 
                          << price_b_ << "," 
                          << ratio << "," 
                          << model_.get_mu() << "," 
                          << model_.get_theta() << "," 
                          << z << "," 
                          << action << "\n";
            }
        }
    };
}