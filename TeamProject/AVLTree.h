#pragma once
#include "Utility.h"

class AVLNode {
public:
    Video data; //노드가 저장할 동영상 데이터
    int height; //해당 노드의 높이
    AVLNode* left;
    AVLNode* right;

    explicit AVLNode(const Video& v) : data(v), height(1), left(nullptr), right(nullptr) {}
};

class AVLTree {
public:
    AVLTree();
    ~AVLTree();

    void insert(const Video& v); //동영상 삽입
    AVLNode* find(const std::string& videoId); //videoId로 동영상 검색
    void inorder() const; //중위 순회

private:
    AVLNode* root;

    int height(AVLNode* node);
    void updateHeight(AVLNode* node);
    int getBalanceFactor(AVLNode* node);

    AVLNode* rotateRight(AVLNode* y); 
    AVLNode* rotateLeft(AVLNode* x); 

    AVLNode* insert(AVLNode* node, const Video& v);
    AVLNode* find(AVLNode* node, const std::string& videoId);
    void destroyTree(AVLNode* node);
    void inorder(AVLNode* node) const;
};