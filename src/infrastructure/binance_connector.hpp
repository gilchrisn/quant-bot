#pragma once
#include "../interfaces.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <map>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace Quant {

    class BinanceConnector : public IMarketData, public IOrderExecutor {
    private:
        net::io_context ioc_;
        ssl::context ctx_{ssl::context::tlsv12_client};
        std::map<std::string, TickHandler> subscribers_;
        std::map<std::string, std::string> reverse_symbol_map_; // binance sym -> internal sym
        
        // Sim state for paper trading
        double paper_balance_usd_ = 10000.0;
        std::map<std::string, double> paper_positions_;

    public:
        BinanceConnector() {
            // Disable cert verification to avoid issues on WSL/local envs
            ctx_.set_verify_mode(ssl::verify_none); 
        }

        void subscribe(const std::string& symbol, TickHandler handler) override {
            std::string sub_symbol = symbol; 
            std::transform(sub_symbol.begin(), sub_symbol.end(), sub_symbol.begin(), ::tolower);
            if (sub_symbol.find("usdt") == std::string::npos) sub_symbol += "usdt";
            
            subscribers_[sub_symbol] = handler;
            reverse_symbol_map_[sub_symbol] = symbol; 
        }

        void start() override {
            std::thread t([this]() {
                run_websocket_loop();
            });
            t.detach();
        }

        void execute_order(const Order& order) override {
            std::cout << "\n[REAL-TIME PAPER TRADE] ------------------" << std::endl;
            std::cout << "TYPE: " << (order.side == Side::BUY ? "BUY" : (order.side == Side::SELL ? "SELL" : "CLOSE")) << std::endl;
            std::cout << "ASSET: " << order.symbol << std::endl;
            std::cout << "QTY: " << order.quantity << std::endl;
            std::cout << "------------------------------------------\n" << std::endl;

            if (order.side == Side::BUY) {
                 paper_positions_[order.symbol] += order.quantity;
            } else if (order.side == Side::SELL) {
                 paper_positions_[order.symbol] -= order.quantity;
            } else {
                 paper_positions_[order.symbol] = 0;
            }
        }

        double get_position(const std::string& symbol) override {
            return paper_positions_[symbol];
        }

        double get_balance() override {
            return paper_balance_usd_;
        }

    private:
        void run_websocket_loop() {
            try {
                tcp::resolver resolver(ioc_);
                auto const results = resolver.resolve("stream.binance.com", "9443");

                websocket::stream<beast::ssl_stream<tcp::socket>> ws(ioc_, ctx_);
                net::connect(beast::get_lowest_layer(ws), results);

                // Required for Binance SSL handshake
                if(! SSL_set_tlsext_host_name(ws.next_layer().native_handle(), "stream.binance.com")) {
                    throw beast::system_error(
                        beast::error_code(
                            static_cast<int>(::ERR_get_error()),
                            net::error::get_ssl_category()),
                        "SSL_set_tlsext_host_name");
                }

                ws.next_layer().handshake(ssl::stream_base::client);

                // Build multi-stream path
                std::string url = "/stream?streams=";
                bool first = true;
                for (const auto& pair : subscribers_) {
                    if (!first) url += "/";
                    url += pair.first + "@trade";
                    first = false;
                }

                std::cout << "[BINANCE] Handshake Success. Subscribing to: " << url << std::endl;
                
                ws.handshake("stream.binance.com:9443", url);

                beast::flat_buffer buffer;
                while(true) {
                    ws.read(buffer);
                    std::string payload = beast::buffers_to_string(buffer.data());
                    
                    auto j = json::parse(payload);
                    
                    if (j.contains("data")) {
                        auto data = j["data"];
                        std::string symbol = data["s"]; 
                        double price = std::stod(data["p"].get<std::string>());
                        long long time = data["T"];

                        std::string lower_sym = symbol;
                        std::transform(lower_sym.begin(), lower_sym.end(), lower_sym.begin(), ::tolower);
                        
                        // Data flow heartbeat
                        std::cout << "." << std::flush;

                        if (subscribers_.count(lower_sym)) {
                            std::string user_sym = reverse_symbol_map_[lower_sym];
                            subscribers_[lower_sym]({user_sym, price, time});
                        }
                    }
                    buffer.consume(buffer.size());
                }
            }
            catch (std::exception const& e) {
                std::cerr << "[BINANCE ERROR] " << e.what() << std::endl;
            }
        }
    };
}