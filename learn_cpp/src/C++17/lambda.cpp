#include <iostream>
#include <thread>
#include <vector>
#include <functional>

class Worker {
public:
    Worker(int id) : id(id) {}

    void doWork() const {
        // 使用lambda表达式捕获*this
        auto task = [*this]() {
            std::cout << "Worker " << id << " is doing work in thread " << std::this_thread::get_id() << std::endl;
        };

        // 启动多个线程执行任务
        std::vector<std::thread> threads;
        for (int i = 0; i < 5; ++i) {
            threads.emplace_back(task);
        }

        // 等待所有线程完成
        for (auto& t : threads) {
            t.join();
        }
    }

private:
    int id;
};

int main() {
    Worker worker(1);
    worker.doWork();

    return 0;
}
