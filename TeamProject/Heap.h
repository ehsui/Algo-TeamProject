#pragma once
#include "Utility.h"

class Heap {
public:
    enum Mode { MIN = -1, MAX = +1 };//최소 힙과 최대 힙을 결정시킬 수 있는 열거형 선언

    Heap();
    explicit Heap(Mode m);
    explicit Heap(const vector<Score>& data, Mode m = MIN);

    void push(const Score& x);
    void pop();
    const Score& top() const;
    bool empty() const;
    int size() const;
    const vector<Score>& data() const;

private:
    vector<Score> a;
    typedef bool (*Cmp)(const Score&, const Score&);//최대힙과 최소힙은 비교 기준이 달라지기 때문에 그때마다 <, >을 바꿔 주기 위한 함수 포인터 
    Cmp comp;//상황 마다 바뀌는 논리(bool)을 리턴하는 함수 포인터 하나 선언

    static bool lessfn(const Score& x, const Score& y);// 최대 힙을 위한 연산자 x<y를 만들어 MAX로 설정되면 comp가 이 함수를 가르키도록 설정
    static bool greaterfn(const Score& x, const Score& y);// 위와 반대

    void heapify();
    void siftUp(int i);//push 할때 사용할 함수
    void siftDown(int i);//pop 혹은 heapify할때 사용할 함수
};