/**
 * @file RankEngine.cpp
 * @brief 랭킹 엔진 구현
 */

#include "RankEngine.h"
#include "BasicSelect.hpp"  // 템플릿 선택 알고리즘

using namespace std;

RankEngine::RankEngine(RankPolicy policy) : P(policy) {}

void RankEngine::sortInterface() {
    print_cutline();
    cout << "\n사용할 정렬 알고리즘을 선택해주세요\n";
    
    for (int i = 0; i < 8; i++) {
        cout << i + 1 << ". " << sortname[i] << "\n";
    }
    
    print_cutline();
    
    while (true) {
        cout << "정렬 입력(숫자): ";
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 8) {
            P.s = static_cast<SortType>(a - 1);
            break;
        }
        
        cout << "잘못된 입력입니다. 다시 입력하세요.\n";
    }
}

void RankEngine::selectInterface() {
    print_cutline();
    cout << "\n사용할 선택(탐색) 알고리즘을 선택해주세요\n";
    cout << "[Top-K 요소를 찾는 알고리즘]\n\n";
    
    for (int i = 0; i < 4; i++) {
        cout << i + 1 << ". " << selectname[i];
        switch (i) {
            case 0: cout << "  - O(n log k)"; break;
            case 1: cout << "  - O(n) 평균"; break;
            case 2: cout << "  - O(n log max), 정수 전용"; break;
            case 3: cout << "  - O(n) 평균"; break;
        }
        cout << "\n";
    }
    
    print_cutline();
    
    while (true) {
        cout << "선택 알고리즘 입력(숫자): ";
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 4) {
            P.sel = static_cast<SelectType>(a - 1);
            break;
        }
        
        cout << "잘못된 입력입니다. 다시 입력하세요.\n";
    }
}

void RankEngine::multiMetricInterface() {
    print_cutline();
    cout << "\n다중 지표 랭킹 설정\n";
    cout << "[사전식 비교: 첫 번째 지표가 같으면 두 번째로...]\n\n";
    
    cout << "1. 기본 (조회수 > 좋아요 > 댓글)\n";
    cout << "2. 트렌딩 (Δ조회수 > Δ좋아요 > Δ댓글)\n";
    cout << "3. 참여도 (좋아요 > 댓글 > 조회수)\n";
    cout << "4. 커스텀 설정\n";
    
    print_cutline();
    
    while (true) {
        cout << "설정 선택(숫자): ";
        int a;
        cin >> a;
        
        switch (a) {
            case 1:
                P.metricConfig = MultiMetricConfig::defaultConfig();
                return;
            case 2:
                P.metricConfig = MultiMetricConfig::trendingConfig();
                return;
            case 3:
                P.metricConfig = MultiMetricConfig::engagementConfig();
                return;
            case 4: {
                // 커스텀 설정
                P.metricConfig.priority.clear();
                cout << "\n지표 우선순위를 설정하세요 (최대 5개, 0으로 종료)\n";
                cout << "1: 조회수  2: 좋아요  3: 댓글  4: Δ조회수  5: Δ좋아요  6: Δ댓글\n";
                
                for (int i = 0; i < 5; i++) {
                    cout << "지표 " << (i + 1) << " (0=종료): ";
                    int m;
                    cin >> m;
                    
                    if (m == 0) break;
                    
                    MetricType type;
                    switch (m) {
                        case 1: type = MetricType::AbsoluteViews; break;
                        case 2: type = MetricType::AbsoluteLikes; break;
                        case 3: type = MetricType::AbsoluteComments; break;
                        case 4: type = MetricType::DeltaViews; break;
                        case 5: type = MetricType::DeltaLikes; break;
                        case 6: type = MetricType::DeltaComments; break;
                        default: continue;
                    }
                    P.metricConfig.priority.push_back(type);
                }
                
                if (P.metricConfig.priority.empty()) {
                    P.metricConfig = MultiMetricConfig::defaultConfig();
                }
                return;
            }
            default:
                cout << "잘못된 입력입니다. 다시 입력하세요.\n";
        }
    }
}

