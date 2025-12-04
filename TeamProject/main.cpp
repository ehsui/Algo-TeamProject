/**
 * @file main.cpp
 * @brief YouTube 실시간 랭킹 시스템 메인 진입점
 * 
 * [프로젝트 구조]
 * 
 * ┌─────────────────┐     ┌─────────────────┐
 * │   YouTube API   │     │   Dummy Data    │
 * │    Response     │     │    Provider     │
 * └────────┬────────┘     └────────┬────────┘
 *          │                       │
 *          └───────────┬───────────┘
 *                      ▼
 *          ┌─────────────────────┐
 *          │   IDataProvider     │  ← 데이터 소스 추상화
 *          │    (Interface)      │
 *          └──────────┬──────────┘
 *                     │
 *          ┌──────────▼──────────┐
 *          │   YouTubeAdapter    │  ← API 응답 → 도메인 모델 변환
 *          └──────────┬──────────┘
 *                     │
 *          ┌──────────▼──────────┐
 *          │    VideoMetrics     │  ← 내부 도메인 모델
 *          │   (Domain Model)    │
 *          └──────────┬──────────┘
 *                     │
 *          ┌──────────▼──────────┐
 *          │   ScoreCalculator   │  ← 점수 계산 (전략 패턴)
 *          └──────────┬──────────┘
 *                     │
 *          ┌──────────▼──────────┐
 *          │   SortingStrategy   │  ← 정렬 알고리즘 (템플릿)
 *          └──────────┬──────────┘
 *                     │
 *          ┌──────────▼──────────┐
 *          │   RankingEngine     │  ← 통합 랭킹 엔진
 *          └──────────┬──────────┘
 *                     │
 *                     ▼
 *          [  Ranking Results  ]
 */

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

// 새로운 아키텍처 헤더
#include "Domain/VideoMetrics.h"
#include "Adapter/DummyDataProvider.h"
#include "Scoring/ScoreCalculator.h"
#include "Sorting/SortingStrategy.hpp"
#include "Engine/RankingEngine.h"

using namespace std;

// ============================================================================
// 유틸리티 함수
// ============================================================================

void printSeparator(const string& title = "") {
    cout << "\n";
    if (!title.empty()) {
        cout << "===== " << title << " =====" << endl;
    } else {
        cout << "======================================" << endl;
    }
}

void printMenu() {
    printSeparator("YouTube 실시간 랭킹 시스템");
    cout << "1. 전체 랭킹 조회" << endl;
    cout << "2. 정렬 알고리즘 변경" << endl;
    cout << "3. 점수 계산 전략 변경" << endl;
    cout << "4. 랭킹 새로고침" << endl;
    cout << "5. 정렬 알고리즘 벤치마크" << endl;
    cout << "6. 특정 영상 순위 조회" << endl;
    cout << "0. 종료" << endl;
    printSeparator();
}

void printSortAlgorithms() {
    cout << "\n[정렬 알고리즘 선택]" << endl;
    cout << "1. Selection Sort  (O(n²))" << endl;
    cout << "2. Insertion Sort  (O(n²))" << endl;
    cout << "3. Bubble Sort     (O(n²))" << endl;
    cout << "4. Quick Sort      (O(n log n) avg)" << endl;
    cout << "5. Merge Sort      (O(n log n))" << endl;
    cout << "6. Shell Sort      (O(n^1.5))" << endl;
    cout << "7. Heap Sort       (O(n log n))" << endl;
    cout << "8. std::sort       (O(n log n))" << endl;
}

void printScoreStrategies() {
    cout << "\n[점수 계산 전략 선택]" << endl;
    cout << "1. View Weighted     (조회수 중심)" << endl;
    cout << "2. Engagement Based  (참여도 중심)" << endl;
    cout << "3. Trending          (트렌딩/최근성)" << endl;
    cout << "4. Balanced          (균형)" << endl;
}

// ============================================================================
// 벤치마크 함수
// ============================================================================

void runBenchmark(const vector<Domain::VideoMetrics>& originalData) {
    printSeparator("정렬 알고리즘 벤치마크");
    
    vector<pair<Sorting::Algorithm, string>> algorithms = {
        {Sorting::Algorithm::SelectionSort, "Selection Sort"},
        {Sorting::Algorithm::InsertionSort, "Insertion Sort"},
        {Sorting::Algorithm::BubbleSort, "Bubble Sort"},
        {Sorting::Algorithm::QuickSort, "Quick Sort"},
        {Sorting::Algorithm::MergeSort, "Merge Sort"},
        {Sorting::Algorithm::ShellSort, "Shell Sort"},
        {Sorting::Algorithm::HeapSort, "Heap Sort"},
        {Sorting::Algorithm::StdSort, "std::sort"}
    };
    
    cout << "\n데이터 크기: " << originalData.size() << "개" << endl;
    cout << "--------------------------------------" << endl;
    cout << left << setw(20) << "알고리즘" << setw(15) << "소요 시간" << endl;
    cout << "--------------------------------------" << endl;
    
    for (const auto& [algo, name] : algorithms) {
        // 데이터 복사
        vector<Domain::VideoMetrics> data = originalData;
        
        // 시간 측정
        auto start = chrono::high_resolution_clock::now();
        
        Sorting::sort(data, algo,
            [](const Domain::VideoMetrics& a, const Domain::VideoMetrics& b) {
                return a.score > b.score;
            });
        
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        
        cout << left << setw(20) << name 
             << setw(10) << duration.count() << " μs" << endl;
    }
    
    cout << "--------------------------------------" << endl;
}

