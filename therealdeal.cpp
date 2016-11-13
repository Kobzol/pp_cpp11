#include <iostream>
#include <functional>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <numeric>

#include "stream/stream.h"

#include "vectorization/vectorization.h"

void streams()
{
    std::vector<int> input{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    Stream<int>* s = new ValueStream<int>(input);
    Stream<double>* d = s
            ->map<double>([](auto x) { return x + 1; })
            ->filter([](auto x) { return x > 5; })
            ->take(3);

    std::cout << d->reduce(0, [](auto lhs, auto rhs) { return lhs + rhs; }) << std::endl;

    for (auto& item : d->collect())
    {
        std::cout << item << std::endl;
    }

    delete d;
}

class Test
{
public:
    int a = 5;
    int get(int x) { return x + this->a; }
};
void partial_application()
{
    auto fn = [](int x, int y) {
        return x + y;
    };

    auto fn1 = std::bind(fn, 1, std::placeholders::_1);
    std::cout << fn1(5) << std::endl;

    Test test{8};

    auto fn2 = std::bind(&Test::get, test, 6);
    std::cout << fn2() << std::endl;
}

void foreach()
{
    std::unordered_map<std::string, int> map;
    map["ahoj"] = 5;
    map["cus"] = 10;

    for (auto& it : {1,2,3})
    {
        std::cout << it << ": " << it << std::endl;
    }
}

void threads_1()
{
    int sum(0);
    std::mutex m;

    std::thread threads[4];
    for (int i = 0; i < 4; i++)
    {
        threads[i] = std::thread([&sum, &m]()
         {
             auto start = std::chrono::steady_clock::now();
             std::lock_guard<std::mutex> guard(m);
             for (int j = 0; j < 1000000; j++)
             {
                 sum++;
             }
             std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::steady_clock::now() - start
             ).count() << std::endl;
         });
    }

    for (int i = 0; i < 4; i++)
    {
        threads[i].join();
    }

    std::cout << sum << std::endl;
}

void threads_2()
{
    auto accumulate = [](std::vector<int>& v,
                         std::promise<int> accumulate_promise)
    {
        int sum = std::accumulate(v.begin(), v.end(), 0);
        accumulate_promise.set_value(sum);  // Notify future
    };

    std::vector<int> numbers = { 1, 2, 3, 4, 5, 6 };
    std::promise<int> accumulate_promise;
    std::future<int> accumulate_future = accumulate_promise.get_future();

    auto handle = std::async(std::launch::async, accumulate, std::ref(numbers), std::move(accumulate_promise));
    accumulate_future.wait();  // wait for result
    std::cout << "result=" << accumulate_future.get() << std::endl;
}