void RankEngine::mapping(string videoId, int rank) {
    pos[videoId] = rank;
}

void RankEngine::setData(vector<key>& Data, vector<Video>& Src) {
    Data.clear();
    Data.reserve(Src.size());
    
    for (const Video& v : Src) {
        Data.push_back(v.makekey());
    }
}

void RankEngine::build() {
    switch (P.a) {
        case AlgorithmType::BasicSort:
            build_sortAll();
            break;
        case AlgorithmType::SelectThenSort:
            build_selectThenSort();
            break;
        case AlgorithmType::AVLTree:
            build_AVLTree();
            break;
        case AlgorithmType::OnlineInsert:
            build_onlineInsert();
            break;
        case AlgorithmType::MultiMetric:
            build_MultiMetric();
            break;
    }
    
    // 랭킹 맵 구축
    for (size_t i = 0; i < cur.size(); i++) {
        pos[cur[i].videoId] = static_cast<int>(i);
    }
}

void RankEngine::updateScore(const string& videoId, Score newScore) {
    auto it = pos.find(videoId);
    if (it != pos.end()) {
        int idx = it->second;
        cur[idx].Value = newScore;
        adjust(idx);  // 순위 재조정
    }
}

vector<key> RankEngine::getTopK() const {
    int k = min(P.k, static_cast<int>(cur.size()));
    return vector<key>(cur.begin(), cur.begin() + k);
}

void RankEngine::savePrevRanking() {
    prevCur = cur;
    prevMulti = curMulti;
}

void RankEngine::printRanking() {
    print_cutline();
    cout << "\n===== TOP " << P.k << " 랭킹 =====\n";
    cout << "[방식: " << AlgoName[P.a];
    if (P.a == AlgorithmType::BasicSort) {
        cout << " / 정렬: " << sortname[P.s];
    } else if (P.a == AlgorithmType::SelectThenSort) {
        cout << " / 선택: " << selectname[P.sel] << " / 정렬: " << sortname[P.s];
    } else if (P.a == AlgorithmType::MultiMetric) {
        cout << " / 지표: ";
        for (size_t i = 0; i < P.metricConfig.priority.size(); i++) {
            if (i > 0) cout << " > ";
            cout << getMetricName(P.metricConfig.priority[i]);
        }
    }
    cout << "]\n\n";
    
    // 다중 지표 모드일 때
    if (P.a == AlgorithmType::MultiMetric && !curMulti.empty()) {
        for (size_t i = 0; i < curMulti.size() && i < static_cast<size_t>(P.k); i++) {
            cout << "#" << (i + 1) << " | ";
            for (size_t j = 0; j < curMulti[i].metrics.size(); j++) {
                if (j > 0) cout << " > ";
                cout << curMulti[i].metrics[j];
            }
            cout << " | " << curMulti[i].title << "\n";
        }
    } else {
        auto topK = getTopK();
        for (size_t i = 0; i < topK.size(); i++) {
            cout << "#" << (i + 1) << " | Score: " << topK[i].Value 
                 << " | " << topK[i].title << "\n";
        }
    }
    print_cutline();
}

void RankEngine::printPrevRanking() {
    if (prevCur.empty() && prevMulti.empty()) {
        print_cutline();
        cout << "\n직전 랭킹이 없습니다. (갱신을 먼저 수행해주세요)\n";
        print_cutline();
        return;
    }
    
    print_cutline();
    cout << "\n===== 직전 랭킹 (갱신 전) =====\n";
    cout << "[방식: " << AlgoName[P.a] << "]\n\n";
    
    if (P.a == AlgorithmType::MultiMetric && !prevMulti.empty()) {
        for (size_t i = 0; i < prevMulti.size() && i < static_cast<size_t>(P.k); i++) {
            cout << "#" << (i + 1) << " | ";
            for (size_t j = 0; j < prevMulti[i].metrics.size(); j++) {
                if (j > 0) cout << " > ";
                cout << prevMulti[i].metrics[j];
            }
            cout << " | " << prevMulti[i].title << "\n";
        }
    } else {
        int k = min(P.k, static_cast<int>(prevCur.size()));
        for (int i = 0; i < k; i++) {
            cout << "#" << (i + 1) << " | Score: " << prevCur[i].Value 
                 << " | " << prevCur[i].title << "\n";
        }
    }
    print_cutline();
}