// ============================================================================
// 메인 함수
// ============================================================================

int main() {
    // 콘솔 UTF-8 설정 (Windows)
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif
    
    cout << "시스템 초기화 중..." << endl;
    
    // 1. 데이터 제공자 생성 (테스트용 더미 데이터)
    auto dataProvider = make_unique<Adapter::DummyDataProvider>(1000, 42);
    
    // 2. 랭킹 엔진 설정
    Engine::RankingConfig config;
    config.topK = 100;
    config.sortAlgorithm = Sorting::Algorithm::QuickSort;
    config.scoreStrategy = Scoring::Strategy::Balanced;
    
    // 3. 랭킹 엔진 생성 및 초기화
    Engine::RankingEngine engine(config);
    engine.setDataProvider(move(dataProvider));
    
    if (!engine.build()) {
        cerr << "랭킹 엔진 초기화 실패: " << engine.getLastError() << endl;
        return 1;
    }
    
    cout << "초기화 완료! " << engine.getVideoCount() << "개 영상 로드됨." << endl;
    
    // 메인 루프
    int choice;
    bool running = true;
    
    while (running) {
        printMenu();
        cout << "선택: ";
        cin >> choice;
        
        switch (choice) {
            case 1: {
                // 전체 랭킹 조회
                int count;
                cout << "출력할 순위 수: ";
                cin >> count;
                engine.printRanking(count);
                break;
            }
            
            case 2: {
                // 정렬 알고리즘 변경
                printSortAlgorithms();
                int algoChoice;
                cout << "선택: ";
                cin >> algoChoice;
                
                if (algoChoice >= 1 && algoChoice <= 8) {
                    Sorting::Algorithm algos[] = {
                        Sorting::Algorithm::SelectionSort,
                        Sorting::Algorithm::InsertionSort,
                        Sorting::Algorithm::BubbleSort,
                        Sorting::Algorithm::QuickSort,
                        Sorting::Algorithm::MergeSort,
                        Sorting::Algorithm::ShellSort,
                        Sorting::Algorithm::HeapSort,
                        Sorting::Algorithm::StdSort
                    };
                    engine.setSortAlgorithm(algos[algoChoice - 1]);
                    engine.build();  // 재정렬
                    cout << "알고리즘 변경 완료!" << endl;
                }
                break;
            }
            
            case 3: {
                // 점수 계산 전략 변경
                printScoreStrategies();
                int stratChoice;
                cout << "선택: ";
                cin >> stratChoice;
                
                if (stratChoice >= 1 && stratChoice <= 4) {
                    Scoring::Strategy strats[] = {
                        Scoring::Strategy::ViewWeighted,
                        Scoring::Strategy::EngagementBased,
                        Scoring::Strategy::Trending,
                        Scoring::Strategy::Balanced
                    };
                    engine.setScoreStrategy(strats[stratChoice - 1]);
                    engine.build();  // 재계산 및 재정렬
                    cout << "전략 변경 완료!" << endl;
                }
                break;
            }
            
            case 4: {
                // 랭킹 새로고침
                cout << "랭킹 새로고침 중..." << endl;
                if (engine.refresh()) {
                    cout << "새로고침 완료!" << endl;
                } else {
                    cout << "새로고침 실패: " << engine.getLastError() << endl;
                }
                break;
            }
            
            case 5: {
                // 벤치마크
                // 벤치마크용 더미 데이터 생성
                Adapter::DummyDataProvider benchProvider(5000, 123);
                auto result = benchProvider.fetchVideos({5000, "", "", "KR", true});
                
                // 점수 계산
                Scoring::ScoreCalculator calc(Scoring::Strategy::Balanced);
                calc.calculateAll(result.videos);
                
                runBenchmark(result.videos);
                break;
            }
            
            case 6: {
                // 특정 영상 순위 조회
                cout << "영상 순위 조회 (1~" << engine.getVideoCount() << "): ";
                int rank;
                cin >> rank;
                
                auto video = engine.getVideoAtRank(rank);
                if (video) {
                    printSeparator("영상 정보");
                    cout << "순위: #" << rank << endl;
                    cout << "제목: " << video->title << endl;
                    cout << "채널: " << video->channelName << endl;
                    cout << "점수: " << static_cast<int>(video->score) << endl;
                    cout << "조회수: " << video->viewCount << endl;
                    cout << "좋아요: " << video->likeCount << endl;
                    cout << "댓글: " << video->commentCount << endl;
                } else {
                    cout << "해당 순위의 영상이 없습니다." << endl;
                }
                break;
            }
            
            case 0:
                running = false;
                cout << "프로그램을 종료합니다." << endl;
                break;
            
            default:
                cout << "잘못된 선택입니다." << endl;
                break;
        }
    }
    
    return 0;
}
