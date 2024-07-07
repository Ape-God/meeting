#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <thread>
#include <utility>
#include <vector>
#include <random>
#include <iostream>

template<typename T>
class SafeQueue{
    std::queue<T> _que;
    std::mutex _mutex;
public:
    bool empty(){
        std::lock_guard<std::mutex> lock(_mutex);
        return _que.empty();
    }

    size_t size(){
        std::lock_guard<std::mutex> lock(_mutex);
        return _que.size();
    }

    void push(T &t){
        std::lock_guard<std::mutex> lock(_mutex);
        _que.push(t);
    }

    bool pop(T &t){
        std::lock_guard<std::mutex> lock(_mutex);
        if(_que.empty()){
            return false;
        }
        t = std::move(_que.front());
        _que.pop();
        return true;
    }
};

class ThreadPool{
    
private:
    class Worker{
        int _work_id;
        ThreadPool* _pool;
    public:
        Worker(ThreadPool* pool, int id):_work_id(id), _pool(pool){};


        void operator()(){
            std::function<void()> func;
            
            bool get_task = false;

            while (!_pool->_shutdown)
            {
                {
                    std::unique_lock<std::mutex> lock(_pool->_mutex);
                    if(_pool->_safe_que.empty()){
                        _pool->_conditional_lock.wait(lock);
                    }
                    get_task = _pool->_safe_que.pop(func);
                }
                // 执行
                if(get_task){
                    func();
                }

            }
        }
    };

    SafeQueue<std::function<void()>> _safe_que;
    std::vector<std::thread> _threads;
    std::mutex _mutex;
    std::condition_variable _conditional_lock;
    bool _shutdown;

public:
    explicit ThreadPool(int n_threads = 4): _threads(std::vector<std::thread>(n_threads)), _shutdown(false){};
    // ThreadPool(const ThreadPool& ) = delete;
    // ThreadPool(ThreadPool&& ) = delete;
    // ThreadPool& operator=(const ThreadPool& ) = delete;
    // ThreadPool& operator=(ThreadPool&& ) = delete;


    void init(){
        for(int i = 0; i < _threads.size(); i++){
            _threads[i] = std::thread(Worker(this, i)); // 分配工作线程
            
        }
    }

    void shutdown(){
        _shutdown = true;
        _conditional_lock.notify_all(); // 唤醒所有线程

        // 等待关闭所有线程
        for(int i = 0; i < _threads.size(); i++){
            if(_threads[i].joinable()){ // 线程是否在等待
                _threads[i].join();
            }
        }
    }

    template<typename F, typename... Args>
    auto add(F &&f, Args && ...args) -> std::future<decltype(f(args...))> {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        std::function<void()> warpper_fun = [task_ptr](){
            (*task_ptr)();
        };

        _safe_que.push(warpper_fun);
        _conditional_lock.notify_one(); // 唤醒一个线程

        return task_ptr->get_future();
    }
};



// std::random_device rd; // 真实随机数产生器
// std::mt19937 mt(rd()); //生成计算随机数mt
// std::uniform_int_distribution<int> dist(-1000, 1000); //生成-1000到1000之间的离散均匀分布数
// auto rnd = std::bind(dist, mt);

// // 设置线程睡眠时间
// void simulate_hard_computation()
// {
//     std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
// }

// // 添加两个数字的简单函数并打印结果
// void multiply(const int a, const int b)
// {
//     simulate_hard_computation();
//     const int res = a * b;
//     std::cout << a << " * " << b << " = " << res << std::endl;
// }

// // 添加并输出结果
// void multiply_output(int &out, const int a, const int b)
// {
//     simulate_hard_computation();
//     out = a * b;
//     std::cout << a << " * " << b << " = " << out << std::endl;
// }

// // 结果返回
// int multiply_return(const int a, const int b)
// {
//     simulate_hard_computation();
//     const int res = a * b;
//     std::cout << a << " * " << b << " = " << res << std::endl;
//     return res;
// }


// int main(){
//     // 创建3个线程的线程池
//     hust::ThreadPool pool(5);
//     pool.init();
//     // 提交乘法操作，总共30个
//     for (int i = 1; i <= 3; ++i)
//         for (int j = 1; j <= 10; ++j) {
//             pool.add(multiply, i, j);
//         }

//     // 使用ref传递的输出参数提交函数
//     int output_ref;
//     auto future1 = pool.add(multiply_output, std::ref(output_ref), 5, 6);
//     future1.get();
//     std::cout << "Last operation result is equals to " << output_ref << std::endl;

//     // 使用return参数提交函数
//     auto future2 = pool.add(multiply_return, 5, 3);

//     // 等待乘法输出完成
//     int res = future2.get();
//     std::cout << "Last operation result is equals to " << res << std::endl;

//     // 关闭线程池
//     pool.shutdown();
//     return 0;
// }