#include "AVLTree.h"
#include <iostream>
#include <algorithm>

using namespace std;
AVLTree::AVLTree() : root(nullptr) {}

AVLTree::~AVLTree() {
    destroyTree(root);
}

void AVLTree::insert(const Video& v) {
    root = insert(root, v);
}

AVLNode* AVLTree::find(const std::string& videoId) {
    return find(root, videoId);
}

void AVLTree::inorder() const {
    inorder(root);
}

int AVLTree::height(AVLNode* node){ //노드 높이 반환
    if(node == nullptr)
        return 0;
    return node->height;
}

void AVLTree::updateHeight(AVLNode* node){
    if (node!=nullptr){
        node->height = 1 + max(height(node->left), height(node->right));
    }
}

AVLNode* AVLTree::getBalanceFactor(AVLNode* node) {
    if (node == nullptr) return 0;
    return height(node->left) - height(node->right);
}
AVLNode* AVLTree::rotateRight(AVLNode* y){
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;

    updateHeight(y);
    updateHeight(x);
    return x;
}
AVLNode* AVLTree::rotateLeft(AVLNode* x){
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);
    return y;
}

AVLNode* AVLTree::insert(AVLNode* node, const Video& v) {
    if (node == nullptr) {
        return new AVLNode(v);
    }

    if (v.videoId < node->data.videoId) {
        node->left = insert(node->left, v);
    } else if (v.videoId > node->data.videoId) {
        node->right = insert(node->right, v);
    } else {
        return node;
    }

    updateHeight(node);
    int balance = getBalanceFactor(node);

    if (balance > 1 && v.videoId < node->left->data.videoId) {
        return rotateRight(node);
    }

    if (balance < -1 && v.videoId > node->right->data.videoId) {
        return rotateLeft(node);
    }

    if (balance > 1 && v.videoId > node->left->data.videoId) {
        node->left = rotateLeft(node->left); 
        return rotateRight(node); 
    }

    if (balance < -1 && v.videoId < node->right->data.videoId) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

AVLNode* AVLTree::find(AVLNode* node, const std::string& videoId){
    if (node == nullptr) return nullptr;
    if (videoId == node->data.videoId) return node;
    if (videoId < node->data.videoId) return find(node->left, videoId);
    return find(node->right, videoId);
}
void AVLTree::destroyTree(AVLNode* node){
    if (node != nullptr) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}
void AVLTree::inorder(AVLNode* node) const{
    if (node != nullptr) {
        inorder(node->left);
        cout << "Video ID : " << node->data.videoId 
             << "ㄴ Score : " << node->data.score 
             << "ㄴ Height : " << node->height << endl;
        inorder(node->right);
    }
}