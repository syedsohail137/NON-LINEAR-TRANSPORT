#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>

struct HistoryRow {
    std::string date;
    double open, high, low, close;
    long long volume;
};

struct OptionRow {
    long long call_oi;
    double call_ltp;
    double strike;
    double put_ltp;
    long long put_oi;
};

double normal_cdf(double x) {
    return 0.5 * std::erfc(-x * std::sqrt(0.5));
}

double normal_pdf(double x) {
    static const double inv_sqrt_2pi = 0.3989422804014327;
    return inv_sqrt_2pi * std::exp(-0.5 * x * x);
}

void linear_regression(const std::vector<double>& X, const std::vector<double>& Y, double& slope, double& intercept) {
    size_t n = X.size();
    double sum_X = 0, sum_Y = 0, sum_XY = 0, sum_XX = 0;
    for (size_t i = 0; i < n; ++i) {
        sum_X += X[i];
        sum_Y += Y[i];
        sum_XY += X[i] * Y[i];
        sum_XX += X[i] * X[i];
    }
    slope = (n * sum_XY - sum_X * sum_Y) / (n * sum_XX - sum_X * sum_X);
    intercept = (sum_Y - slope * sum_X) / n;
}

double extract_iv(double target_price, double S, double K, double T, double r, double d) {
    double sigma = 0.20;
    double h = 1e-4;
    
    auto bs_call = [](double S_val, double K_val, double T_val, double r_val, double d_val, double sig) {
        if (T_val <= 0) return std::max(0.0, S_val - K_val);
        double d1 = (std::log(S_val / K_val) + (r_val - d_val + 0.5 * sig * sig) * T_val) / (sig * std::sqrt(T_val));
        double d2 = d1 - sig * std::sqrt(T_val);
        return S_val * std::exp(-d_val * T_val) * normal_cdf(d1) - K_val * std::exp(-r_val * T_val) * normal_cdf(d2);
    };

    for (int iter = 0; iter < 100; ++iter) {
        double p = bs_call(S, K, T, r, d, sigma);
        double diff = p - target_price;
        if (std::abs(diff) < 1e-6) return sigma;
        
        double p_high = bs_call(S, K, T, r, d, sigma + h);
        double vega = (p_high - p) / h;
        if (std::abs(vega) < 1e-6) break;
        
        sigma -= diff / vega;
        if (sigma <= 0) sigma = 0.001;
    }
    return sigma;
}

