#include "Utility.h"
#include "Heap.h"

Heap::Heap() : Heap(MIN) {}

Heap::Heap(Mode m) {
    comp = (m == MAX) ? &greaterfn : &lessfn;
}

Heap::Heap(const vector<Score>& data, Mode m) : a(data) {
    comp = (m == MAX) ? &greaterfn : &lessfn;
    heapify();
}

void Heap::push(const Score& x) {
    a.push_back(x);
    siftUp((int)a.size() - 1);
}

void Heap::pop() {
    if (a.empty()) throw out_of_range("pop on empty heap");//예외 처리
    swap(a[0], a.back());
    a.pop_back();
    if (!a.empty()) siftDown(0);
}

const Score& Heap::top() const {
    if (a.empty()) throw out_of_range("top on empty heap");
    return a[0];
}

bool Heap::empty() const { return a.empty(); }
int Heap::size() const { return (int)a.size(); }//벡터는 size()호출시 size_t반환 하므로 명시적 형변환
const vector<Score>& Heap::data() const { return a; }

bool Heap::lessfn(const Score& x, const Score& y) { return x < y; }
bool Heap::greaterfn(const Score& x, const Score& y) { return x > y; }

void Heap::heapify() {
    for (int i = (int)a.size() / 2 - 1; i >= 0; --i) siftDown(i);
}

void Heap::siftUp(int i) {
    while (i > 0) {
        int p = (i - 1) / 2;
        if (!comp(a[i], a[p])) break;
        swap(a[i], a[p]);
        i = p;
    }
}

void Heap::siftDown(int i) {
    int n = (int)a.size();
    while (true) {
        int l = i * 2 + 1;
        int r = i * 2 + 2;
        int best = i;

        if (l < n && comp(a[l], a[best])) best = l;
        if (r < n && comp(a[r], a[best])) best = r;

        if (best == i) break;
        swap(a[i], a[best]);
        i = best;
    }
}
