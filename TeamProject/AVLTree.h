#pragma once
#include "Utility.h" // Video 구조체 사용을 위해 필요
#include <vector>
#include <algorithm>

// AVL 트리 노드 정의
struct AVLNode {
    Video data;
    int height;
    AVLNode* left;
    AVLNode* right;
    AVLNode(const Video& v) : data(v), height(1), left(nullptr), right(nullptr) {}
};

// AVL 트리 클래스 선언
class AVLTree {
private:
    AVLNode* root;

    // 보조 함수들
    int height(AVLNode* node);
    int getBalance(AVLNode* node);
    void updateHeight(AVLNode* node);
    
    // 회전 함수
    AVLNode* rightRotate(AVLNode* y);
    AVLNode* leftRotate(AVLNode* x);

    // 삽입 및 순회
    AVLNode* insert(AVLNode* node, const Video& v);
    void inorder(AVLNode* node, std::vector<Video>& list);
    void destroy(AVLNode* node);

    // [중요] 우선순위 비교 (점수 높은 게 우선)
    bool isMorePopular(const Video& a, const Video& b);

public:
    AVLTree();
    ~AVLTree();

    void insert(const Video& v);
    std::vector<Video> getSortedList(); // 정렬된 결과 반환
};