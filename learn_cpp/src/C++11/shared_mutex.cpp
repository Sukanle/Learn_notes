#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fmt/color.h>
#include <mutex>
#include <thread>
#include <random>
#include <shared_mutex>
#include <functional>
#include <array>

int g_num = 0;

using namespace std::literals;

void pfn(std::mutex & m);
void pfns(std::shared_mutex& m);
void wfn(std::mutex& m);
void wfns(std::shared_mutex& m);

template<typename mutex_type,
typename = typename std::enable_if_t<std::is_same_v<mutex_type, std::mutex> || std::is_same_v<mutex_type, std::shared_mutex>>>
double text(mutex_type& mtxs, std::array<std::function<void(mutex_type&)>,2> fn){

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    auto lambda = [&mtxs](std::function<void(mutex_type&)> fnptr){
        for(int i = 0; i < 10; ++i){
            fnptr(mtxs);
            std::this_thread::sleep_for(500ms);
        }
    }; 

    std::thread w_thread(lambda, fn[0]);
    std::array<std::thread, 4> p_threads;
    for(auto& t : p_threads)
        t = std::thread(lambda, fn[1]);
    w_thread.join();
    for(auto& t : p_threads)
        t.join();
    end = std::chrono::system_clock::now();

    system("clear");

    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}


int main() {
    std::mutex mtx;
    std::shared_mutex smtx;

    std::array<std::function<void(std::mutex&)>,2> fn_arr = {wfn, pfn};
    std::array<std::function<void(std::shared_mutex&)>,2> fns_arr = {wfns, pfns};

    double time=text(mtx, fn_arr);
    double time_s=text(smtx, fns_arr);

    fmt::print(stdout, "Use mutex:\n");
    fmt::print(stdout, "Time: {}\n\n", time);

    fmt::print(stdout, "Use shared_mutex:\n");
    fmt::print(stdout, "Time: {}\n", time_s);

    return 0;
}

void pfn(std::mutex & m){
    std::lock_guard<std::mutex> lock(m);
    fmt::print(stdout, "pfn[{}], g_num = {}\n", std::this_thread::get_id(), g_num);
    //模拟耗时操作
    std::this_thread::sleep_for(500ms);
}

void wfn(std::mutex& m){
    std::lock_guard<std::mutex> lock(m);
    //获取随机数
    g_num = std::random_device()();
    auto str_f = fmt::format(fg(fmt::color::green) | fmt::emphasis::bold, "wfn[{}]", std::this_thread::get_id());
    fmt::print(stdout, "{}, g_num = {}\n", str_f, g_num);
    //模拟耗时操作
    std::this_thread::sleep_for(1s);
}
void pfns(std::shared_mutex& m){
    std::shared_lock<std::shared_mutex> lock(m);
    fmt::print(stdout, "pfn[{}], g_num = {}\n", std::this_thread::get_id(), g_num);
    //模拟耗时操作
    std::this_thread::sleep_for(500ms);
}

void wfns(std::shared_mutex& m){
    std::lock_guard<std::shared_mutex> lock(m);
    //获取随机数
    g_num = std::random_device()();
    auto str_f = fmt::format(fg(fmt::color::green) | fmt::emphasis::bold, "wfn[{}]", std::this_thread::get_id());
    fmt::print(stdout, "{}, g_num = {}\n", str_f, g_num);
    //模拟耗时操作
    std::this_thread::sleep_for(1s);
}
