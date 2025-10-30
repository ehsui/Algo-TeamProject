#pragma once
#include "Headers.h"
#include "Heap.h"
#include "Score.h"

using namespace std;

typedef struct YouTubeVideoInfo {
   //기본 정보
    string videoId;            // 영상 ID
    string channelId;          // 업로더 채널 ID
    string channelTitle;       // 업로더 채널 제목
    string title;              // 영상 제목
    string description;        // 영상 설명 (길 수 있음)
    string categoryId;         // 영상 카테고리 ID
    vector<string> tags;       // 태그 리스트 (옵션)
    string publishedAt;        // 업로드 일시
    Score score;

    // 썸네일 정보 (필요하다면 확장 가능)
    string thumbnailUrl;       // 기본 썸네일 URL

    // 통계 수치 변화량 계산은 따로
    long long viewCount = 0;
    long long likeCount = 0;
    long long commentCount = 0;
    //다른 변수들 추가 할 필요 있음
    // 상세 정보
    int durationSeconds = 0;   // 영상 길이 초 단위로 변환해서 저장
    string definition;         // hd,sd 등
    string dimension;          // 2d,3d
    bool caption = false;      // 캡션(자막) 존재 여부
    bool licensedContent = false;  // 라이선스 여부
    // regionRestriction는 단순히 “차단된 국가 수” 정도로 저장해도 됨
    vector<string> regionBlocked;    // 차단된 국가 코드 리스트 

    // 상태 정보
    string privacyStatus;      // 공개 비공개 동영상
    bool embeddable = true;    // 임베드 가능 여부

    string fetchTimestamp;     // 이 데이터를 API로 초기화 시각(타임 스탬프, 우리가 만들어줘야함)
}Video;

int partition_d(vector<Score>& p, int left, int right);//힙 정렬 힙 선택 할때 사용될 파티션 함수 중복되서 이곳에 정의
inline void swapValueheap(Heap& det, vector<Score>& src, int i);//이 두 함수는 쓸모가 없음 
void copyValue(const Heap heap, int i, vector<Score>& q, int j);// 무시 해도됨