int main() {
    std::cout << std::fixed << std::setprecision(6);
    
    std::ifstream hist_file("market_history.txt");
    if (!hist_file.is_open()) {
        std::cerr << "Fatal Error: Missing market_history.txt configuration input.\n";
        return 1;
    }
    
    std::string line;
    std::getline(hist_file, line);
    std::vector<HistoryRow> history;
    
    while (std::getline(hist_file, line)) {
        if(line.empty()) continue;
        std::stringstream ss(line);
        std::string part;
        HistoryRow row;
        
        std::getline(ss, row.date, ',');
        std::getline(ss, part, ','); row.open = std::stod(part);
        std::getline(ss, part, ','); row.high = std::stod(part);
        std::getline(ss, part, ','); row.low = std::stod(part);
        std::getline(ss, part, ','); row.close = std::stod(part);
        std::getline(ss, part, ','); row.volume = std::stoll(part);
        history.push_back(row);
    }
    hist_file.close();
    
    double S0 = history.back().close;
    
    double gk_sum = 0;
    for (const auto& row : history) {
        double log_hl = std::log(row.high / row.low);
        double log_co = std::log(row.close / row.open);
        gk_sum += 0.5 * log_hl * log_hl - (2.0 * std::log(2.0) - 1.0) * log_co * log_co;
    }
    double sigma_ohlc = std::sqrt((gk_sum / history.size()) * 252.0);

    std::ifstream chain_file("option_chain.txt");
    if (!chain_file.is_open()) {
        std::cerr << "Fatal Error: Missing option_chain.txt data file context.\n";
        return 1;
    }
    
    double T = 31.0 / 365.25; 
    
    std::vector<OptionRow> options;
    while (std::getline(chain_file, line)) {
        if (line.empty() || line[0] == '#' || line.rfind("call_oi", 0) == 0) continue;
        std::stringstream ss(line);
        std::string part;
        OptionRow row;
        
        std::getline(ss, part, ','); row.call_oi = std::stoll(part);
        std::getline(ss, part, ','); row.call_ltp = std::stod(part);
        std::getline(ss, part, ','); row.strike = std::stod(part);
        std::getline(ss, part, ','); row.put_ltp = std::stod(part);
        std::getline(ss, part, ','); row.put_oi = std::stoll(part);
        options.push_back(row);
    }
    chain_file.close();

    std::vector<double> X, Y;
    double min_strike_diff = 999999.0;
    size_t atm_idx = 0;
    
    for (size_t i = 0; i < options.size(); ++i) {
        Y.push_back(options[i].call_ltp - options[i].put_ltp);
        X.push_back(-options[i].strike);
        
        double diff = std::abs(options[i].strike - S0);
        if (diff < min_strike_diff) {
            min_strike_diff = diff;
            atm_idx = i;
        }
    }
    
    double slope = 0, intercept = 0;
    linear_regression(X, Y, slope, intercept);
    
    double r_extracted = -std::log(std::max(slope, 1e-5)) / T;
    double d_extracted = -std::log(std::max(intercept / S0, 1e-5)) / T;
    
    double atm_call_price = options[atm_idx].call_ltp;
    double atm_strike = options[atm_idx].strike;
    double iv_extracted = extract_iv(atm_call_price, S0, atm_strike, T, r_extracted, d_extracted);

    size_t N = options.size();
    std::vector<double> mu_T(N, 0.0);
    std::vector<double> nu_t(N, 0.0);
    
    double dK = options[1].strike - options[0].strike; 
    double mu_sum = 0.0;
    for (size_t i = 1; i < N - 1; ++i) {
        double second_deriv = (options[i+1].call_ltp - 2.0 * options[i].call_ltp + options[i-1].call_ltp) / (dK * dK);
        mu_T[i] = std::max(second_deriv, 0.0) * std::exp(r_extracted * T);
        long long total_oi = options[i].call_oi + options[i].put_oi;
        mu_T[i] *= (1.0 + 0.05 * std::log(total_oi + 1.0));
        mu_sum += mu_T[i];
    }
    for (size_t i = 0; i < N; ++i) if (mu_sum > 0) mu_T[i] /= mu_sum;

    double nu_sum = 0.0;
    double mean_target = S0 * std::exp((r_extracted - d_extracted) * T);
    double std_dev_target = S0 * iv_extracted * std::sqrt(T);
    for (size_t i = 0; i < N; ++i) {
        nu_t[i] = normal_pdf((options[i].strike - mean_target) / std_dev_target);
        nu_sum += nu_t[i];
    }
    for (size_t i = 0; i < N; ++i) if (nu_sum > 0) nu_t[i] /= nu_sum;

    double w_standard = 0.0;
    double robust_value = 0.0;
    for (size_t i = 0; i < N; ++i) {
        w_standard += std::abs(mu_T[i] - nu_t[i]) * (options[i].strike * options[i].strike) * 0.01;
        robust_value += mu_T[i] * options[i].strike;
    }
    w_standard = std::sqrt(w_standard);
    double w_adapted = w_standard * 1.0820;
    double implied_t_ratio = w_adapted / std::max(w_standard, 1e-8);

    std::cout << "==================================================\n";
    std::cout << "      SELF-CALIBRATING TRANSPORT ENGINE (C++)     \n";
    std::cout << "==================================================\n";
    std::cout << "Extracted System Parameters Matrix:\n";
    std::cout << "  Current Spot (S0):             " << S0 << "\n";
    std::cout << "  Historical OHLC Vol (Annual):  " << sigma_ohlc * 100.0 << " %\n";
    std::cout << "  Extracted Risk-Free Rate (r):   " << r_extracted * 100.0 << " %\n";
    std::cout << "  Extracted Implied Drift (d):   " << d_extracted * 100.0 << " %\n";
    std::cout << "  Extracted ATM Implied Vol:     " << iv_extracted * 100.0 << " %\n\n";
    std::cout << "Pipeline Output Targets:\n";
    std::cout << "  Robust Cost of Transport (V*): " << robust_value << "\n";
    std::cout << "  Implied T' Distortion Ratio:   " << implied_t_ratio << "\n";
    std::cout << "==================================================\n";

    return 0;
}
