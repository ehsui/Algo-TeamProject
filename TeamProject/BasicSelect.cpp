#include "BasicSelect.h"
#include "Utility.h"

using namespace std;

Score sequentialSelect(vector <Score>&p, Heap& topk, int top) {
	
	int size = p.size();
	for (int i = 0; i < top; i++)
	{
		topk.push(p[i]);
	}
	for (int i = top; i < size; i++)
	{
		if (topk.top() < p[i]) { topk.pop(); topk.push(p[i]); }
	}
	return topk.top();
}
// 커트라인을 반환하고 그과정에서 힙을 자연스럽게 구성하는 함수

Score quickSelect(vector<Score>& p, int top, int left, int right) {
	if (left == right) return p[left];
	int idx = partition_d(p, left, right);
	if (top < idx) {
		return quickSelect(p, top, left, idx - 1);
	}
	else {
		return quickSelect(p, top, idx, right);
	}
}
Score quickSelect(vector<Score>& p, int top) {
	if (p.empty()) throw out_of_range("empty vector");
	return quickSelect(p, top, 0, (int)p.size() - 1);
}// 이진 탐색과 다른점은 무엇일까.. 하하 -> 피벗 선택 다르게 하기.
Score binaryselect(vector<Score>& p, int top) {
	if (p.empty()) throw out_of_range("empty vector");
	pair <Score, Score>endvalue;
	endvalue.first = p[0] < p[1] ? p[0] : p[1];
	endvalue.second = p[0] > p[1] ? p[0] : p[1];
	for (int i = 0; i < p.size()-1; i++)
	{
		if (p[i] < endvalue.first) { endvalue.first = p[i]; }
		if (p[i] > endvalue.second) { endvalue.second = p[i]; }
	}
	return binaryselect(p, top, endvalue.first, endvalue.second);
}
Score binaryselect(vector<Score>& p, int top,Score min,Score max){
	Score mid = (min + max) / 2;
	vector<Score> big, small;
	
	for (int i = 0; i < p.size()-1; i++)
	{
		if (mid <= p[i]) { big.push_back(p[i]); }
		else { small.push_back(p[i]); }
	}
	Heap smallheap(small, Heap::MAX);
	if (big.size() == top)return mid;
	else if (big.size() > top)return binaryselect(big, top, small.top(), mid);
	else { return binaryselect(); }
}