#include "BasicSort.h"
#include "Utility.h"
#include <vector>
using namespace std;

void selectsort(vector<Score>& p) {
    int size = (int)p.size();
    for (int i = 0; i < size; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < size; j++) {
            if (p[j] > p[maxIdx]) maxIdx = j;
        }
        if (maxIdx != i) swap(p[i], p[maxIdx]);
    }
}

void insertionsort(vector<Score>& p) {
    int size = (int)p.size();
    for (int i = 1; i < size; i++) {
        for (int j = i; j > 0; j--) {
            if (p[j] > p[j - 1]) swap(p[j], p[j - 1]);
            else break;
        }
    }
}

void bubblesort(vector<Score>& p) {
    int size = (int)p.size();
    for (int i = 0; i < size - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < size - 1 - i; j++) {
            if (p[j] < p[j + 1]) {
                swap(p[j], p[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

void quicksort(vector<Score>& p, int left, int right) {
    if (left >= right) return;
    int idx = partition_d(p, left, right);
    if (left < idx - 1) quicksort(p, left, idx - 1);
    if (idx < right) quicksort(p, idx, right);
}

void quicksort(vector<Score>& p) {
    if (p.empty()) return;
    quicksort(p, 0, (int)p.size() - 1);
}

static void merge_d(vector<Score>& p, int left, int mid, int right) {
    int n = right - left + 1;
    vector<Score> temp(n);
    int i = left, j = mid + 1, k = 0;
    while (i <= mid && j <= right) {
        if (p[i] >= p[j]) temp[k++] = p[i++];
        else temp[k++] = p[j++];
    }
    while (i <= mid) temp[k++] = p[i++];
    while (j <= right) temp[k++] = p[j++];
    for (int t = 0; t < n; t++) p[left + t] = temp[t];
}

void mergesort(vector<Score>& p, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergesort(p, left, mid);
    mergesort(p, mid + 1, right);
    merge_d(p, left, mid, right);
}

void mergesort(vector<Score>& p) {
    if (p.empty()) return;
    mergesort(p, 0, (int)p.size() - 1);
}

void shellSort(vector<Score>& p) {
    int size = (int)p.size();
    for (int gap = size / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < size; i++) {
            int temp = p[i];
            int j = i;
            while (j - gap >= 0 && p[j - gap] < temp) {
                p[j] = p[j - gap];
                j -= gap;
            }
            p[j] = temp;
        }
    }
}

static void heapify_max(vector<Score>& p, int n, int i) {
    int largest = i;
    int l = i * 2 + 1;
    int r = i * 2 + 2;
    if (l < n && p[l] > p[largest]) largest = l;
    if (r < n && p[r] > p[largest]) largest = r;
    if (largest != i) {
        swap(p[i], p[largest]);
        heapify_max(p, n, largest);
    }
}

void heapSort(vector<Score>& p) {
    int size = (int)p.size();
    for (int i = size / 2 - 1; i >= 0; i--) heapify_max(p, size, i);
    for (int i = size - 1; i > 0; i--) {
        swap(p[0], p[i]);
        heapify_max(p, i, 0);
    }
    for (int i = 0, j = size - 1; i < j; i++, j--) swap(p[i], p[j]);
}

void countingSort(vector<Score>& p, int max) {
    int size = (int)p.size();
    if (size <= 0) return;
    int range = max + 1;
    vector<Score> count(range, 0), out(size);
    for (int v : p) count[v]++;
    for (int v = range - 2; v >= 0; v--) count[v] += count[v + 1];
    for (int i = 0; i < size; i++) {
        int v = p[i];
        out[--count[v]] = v;
    }
    p = move(out);
}

static void countingSortByDigit_a(vector<Score>& p, int exp) {
    int size = (int)p.size();
    int count[10] = { 0 };
    vector<Score> out(size);
    for (int i = 0; i < size; i++) {
        int digit = (p[i] / exp) % 10;
        count[9 - digit]++;
    }
    for (int d = 1; d < 10; d++) count[d] += count[d - 1];
    for (int i = size - 1; i >= 0; i--) {
        int digit = (p[i] / exp) % 10;
        out[--count[9 - digit]] = p[i];
    }
    p = move(out);
}

void radixSort(vector<Score>& p) {
    int size = (int)p.size();
    if (size <= 0) return;
    int mx = p[0];
    for (int i = 1; i < size; i++) if (p[i] > mx) mx = p[i];
    for (int exp = 1; mx / exp > 0; exp *= 10) {
        countingSortByDigit_a(p, exp);
    }
}
