Casual Quant Bot (C++)

A robust, SOLID-principled trading bot framework designed for Statistical Arbitrage (Pairs Trading) using the Ornstein-Uhlenbeck (OU) process.

🚀 Concept

This bot implements the math described in "Quantitative Market Microstructure."

Ingest Data: Listens to ETH and BTC prices.

OU Calibration: Calculates the Mean Reversion speed ($\theta$) and Equilibrium ($\mu$) of the ETH/BTC ratio in real-time.

Signal Generation: Calculates a Z-Score.

$Z > 2.0$: Short the Ratio (Short ETH, Long BTC).

$Z < -2.0$: Long the Ratio (Long ETH, Short BTC).

$Z \approx 0$: Close positions (Take Profit).

🛠 Prerequisites

CMake (3.10+)

C++ Compiler (C++17 support)

Eigen3 (Math library)

Boost (System/Asio - for networking)

nlohmann_json (JSON parsing)

Ubuntu/Debian Setup

sudo apt-get update
sudo apt-get install build-essential cmake libboost-all-dev libssl-dev nlohmann-json3-dev
# Eigen is usually header-only, but you can install via package manager:
# sudo apt-get install libeigen3-dev 
# OR download manually and place in /usr/local/include/eigen3


🏗 Building & Running

Create Build Directory:

mkdir build && cd build


Compile:

cmake ..
make


Run (Simulation Mode):

./CasualQuantBot


You will see output indicating the Z-score calculation and Simulated Trades:

[STRATEGY] Ratio: 0.0612 | Mean: 0.0600 | Z-Score: 2.15
>>> OPEN SHORT SPREAD (Short ETH, Long BTC)
[MOCK EXECUTION] SELL 0.1 ETH
[MOCK EXECUTION] BUY 0.00612 BTC


🧩 Architecture (SOLID)

IMarketData / IOrderExecutor: Interfaces that decouple the Strategy from the Exchange.

OUModel: Pure math component. Uses Eigen for Least Squares regression.

MockExchange: Generates synthetic Co-integrated data to prove the math works.

PairsTradingStrategy: The business logic.

⚠️ Risk Warning

This code currently uses a Mock Exchange. To trade real money:

Implement a BinanceConnector class inheriting from IMarketData and IOrderExecutor using Boost.Beast.

Add your API Keys securely (Environment Variables).

Use the "Cash and Carry" logic (simpler structural arb) if you want safer yields than Pairs Trading.