void RankEngine::printTimeStats() {
    print_cutline();
    cout << "\n===== 시간 측정 결과 =====\n";
    cout << "초기 빌드 시간: " << buildTimeMs << " ms\n";
    if (refreshCount > 0) {
        cout << "최근 갱신 시간: " << refreshTimeMs << " ms\n";
        cout << "총 갱신 횟수: " << refreshCount << "회\n";
    } else {
        cout << "갱신 기록 없음\n";
    }
    print_cutline();
}

void RankEngine::resultMenu() {
    while (true) {
        print_cutline();
        cout << "\n[결과 메뉴]\n";
        cout << "1. 현재 랭킹 보기\n";
        
        if (hasRefreshData) {
            cout << "2. 새로고침 (두 번째 시점 데이터로 갱신)\n";
        } else {
            cout << "2. 새로고침 (동일 데이터로 갱신)\n";
        }
        
        cout << "3. 직전 랭킹 보기\n";
        cout << "4. 시간 측정 결과\n";
        cout << "5. 종료\n";
        print_cutline();
        
        cout << "메뉴 선택: ";
        int ch;
        cin >> ch;
        
        switch (ch) {
            case 1:
                printRanking();
                break;
            case 2: {
                cout << "\n데이터 갱신 중...\n";
                
                if (hasRefreshData) {
                    // 두 번째 시점 데이터로 갱신
                    refresh(refreshData);
                } else {
                    // 동일 데이터로 갱신 (테스트용)
                    refresh(srcData);
                }
                
                cout << "갱신 완료! (소요 시간: " << refreshTimeMs << " ms)\n";
                printRanking();
                break;
            }
            case 3:
                printPrevRanking();
                break;
            case 4:
                printTimeStats();
                break;
            case 5:
                return;
            default:
                cout << "잘못된 입력입니다. 다시 입력하세요.\n";
        }
    }
}

