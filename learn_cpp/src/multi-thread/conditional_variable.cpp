#include <thread>
#include <mutex>
#include <condition_variable>
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

using namespace std::literals;

void rthread(std::mutex& mtx, std::condition_variable& cv, std::string& data){
    auto id = std::this_thread::get_id();
    std::unique_lock<std::mutex> lock(mtx);
    fmt::print(fg(fmt::color::yellow),"Received [{}] waiting for data\n", id);
    cv.wait(lock,[&data]{return !data.empty();});
    fmt::print(fg(fmt::color::green), "Received [{}] data: {}\n", id, data);
}
void sthread(std::mutex& mtx, std::condition_variable& cv, std::string& data){
    static int id = 0;
    std::string msg = fmt::format("Hello from thread {}", id);
    {
        std::unique_lock<std::mutex> lock(mtx);
        data = msg;
        fmt::print(fg(fmt::color::yellow), "Sent [{}] data: {}\n", id, data);
        id++;
    }
    cv.notify_one();
    std::this_thread::sleep_for(1ms);
}

int main(){
    std::mutex mtx;

    std::condition_variable cv;
    std::string shared_data;

    std::thread t1(rthread, std::ref(mtx), std::ref(cv), std::ref(shared_data));
    std::thread t2(sthread, std::ref(mtx), std::ref(cv), std::ref(shared_data));

    t1.join();
    t2.join();
    return 0;
}
