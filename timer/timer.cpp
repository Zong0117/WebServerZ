#include "timer.h"

MinTimeHeap::MinTimeHeap() 
{
   _heap.reserve(100);  //初始化大小
}

MinTimeHeap::~MinTimeHeap()
{
    _heap.clear();      
    _mp.clear();
}

void MinTimeHeap::Add(int id, int timeout, const CallBackFunc& cb)
{
    assert(id >= 0);
    if(_mp.count(id) == 0)  //没找到 队尾插入新节点 调整位置
    {
        size_t pos = _heap.size();  //保存新节点位置
        _mp[id] = pos;              //key：id, value:pos
        _heap.push_back({id, Clock::now() + MillSec(timeout), cb});
        _ShiftUp(pos);
    }
    else    //找到了 直接调整位置
    {
        size_t pos = _mp[id];  //根据id获取位置索引
        //修改参数
        _heap[pos].expries = Clock::now() + MillSec(timeout);   
        _heap[pos].callBack = cb;
        //调整位置
        if(!_ShiftDn(pos))
            _ShiftUp(pos);
    }
}

void MinTimeHeap::Tick()
{
    //堆头清除超时节点
    if(_heap.empty())
        return;
    while(!_heap.empty())   
    {
        TimeNode node = _heap.front();  
        //计算是否超时
        if(std::chrono::duration_cast<MillSec>
           (node.expries - Clock::now()).count() > 0)
        {
            break;
        }
        //无剩余时间回调 删除
        node.callBack();
        pop();
    }
}
void MinTimeHeap::Adjust(int id, int timeout)
{
    //修改对应id节点并调整位置
    assert(!_heap.empty() && _mp.count(id) > 0); 
    _heap[_mp[id]].expries = Clock::now() + MillSec(timeout);
    _ShiftDn(_mp[id]);
}
void MinTimeHeap::IdCallBack(int id)
{
    //对应id节点删除并回调
    if(_heap.empty() || _mp.count(id) == 0)
        return;
    size_t pos = _mp[id];
    _heap[pos].callBack();
    _DelNode(pos);
}
int MinTimeHeap::NextTick()
{
    //清除超时节点
    Tick();
    size_t ms = 0;
    if(!_heap.empty())
    {
        ms = std::chrono::duration_cast<MillSec>
            (_heap.front().expries - Clock::now()).count();
        if(ms < 0)
            ms = 0;
    }
    return ms;
    
}

void MinTimeHeap::pop()
{
    assert(!_heap.empty());
    _DelNode(0);
}

void MinTimeHeap::_SwapNode(size_t lp, size_t rp)
{
    assert(lp >= 0 && lp < _heap.size());
    assert(rp >= 0 && rp < _heap.size());
    std::swap(_heap[lp], _heap[rp]);
    _mp[_heap[lp].TNkey] = lp;
    _mp[_heap[rp].TNkey] = rp;
}

void MinTimeHeap::_ShiftUp(size_t pos)
{
    assert(pos >= 0 && pos < _heap.size());
    size_t parent = (pos - 1) / 2;
    while(parent >= 0)
    {
        if(_heap[parent] < _heap[pos])
            break;
        _SwapNode(pos, parent);
        pos = parent;
        parent = (pos - 1) / 2;
    }
}

bool MinTimeHeap::_ShiftDn(size_t pos)
{
    assert(pos >= 0 && pos < _heap.size());
    size_t child = pos * 2 + 1;
    int size = _heap.size();
    size_t temp = pos;
    while(child < size)
    {
        if(child + 1 < size && _heap[child + 1] < _heap[child])
            ++child;
        if(_heap[child] < _heap[temp])
            break;
        _SwapNode(temp, child);
        temp = child;
        child = temp * 2 + 1;
    }
    return pos < temp;
}

void MinTimeHeap::_DelNode(size_t pos)
{
    assert(!_heap.empty() && pos >= 0 && pos < _heap.size());
    size_t cur = pos;
    size_t last = _heap.size() - 1;
    assert(cur <= last);
    if(cur < last)
    {
        _SwapNode(cur, last);
        if(!_ShiftDn(cur))
        {
            _ShiftUp(cur);
        }
    }
    _heap.pop_back();
    _mp.erase(_heap.back().TNkey);
}
