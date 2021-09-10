#pragma once
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <algorithm>
#include <cassert>

using CallBackFunc = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MillSec = std::chrono::milliseconds;
using TimePoint = Clock::time_point;

struct TimeNode
{
    int TNkey;
    TimePoint expries;  //时间过期
    CallBackFunc callBack;
    bool operator<(const TimeNode& tn)
    {    
        return expries < tn.expries; 
    }
};
class MinTimeHeap
{
public:
    MinTimeHeap();
    ~MinTimeHeap();

    void Add(int id, int timeout, const CallBackFunc& cb);  //添加节点
    void Tick();   //清除超时节点
    void Adjust(int id, int timeout);  //根据id调整节点位置
    void IdCallBack(int id);    //根据id删除节点并回调
    int NextTick(); //返回下一个到期时间
    void pop();
private:
    void _ShiftUp(size_t pos);    //节点上滤
    bool _ShiftDn(size_t pos);    //节点下滤
    void _SwapNode(size_t lpos, size_t rpos);   //交换
    void _DelNode(size_t pos);  //删除位置节点

    std::vector<TimeNode> _heap;
    std::unordered_map<int, size_t> _mp;
};

