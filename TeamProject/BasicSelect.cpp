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
Score quickSelect(vector<Score>& p,int top) {
	if (p.empty()) throw out_of_range("empty vector");
	return quickSelect(p, top, 0, (int)p.size() - 1);
}// 이진 탐색과 다른점은 무엇일까.. 하하 -> 피벗 선택 다르게 하기.
// 전처리 함수
Score binaryselect(vector<Score>& p,vector<Score>& toplist, int top) {
	if (p.empty()) throw out_of_range("empty vector");
	if (top < 0 || top >= (int)p.size()) throw out_of_range("k out of range");
	auto result = minmax_element(p.begin(), p.end());
	auto mn_it = result.first;
	auto mx_it = result.second;
	return binaryselect(p, toplist, top, *mn_it, *mx_it);
}

Score binaryselect(vector<Score>& p, vector<Score>& result, int top, Score minv, Score maxv) {
    // (선택) 재사용 시 정리
    result.clear();
    result.reserve(top + 1);      
    vector<Score> cur = p;
    vector<Score> big, small;
    int remain = top + 1;         
    Score lo = minv, hi = maxv;

    while (lo < hi) {
        Score mid = (Score)(((long long)lo + hi + 1) / 2);

        big.clear(); small.clear();

        
        for (Score x : cur) {
            if (x >= mid) big.push_back(x);
            else          small.push_back(x);
        }

        if ((int)big.size() >= remain) {
            
            lo = mid;
            cur.swap(big);
        }
        else {
            // mid 큼: big은 전부 확정 포함
            for (Score x : big) result.push_back(x);
            remain -= (int)big.size();     

            hi = mid - 1;
            cur.swap(small);
        }
    }

    //부족하면 lo로 채우기
    if ((int)result.size() < top + 1) {
        for (Score x : p) if (x == lo) {
            result.push_back(x);
            if ((int)result.size() == top + 1) break;
        }
    }

    if ((int)result.size() > top + 1) {
        nth_element(result.begin(), result.begin() + top, result.end(), greater<Score>());
        result.resize(top + 1);
    }

    return lo;  // 커트라인
}

