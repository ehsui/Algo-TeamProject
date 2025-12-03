#include "RankingSort.h"
#include <vector>
#include <algorithm>

using namespace std;

void calculateTrendScores(vector<Video>& p){
    for(auto& v:p){
        v.score = (int)(v.viewCount*1 + v.likeCount*50 + v.commentCount*10);
    }
}

static void countingSortForVideo(vector<Video>& p, long long (Video::*field), long long exp) {
    int size = (int)p.size();
    if (size <= 0) return;
    
    int count[10] = { 0 };
    vector<Video> out(size);

    for (int i = 0; i < size; i++) {
        long long value = p[i].*field;
        int digit = (int)((value / exp) % 10);
        count[9 - digit]++; // 내림차순
    }

    for (int d = 1; d < 10; d++) count[d] += count[d - 1];

    for (int i = size - 1; i >= 0; i--) {
        long long value = p[i].*field;
        int digit = (int)((value / exp) % 10);
        out[--count[9 - digit]] = p[i];
    }
    
    p = move(out);
}

void radixSort_Ranking(vector<Video>& p) {
    long long (Video::*priority_fields[]) = { 
        &Video::commentCount, 
        &Video::viewCount, 
        &Video::likeCount 
    };

    for (auto field : priority_fields) {
        long long max_val = 0;
        for (const auto& v : p) {
            if ((v.*field) > max_val) max_val = (v.*field);
        }
        
        for (long long exp = 1; max_val / exp > 0; exp *= 10) {
            countingSortForVideo(p, field, exp);
        }
    }
}