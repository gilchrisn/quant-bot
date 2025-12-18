#pragma once
#include <vector>
#include <cmath>
#include <numeric>
#include <Eigen/Dense>
#include <iostream>

namespace Quant {

    class OUModel {
    private:
        size_t window_size_;
        std::vector<double> history_;
        
        // Ornstein-Uhlenbeck params
        double theta_ = 0.0; // speed
        double mu_ = 0.0;    // level
        double sigma_ = 0.0; // vol
        bool is_calibrated_ = false;

    public:
        OUModel(size_t window_size) : window_size_(window_size) {
            history_.reserve(window_size);
        }

        void update_price(double price) {
            history_.push_back(price);
            if (history_.size() > window_size_) {
                history_.erase(history_.begin());
                calibrate();
            }
        }

        // MLE via AR(1) regression: X_t = alpha + beta * X_{t-1} + eps
        void calibrate() {
            if (history_.size() < window_size_) return;

            size_t n = history_.size() - 1;
            Eigen::VectorXd Y(n); 
            Eigen::VectorXd X(n); 
            Eigen::VectorXd Ones = Eigen::VectorXd::Ones(n);

            for (size_t i = 0; i < n; ++i) {
                Y(i) = history_[i + 1];
                X(i) = history_[i];
            }

            // A = [1, X_{t-1}]
            Eigen::MatrixXd A(n, 2);
            A.col(0) = Ones;
            A.col(1) = X;

            // Solve (A^T A)^-1 A^T Y
            Eigen::VectorXd params = (A.transpose() * A).ldlt().solve(A.transpose() * Y);

            double alpha = params(0);
            double beta = params(1);

            // Compute residuals for sigma estimation
            Eigen::VectorXd residuals = Y - (A * params);
            double variance = residuals.squaredNorm() / (n - 2);

            // Map discrete AR(1) to continuous OU (dt=1)
            // theta = -ln(beta), mu = alpha/(1-beta)
            
            // Stationary check
            if (beta <= 0 || beta >= 1) {
                is_calibrated_ = false;
                return; 
            }

            theta_ = -std::log(beta); 
            mu_ = alpha / (1.0 - beta);
            sigma_ = std::sqrt(variance * 2 * theta_ / (1 - beta * beta));
            
            is_calibrated_ = true;
        }

        // Distance from mean relative to stationary std dev
        double get_z_score(double current_price) const {
            if (!is_calibrated_ || sigma_ == 0) return 0.0;
            return (current_price - mu_) / std::sqrt(sigma_ * sigma_ / (2 * theta_));
        }

        // Betting fraction heuristic
        double get_kelly_fraction() const {
             // TODO: implement full Kelly based on expected return/convergence
             return 0.5; // Half-Kelly
        }

        bool is_ready() const { return is_calibrated_; }
        
        double get_mu() const { return mu_; }
        double get_theta() const { return theta_; }
    };
}