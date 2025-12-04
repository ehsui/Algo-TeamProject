/**
 * @file Video.cpp
 * @brief YouTube 영상 정보 구조체 구현
 */

#include "Video.h"

void Video::updateInfo(const Video& v) {
    // 통계 정보 업데이트
    viewCount = v.viewCount;
    likeCount = v.likeCount;
    commentCount = v.commentCount;
    
    // 메타 정보 업데이트
    fetchTimestamp = v.fetchTimestamp;
    
    // 점수 재계산
    calculateScore();
}

key Video::makekey() const {
    return key{ score, videoId, title };
}

void Video::calculateScore() {
    // Score.h의 함수 사용
    score = CalculateScore(viewCount, likeCount, commentCount);
}