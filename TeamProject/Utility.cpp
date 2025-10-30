#include "Utility.h"

inline void swapValueheap(Heap& det, vector<Score>& src, int i) {
	det.pop(); det.push(src[i]);
}
void copyValue(Heap& heap, Score Origin) {
	int size = heap.size();
	if()
	heap.push(Origin);
}
int partition_d(vector<Score>& p, int left, int right) {
    int pivot = p[(left + right) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (p[i] > pivot) i++;
        while (p[j] < pivot) j--;
        if (i <= j) {
            swap(p[i], p[j]);
            i++; j--;
        }
    }
    return i;
}
