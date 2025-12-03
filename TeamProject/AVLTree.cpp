#include "AVLTree.h"
#include <algorithm>

using namespace std;

AVLTree::AVLTree() : root(nullptr) {}
AVLTree::~AVLTree() { destroy(root); }

int AVLTree::height(AVLNode* node) {
    return (node == nullptr) ? 0 : node->height;
}

int AVLTree::getBalance(AVLNode* node) {
    return (node == nullptr) ? 0 : height(node->left) - height(node->right);
}

void AVLTree::updateHeight(AVLNode* node) {
    if (node) node->height = 1 + max(height(node->left), height(node->right));
}

AVLNode* AVLTree::rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y);
    updateHeight(x);
    return x;
}

AVLNode* AVLTree::leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x);
    updateHeight(y);
    return y;
}

// [비교 로직] 점수가 높으면 "더 크다(인기 있다)"고 판단 -> 트리의 왼쪽에 배치
bool AVLTree::isMorePopular(const Video& a, const Video& b) {
    if (a.score != b.score) return a.score > b.score; 
    return a.viewCount > b.viewCount; // 점수 같으면 조회수 증가량으로 비교
}

AVLNode* AVLTree::insert(AVLNode* node, const Video& v) {
    if (node == nullptr) return new AVLNode(v);

    // 인기 많은(큰) 값을 왼쪽으로 보냄 -> 중위 순회 시 내림차순 정렬됨
    if (isMorePopular(v, node->data)) {
        node->left = insert(node->left, v);
    } else {
        node->right = insert(node->right, v);
    }

    updateHeight(node);
    int balance = getBalance(node);

    // 균형 맞추기 (4가지 케이스)
    if (balance > 1 && isMorePopular(v, node->left->data))
        return rightRotate(node);

    if (balance < -1 && !isMorePopular(v, node->right->data))
        return leftRotate(node);

    if (balance > 1 && !isMorePopular(v, node->left->data)) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    if (balance < -1 && isMorePopular(v, node->right->data)) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

void AVLTree::insert(const Video& v) {
    root = insert(root, v);
}

void AVLTree::inorder(AVLNode* node, vector<Video>& list) {
    if (node == nullptr) return;
    inorder(node->left, list);  // 왼쪽 (상위권)
    list.push_back(node->data);
    inorder(node->right, list); // 오른쪽 (하위권)
}

void AVLTree::destroy(AVLNode* node) {
    if (node) {
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
}

vector<Video> AVLTree::getSortedList() {
    vector<Video> list;
    inorder(root, list);
    return list;
}