void RankEngine::interface(vector<Video>& Src) {
    // 외부 데이터 저장
    srcData = Src;
    hasRefreshData = false;
    
    print_cutline();
    cout << "유튜브 랭킹 출력 프로그램\n\n";
    print_cutline();
    cout << "\n1. 시작하기\n2. 종료하기\n";
    
    while (true) {
        cout << "메뉴 선택: ";
        int ch;
        cin >> ch;
        
        if (ch == 2) return;
        if (ch == 1) break;
        
        cout << "잘못된 입력입니다. 다시 입력하세요.\n";
    }
    
    // 랭킹 방식 선택
    print_cutline();
    cout << "\n사용할 랭킹 방식을 선택해주세요\n";
    for (int i = 0; i < 5; i++) {
        cout << i + 1 << ". " << AlgoName[i];
        if (i == 0) cout << " (전체 정렬)";
        if (i == 1) cout << " (선택 후 정렬)";
        if (i == 3) cout << " (실시간 부분 갱신)";
        cout << "\n";
    }
    print_cutline();
    
    while (true) {
        cout << "랭킹 방식 입력(숫자): ";
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 5) {
            P.a = static_cast<AlgorithmType>(a - 1);
            break;
        }
        
        cout << "잘못된 입력입니다. 다시 입력하세요.\n";
    }
    
    // Top-K 설정
    print_cutline();
    while (true) {
        cout << "\n출력할 순위 수를 입력해주세요 (1~" << srcData.size() << "): ";
        int k;
        cin >> k;
        
        if (k >= 1 && k <= static_cast<int>(srcData.size())) {
            P.k = k;
            break;
        }
        
        cout << "잘못된 입력입니다. 다시 입력하세요.\n";
    }
    
    // 데이터 설정
    setData(cur, srcData);
    
    // 방식에 따라 추가 옵션 선택
    if (P.a == AlgorithmType::BasicSort) {
        // 전체 정렬: 정렬 알고리즘만 선택
        sortInterface();
    } 
    else if (P.a == AlgorithmType::SelectThenSort) {
        // 선택 후 정렬: 선택 알고리즘 + 정렬 알고리즘 선택
        selectInterface();
        sortInterface();
    }
    else if (P.a == AlgorithmType::OnlineInsert) {
        // 온라인 삽입: 선택 + 정렬 (초기 빌드용)
        selectInterface();
        sortInterface();
    }
    else if (P.a == AlgorithmType::MultiMetric) {
        // 다중 지표: 지표 우선순위 설정
        multiMetricInterface();
        
        // 다중 지표 키 생성
        curMulti.clear();
        curMulti.reserve(srcData.size());
        for (const Video& v : srcData) {
            MultiMetricKey mmk = createMultiMetricKey(
                v.videoId,
                v.title,
                v.viewCount,
                v.likeCount,
                v.commentCount,
                P.metricConfig
            );
            curMulti.push_back(mmk);
        }
    }
    
    // 빌드 (시간 측정)
    cout << "\n초기 빌드 중...\n";
    auto startBuild = chrono::high_resolution_clock::now();
    build();
    auto endBuild = chrono::high_resolution_clock::now();
    buildTimeMs = chrono::duration<double, milli>(endBuild - startBuild).count();
    cout << "빌드 완료! (소요 시간: " << buildTimeMs << " ms)\n";
    
    // 결과 출력
    printRanking();
    
    // 결과 메뉴 진입
    resultMenu();
}

// ============================================================================
// CSV 파일 로드 인터페이스
// ============================================================================

