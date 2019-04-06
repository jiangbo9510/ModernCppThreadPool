#include"thread_pool.hpp"
#include<iostream>
int add(int a,int b)
{
    int c = a + b;
    c+=1;
    return c;
}

int main()
{
    thread_pool pool(10);
    std::vector<std::future<int>> results;

    for(int i = 0; i < 100 ; i++)
    {
        auto res = pool.addTask(add,i,i+1);
        std::cout << "add succ" << std::endl;
        results.push_back(std::move(res));
    }

    for(auto &r : results)
    {
        std::cout << "res = " << r.get() << std::endl;
    }


    pool.stop();
}