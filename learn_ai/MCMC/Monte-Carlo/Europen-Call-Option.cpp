#include<fmt/ostream.h>
#include<random>
#include<cmath>
#include<tbb/parallel_for.h>

int main(){
    const double S0=100;
    const double K=110;
    const double T=1;
    const double r=0.05;
    const double sigma=0.2;
    const int simulations=1000;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0,1.0);

    std::vector<double> payoffs(simulations);
    tbb::parallel_for(tbb::blocked_range<int>(0,simulations),[&](tbb::blocked_range<int> &range){
        for(int i=range.begin();i!=range.end();++i){
            double Z=dist(gen);
            double ST=S0*exp((r - 0.5 * sigma * sigma) * T + sigma * sqrt(T) * Z);
            payoffs[i] = std::max(ST - K, 0.0);
        }
    });
    double mean_payoff = std::accumulate(
        payoffs.begin(), payoffs.end(), 0.0
    ) / simulations;
    
    double C0 = std::exp(-r * T) * mean_payoff;
    fmt::print("\n===== Final Results =====\n");
    fmt::print("蒙特卡洛估算的欧式看涨期权价格: {:.4f}\n" ,C0);
    return 0;
}