void RankEngine::interface() {
    print_cutline();
    cout << "유튜브 랭킹 출력 프로그램\n\n";
    cout << "[CSV 파일 로드 모드]\n";
    print_cutline();
    cout << "\n1. 시작하기\n2. 종료하기\n";
    
    while (true) {
        cout << "메뉴 선택: ";
        int ch;
        cin >> ch;
        
        if (ch == 2) return;
        if (ch == 1) break;
        
        cout << "잘못된 입력입니다. 다시 입력하세요.\n";
    }
    
    // CSV 파일 경로 입력
    print_cutline();
    cout << "\nCSV 파일 경로를 입력해주세요\n";
    cout << "(예: C:/data/youtube.csv)\n";
    print_cutline();
    
    cin.ignore();  // 버퍼 비우기
    string csvPath;
    cout << "파일 경로: ";
    getline(cin, csvPath);
    
    // 시점 목록 확인
    vector<string> timestamps = CsvDataLoader::getTimestamps(csvPath);
    
    if (timestamps.empty()) {
        cout << "\nCSV 파일을 읽을 수 없거나 데이터가 없습니다.\n";
        return;
    }
    
    cout << "\n발견된 시점: ";
    for (size_t i = 0; i < timestamps.size(); i++) {
        if (i > 0) cout << ", ";
        cout << timestamps[i];
    }
    cout << " (" << timestamps.size() << "개)\n";
    
    // 랭킹 방식 선택
    print_cutline();
    cout << "\n사용할 랭킹 방식을 선택해주세요\n";
    for (int i = 0; i < 5; i++) {
        cout << i + 1 << ". " << AlgoName[i];
        if (i == 0) cout << " (전체 정렬)";
        if (i == 1) cout << " (선택 후 정렬)";
        if (i == 2) cout << " (AVL 트리) - 두 시점 지원";
        if (i == 3) cout << " (실시간 부분 갱신) - 두 시점 지원";
        cout << "\n";
    }
    print_cutline();
    
    while (true) {
        cout << "랭킹 방식 입력(숫자): ";
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 5) {
            P.a = static_cast<AlgorithmType>(a - 1);
            break;
        }
        
        cout << "잘못된 입력입니다. 다시 입력하세요.\n";
    }
    
    // 데이터 로드 방식 결정
    bool useTwoTimestamps = (P.a == AlgorithmType::AVLTree || P.a == AlgorithmType::OnlineInsert);
    
    if (useTwoTimestamps && timestamps.size() >= 2) {
        // 두 시점 데이터 분리 로드
        cout << "\n두 시점 데이터를 로드합니다...\n";
        cout << "  - 초기 데이터: " << timestamps[0] << "\n";
        cout << "  - 갱신 데이터: " << timestamps[1] << "\n";
        
        auto [initial, refresh] = CsvDataLoader::loadAndSplit(csvPath);
        srcData = initial;
        refreshData = refresh;
        hasRefreshData = true;
        
        cout << "로드 완료! (초기: " << srcData.size() << "개, 갱신: " << refreshData.size() << "개)\n";
    } else {
        // 단일 시점 또는 전체 로드
        cout << "\n전체 데이터를 로드합니다...\n";
        srcData = CsvDataLoader::loadAll(csvPath);
        hasRefreshData = false;
        
        cout << "로드 완료! (" << srcData.size() << "개)\n";
    }
    
    if (srcData.empty()) {
        cout << "\n데이터가 없습니다. 종료합니다.\n";
        return;
    }
    
    // Top-K 설정
    print_cutline();
    while (true) {
        cout << "\n출력할 순위 수를 입력해주세요 (1~" << srcData.size() << "): ";
        int k;
        cin >> k;
        
        if (k >= 1 && k <= static_cast<int>(srcData.size())) {
            P.k = k;
            break;
        }
        
        cout << "잘못된 입력입니다. 다시 입력하세요.\n";
    }
    
    // 데이터 설정
    setData(cur, srcData);
    
    // 방식에 따라 추가 옵션 선택
    if (P.a == AlgorithmType::BasicSort) {
        sortInterface();
    } 
    else if (P.a == AlgorithmType::SelectThenSort) {
        selectInterface();
        sortInterface();
    }
    else if (P.a == AlgorithmType::AVLTree) {
        // AVL 트리: 추가 옵션 없음
    }
    else if (P.a == AlgorithmType::OnlineInsert) {
        selectInterface();
        sortInterface();
    }
    else if (P.a == AlgorithmType::MultiMetric) {
        multiMetricInterface();
        
        curMulti.clear();
        curMulti.reserve(srcData.size());
        for (const Video& v : srcData) {
            MultiMetricKey mmk = createMultiMetricKey(
                v.videoId,
                v.title,
                v.viewCount,
                v.likeCount,
                v.commentCount,
                P.metricConfig
            );
            curMulti.push_back(mmk);
        }
    }
    
    // 빌드 (시간 측정)
    cout << "\n초기 빌드 중...\n";
    auto startBuild = chrono::high_resolution_clock::now();
    build();
    auto endBuild = chrono::high_resolution_clock::now();
    buildTimeMs = chrono::duration<double, milli>(endBuild - startBuild).count();
    cout << "빌드 완료! (소요 시간: " << buildTimeMs << " ms)\n";
    
    if (hasRefreshData) {
        cout << "\n[알림] 두 번째 시점 데이터가 준비되어 있습니다.\n";
        cout << "결과 메뉴에서 '새로고침'을 선택하면 갱신을 테스트할 수 있습니다.\n";
    }
    
    // 결과 출력
    printRanking();
    
    // 결과 메뉴 진입
    resultMenu();
}
