#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>
using namespace std;

template <typename Element>
class Heap {
public:
    enum Mode { MIN = -1, MAX = +1 };

    Heap() : Heap(MIN) {}

    explicit Heap(Mode m) {
        comp = (m == MAX) ? &greaterfn : &lessfn;
    }

    explicit Heap(const vector<Element>& data, Mode m = MIN)
        : a(data) {
        comp = (m == MAX) ? &greaterfn : &lessfn;
        heapify();
    }

    void push(const Element& x) {
        a.push_back(x);
        siftUp((int)a.size() - 1);
    }

    void pop() {
        if (a.empty()) throw out_of_range("pop on empty heap");
        swap(a[0], a.back());
        a.pop_back();
        if (!a.empty()) siftDown(0);
    }

    const Element& top() const {
        if (a.empty()) throw out_of_range("top on empty heap");
        return a[0];
    }

    bool empty() const { return a.empty(); }
    int  size()  const { return (int)a.size(); }
    const vector<Element>& data() const { return a; }

private:
    vector<Element> a;

    using Cmp = bool (*)(const Element&, const Element&);
    Cmp comp;

    static bool lessfn(const Element& x, const Element& y) { return x < y; }
    static bool greaterfn(const Element& x, const Element& y) { return x > y; }

    void heapify() {
        for (int i = (int)a.size() / 2 - 1; i >= 0; --i) siftDown(i);
    }

    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (!comp(a[i], a[p])) break;
            swap(a[i], a[p]);
            i = p;
        }
    }

    void siftDown(int i) {
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
};
