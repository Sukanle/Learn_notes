#include<fmt/format.h>
#include<fmt/ostream.h>
#include<condition_variable>
#include<mutex>
#include<thread>

class shared_mutex {
private:
    std::mutex _mtx;
    std::condition_variable _reader_cv;
    std::condition_variable _writer_cv;
    bool is_writer_entered = false;
    unsigned _reader_count = 0;

    static constexpr unsigned int MAX_REABDER = 1000;

public:

    shared_mutex() = default;
    ~shared_mutex() = default;
    shared_mutex(shared_mutex&&) = delete;

    void lock_shared() {
        std::unique_lock<std::mutex> _lock;
        _writer_cv.wait(_lock, [this] { return !is_writer_entered; });
        _reader_count++;
    }
    void lock() {
        std::unique_lock<std::mutex> _lock;
        _reader_cv.wait(_lock, [this] { return _reader_count == 0; });
        _writer_cv.wait(_lock, [this] { return !is_writer_entered && _reader_count == 0; });
    }
    void unlock_shared() {
        _reader_count--;
        _reader_cv.notify_all();
    }
    void unlock() {
        is_writer_entered = false;
        _writer_cv.notify_all();
        _reader_cv.notify_all();
    }
};

int main() {
    using namespace std::literals;
    shared_mutex mtx;
    std::thread t1([&mtx] {
        mtx.lock();
        fmt::print("Thread 1 is writing\n");
        std::this_thread::sleep_for(5s);
        mtx.unlock();
    });
    std::thread t2([&mtx] {
        for(int i=0;i<10;i++){
            mtx.lock_shared();
            fmt::print("Thread 2 is reading\n");
            std::this_thread::sleep_for(500ms);
            mtx.unlock_shared();
        }
    });
    std::thread t3([&mtx] {
        for(int i=0;i<10;i++){
            mtx.lock_shared();
            fmt::print("Thread 3 is reading\n");
            std::this_thread::sleep_for(500ms);
            mtx.unlock_shared();
        }
    });
    t1.join();
    t2.join();
    t3.join();
    return 0;
}
