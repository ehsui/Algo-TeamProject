#pragma once
/**
 * @file AVLTree.hpp
 * @brief 템플릿 기반 AVL 트리 구현
 * 
 * [특징]
 * - 제네릭: 어떤 데이터 타입이든 사용 가능
 * - 커스텀 비교자: 정렬 기준 자유롭게 설정
 * - 자동 균형: 삽입/삭제 시 O(log n) 보장
 * 
 * [사용 예시]
 * // 기본 사용 (오름차순)
 * AVLTree<int> tree;
 * tree.insert(5);
 * 
 * // 커스텀 비교자 (내림차순)
 * AVLTree<int, std::greater<int>> descTree;
 * 
 * // Video 타입 (점수 기준 내림차순)
 * auto cmp = [](const Video& a, const Video& b) { return a.score > b.score; };
 * AVLTree<Video, decltype(cmp)> videoTree(cmp);
 */

#include <functional>
#include <vector>
#include <algorithm>
#include <stack>
#include <unordered_map>
#include <string>

namespace DataStructure {

// ============================================================================
// AVL 노드 템플릿
// ============================================================================

template <typename T>
struct AVLNode {
    T data;
    int height;
    AVLNode* left;
    AVLNode* right;
    
    explicit AVLNode(const T& value) 
        : data(value), height(1), left(nullptr), right(nullptr) {}
    
    explicit AVLNode(T&& value) 
        : data(std::move(value)), height(1), left(nullptr), right(nullptr) {}
};

// ============================================================================
// AVL 트리 템플릿 클래스
// ============================================================================

/**
 * @class AVLTree
 * @brief 자가 균형 이진 탐색 트리
 * 
 * @tparam T 저장할 데이터 타입
 * @tparam Compare 비교 함수 타입 (기본: std::less<T>)
 * 
 * Compare(a, b) == true이면 a가 b보다 "앞"에 위치합니다.
 * - std::less<T>: 오름차순 (작은 값이 앞)
 * - std::greater<T>: 내림차순 (큰 값이 앞)
 */
template <typename T, typename Compare = std::less<T>>
class AVLTree {
public:
    using Node = AVLNode<T>;
    using value_type = T;
    using size_type = std::size_t;
    
    // ========================================================================
    // 생성자/소멸자
    // ========================================================================
    
    /// 기본 생성자
    AVLTree() : root_(nullptr), size_(0), comp_() {}
    
    /// 비교자 지정 생성자
    explicit AVLTree(const Compare& comp) : root_(nullptr), size_(0), comp_(comp) {}
    
    /// 소멸자
    ~AVLTree() { clear(); }
    
    /// 복사 생성자
    AVLTree(const AVLTree& other) : root_(nullptr), size_(0), comp_(other.comp_) {
        copyFrom(other.root_);
    }
    
    /// 이동 생성자
    AVLTree(AVLTree&& other) noexcept 
        : root_(other.root_), size_(other.size_), comp_(std::move(other.comp_)) {
        other.root_ = nullptr;
        other.size_ = 0;
    }
    
    /// 복사 대입 연산자
    AVLTree& operator=(const AVLTree& other) {
        if (this != &other) {
            clear();
            comp_ = other.comp_;
            copyFrom(other.root_);
        }
        return *this;
    }
    
    /// 이동 대입 연산자
    AVLTree& operator=(AVLTree&& other) noexcept {
        if (this != &other) {
            clear();
            root_ = other.root_;
            size_ = other.size_;
            comp_ = std::move(other.comp_);
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    // ========================================================================
    // 기본 연산
    // ========================================================================
    
    /// 요소 삽입
    void insert(const T& value) {
        root_ = insertNode(root_, value);
        ++size_;
    }
    
    /// 요소 삽입 (이동)
    void insert(T&& value) {
        root_ = insertNode(root_, std::move(value));
        ++size_;
    }
    
    /// 요소 삭제
    bool remove(const T& value) {
        if (!contains(value)) return false;
        root_ = removeNode(root_, value);
        --size_;
        return true;
    }
    
    /// 요소 존재 여부 확인
    bool contains(const T& value) const {
        return findNode(root_, value) != nullptr;
    }
    
    /// 모든 요소 삭제
    void clear() {
        destroyTree(root_);
        root_ = nullptr;
        size_ = 0;
    }
    
    // ========================================================================
    // 조회
    // ========================================================================
    
    /// 요소 개수
    size_type size() const { return size_; }
    
    /// 비어있는지 확인
    bool empty() const { return size_ == 0; }
    
    /// 트리 높이
    int height() const { return getHeight(root_); }
    
    /// 최소값 (정렬 순서 기준 첫 번째)
    const T* minimum() const {
        if (!root_) return nullptr;
        Node* node = root_;
        while (node->left) node = node->left;
        return &node->data;
    }
    
    /// 최대값 (정렬 순서 기준 마지막)
    const T* maximum() const {
        if (!root_) return nullptr;
        Node* node = root_;
        while (node->right) node = node->right;
        return &node->data;
    }
    
    // ========================================================================
    // 정렬된 결과 반환
    // ========================================================================
    
    /// 정렬된 벡터 반환 (중위 순회)
    std::vector<T> toSortedVector() const {
        std::vector<T> result;
        result.reserve(size_);
        inorderTraversal(root_, result);
        return result;
    }
    
    /// 상위 K개 반환
    std::vector<T> topK(size_type k) const {
        std::vector<T> result;
        result.reserve(std::min(k, size_));
        inorderTopK(root_, result, k);
        return result;
    }
    
    // ========================================================================
    // 순회 (콜백 기반)
    // ========================================================================
    
    /// 중위 순회 (정렬 순서)
    template <typename Func>
    void inorder(Func&& func) const {
        inorderVisit(root_, std::forward<Func>(func));
    }
    
    /// 전위 순회
    template <typename Func>
    void preorder(Func&& func) const {
        preorderVisit(root_, std::forward<Func>(func));
    }
    
    /// 후위 순회
    template <typename Func>
    void postorder(Func&& func) const {
        postorderVisit(root_, std::forward<Func>(func));
    }

private:
    Node* root_;
    size_type size_;
    Compare comp_;
    
    // ========================================================================
    // 유틸리티 함수
    // ========================================================================
    
    int getHeight(Node* node) const {
        return node ? node->height : 0;
    }
    
    int getBalance(Node* node) const {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }
    
    void updateHeight(Node* node) {
        if (node) {
            node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        }
    }
    
    // ========================================================================
    // 회전 연산
    // ========================================================================
    
    Node* rightRotate(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;
        
        x->right = y;
        y->left = T2;
        
        updateHeight(y);
        updateHeight(x);
        
        return x;
    }
    
    Node* leftRotate(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;
        
        y->left = x;
        x->right = T2;
        
        updateHeight(x);
        updateHeight(y);
        
        return y;
    }
    
    // ========================================================================
    // 균형 조정
    // ========================================================================
    
    Node* balance(Node* node) {
        updateHeight(node);
        int balanceFactor = getBalance(node);
        
        // Left Heavy
        if (balanceFactor > 1) {
            if (getBalance(node->left) < 0) {
                // Left-Right Case
                node->left = leftRotate(node->left);
            }
            // Left-Left Case
            return rightRotate(node);
        }
        
        // Right Heavy
        if (balanceFactor < -1) {
            if (getBalance(node->right) > 0) {
                // Right-Left Case
                node->right = rightRotate(node->right);
            }
            // Right-Right Case
            return leftRotate(node);
        }
        
        return node;
    }
    
    // ========================================================================
    // 삽입
    // ========================================================================
    
    Node* insertNode(Node* node, const T& value) {
        if (!node) return new Node(value);
        
        if (comp_(value, node->data)) {
            node->left = insertNode(node->left, value);
        } else {
            node->right = insertNode(node->right, value);
        }
        
        return balance(node);
    }
    
    Node* insertNode(Node* node, T&& value) {
        if (!node) return new Node(std::move(value));
        
        if (comp_(value, node->data)) {
            node->left = insertNode(node->left, std::move(value));
        } else {
            node->right = insertNode(node->right, std::move(value));
        }
        
        return balance(node);
    }
    
    // ========================================================================
    // 삭제
    // ========================================================================
    
    Node* findMin(Node* node) const {
        while (node && node->left) node = node->left;
        return node;
    }
    
    Node* removeNode(Node* node, const T& value) {
        if (!node) return nullptr;
        
        if (comp_(value, node->data)) {
            node->left = removeNode(node->left, value);
        } else if (comp_(node->data, value)) {
            node->right = removeNode(node->right, value);
        } else {
            // 삭제할 노드 찾음
            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                delete node;
                return temp;
            }
            
            // 두 자식이 있는 경우: 후계자로 대체
            Node* successor = findMin(node->right);
            node->data = successor->data;
            node->right = removeNode(node->right, successor->data);
        }
        
        return balance(node);
    }
    
    // ========================================================================
    // 검색
    // ========================================================================
    
    Node* findNode(Node* node, const T& value) const {
        if (!node) return nullptr;
        
        if (comp_(value, node->data)) {
            return findNode(node->left, value);
        } else if (comp_(node->data, value)) {
            return findNode(node->right, value);
        }
        return node;
    }
    
    // ========================================================================
    // 순회
    // ========================================================================
    
    void inorderTraversal(Node* node, std::vector<T>& result) const {
        if (!node) return;
        inorderTraversal(node->left, result);
        result.push_back(node->data);
        inorderTraversal(node->right, result);
    }
    
    void inorderTopK(Node* node, std::vector<T>& result, size_type k) const {
        if (!node || result.size() >= k) return;
        inorderTopK(node->left, result, k);
        if (result.size() < k) {
            result.push_back(node->data);
        }
        if (result.size() < k) {
            inorderTopK(node->right, result, k);
        }
    }
    
    template <typename Func>
    void inorderVisit(Node* node, Func&& func) const {
        if (!node) return;
        inorderVisit(node->left, func);
        func(node->data);
        inorderVisit(node->right, func);
    }
    
    template <typename Func>
    void preorderVisit(Node* node, Func&& func) const {
        if (!node) return;
        func(node->data);
        preorderVisit(node->left, func);
        preorderVisit(node->right, func);
    }
    
    template <typename Func>
    void postorderVisit(Node* node, Func&& func) const {
        if (!node) return;
        postorderVisit(node->left, func);
        postorderVisit(node->right, func);
        func(node->data);
    }
    
    // ========================================================================
    // 복사/삭제
    // ========================================================================
    
    void copyFrom(Node* node) {
        if (!node) return;
        insert(node->data);
        copyFrom(node->left);
        copyFrom(node->right);
    }
    
    void destroyTree(Node* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
};

// ============================================================================
// 편의 타입 별칭
// ============================================================================

/// 내림차순 AVL 트리 (큰 값이 앞)
template <typename T>
using MaxAVLTree = AVLTree<T, std::greater<T>>;

/// 오름차순 AVL 트리 (작은 값이 앞)
template <typename T>
using MinAVLTree = AVLTree<T, std::less<T>>;

// ============================================================================
// Order Statistics AVL 노드 (서브트리 크기 포함)
// ============================================================================

template <typename T>
struct RankAVLNode {
    T data;
    int height;
    int subtreeSize;        // 서브트리 내 노드 개수 (자신 포함)
    RankAVLNode* left;
    RankAVLNode* right;
    
    explicit RankAVLNode(const T& value) 
        : data(value), height(1), subtreeSize(1), left(nullptr), right(nullptr) {}
    
    explicit RankAVLNode(T&& value) 
        : data(std::move(value)), height(1), subtreeSize(1), left(nullptr), right(nullptr) {}
};

// ============================================================================
// Order Statistics AVL 트리 (랭킹용)
// ============================================================================

/**
 * @class RankAVLTree
 * @brief 서브트리 크기 정보를 가진 Order Statistics AVL 트리
 * 
 * [핵심 기능]
 * - O(log n) 삽입/삭제/수정
 * - O(log n) k번째 요소 조회
 * - O(k) Top-K 추출
 * - O(1) ID로 노드 접근 (ID 맵 사용)
 * 
 * @tparam T 저장할 데이터 타입
 * @tparam Compare 비교 함수 타입 (기본: std::less<T>)
 * @tparam KeyExtractor ID 추출 함수 타입
 */
template <typename T, typename Compare = std::less<T>, 
          typename KeyExtractor = std::function<std::string(const T&)>>
class RankAVLTree {
public:
    using Node = RankAVLNode<T>;
    using value_type = T;
    using size_type = std::size_t;
    
    // ========================================================================
    // 생성자/소멸자
    // ========================================================================
    
    /// 기본 생성자
    RankAVLTree() : root_(nullptr), size_(0), comp_(), keyExtractor_(nullptr) {}
    
    /// 비교자 + 키 추출자 지정 생성자
    RankAVLTree(const Compare& comp, const KeyExtractor& keyExtractor) 
        : root_(nullptr), size_(0), comp_(comp), keyExtractor_(keyExtractor) {}
    
    /// 소멸자
    ~RankAVLTree() { clear(); }
    
    /// 복사 생성자
    RankAVLTree(const RankAVLTree& other) 
        : root_(nullptr), size_(0), comp_(other.comp_), keyExtractor_(other.keyExtractor_) {
        copyFrom(other.root_);
    }
    
    /// 이동 생성자
    RankAVLTree(RankAVLTree&& other) noexcept 
        : root_(other.root_), size_(other.size_), comp_(std::move(other.comp_)),
          keyExtractor_(std::move(other.keyExtractor_)), nodeMap_(std::move(other.nodeMap_)) {
        other.root_ = nullptr;
        other.size_ = 0;
    }
    
    /// 복사 대입 연산자
    RankAVLTree& operator=(const RankAVLTree& other) {
        if (this != &other) {
            clear();
            comp_ = other.comp_;
            keyExtractor_ = other.keyExtractor_;
            copyFrom(other.root_);
        }
        return *this;
    }
    
    /// 이동 대입 연산자
    RankAVLTree& operator=(RankAVLTree&& other) noexcept {
        if (this != &other) {
            clear();
            root_ = other.root_;
            size_ = other.size_;
            comp_ = std::move(other.comp_);
            keyExtractor_ = std::move(other.keyExtractor_);
            nodeMap_ = std::move(other.nodeMap_);
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    // ========================================================================
    // 기본 연산
    // ========================================================================
    
    /// 요소 삽입
    void insert(const T& value) {
        root_ = insertNode(root_, value);
        ++size_;
    }
    
    /// 요소 삽입 (이동)
    void insert(T&& value) {
        root_ = insertNode(root_, std::move(value));
        ++size_;
    }
    
    /// 요소 삭제
    bool remove(const T& value) {
        if (!contains(value)) return false;
        if (keyExtractor_) {
            nodeMap_.erase(keyExtractor_(value));
        }
        root_ = removeNode(root_, value);
        --size_;
        return true;
    }
    
    /// ID로 요소 삭제
    bool removeById(const std::string& id) {
        auto it = nodeMap_.find(id);
        if (it == nodeMap_.end()) return false;
        
        Node* node = it->second;
        root_ = removeNode(root_, node->data);
        nodeMap_.erase(it);
        --size_;
        return true;
    }
    
    /// ID로 요소 조회
    T* findById(const std::string& id) {
        auto it = nodeMap_.find(id);
        if (it == nodeMap_.end()) return nullptr;
        return &(it->second->data);
    }
    
    /// ID로 요소 조회 (const)
    const T* findById(const std::string& id) const {
        auto it = nodeMap_.find(id);
        if (it == nodeMap_.end()) return nullptr;
        return &(it->second->data);
    }
    
    /**
     * @brief ID로 데이터 업데이트 (삭제 후 재삽입)
     * @param id 업데이트할 요소의 ID
     * @param newData 새 데이터
     * @return 성공 여부
     * 
     * 시간복잡도: O(log n)
     */
    bool update(const std::string& id, const T& newData) {
        auto it = nodeMap_.find(id);
        if (it == nodeMap_.end()) return false;
        
        // 삭제 후 재삽입 (위치가 변할 수 있으므로)
        Node* oldNode = it->second;
        root_ = removeNode(root_, oldNode->data);
        nodeMap_.erase(it);
        --size_;
        
        // 새 데이터로 삽입
        insert(newData);
        return true;
    }
    
    /// 요소 존재 여부 확인
    bool contains(const T& value) const {
        return findNode(root_, value) != nullptr;
    }
    
    /// ID로 존재 여부 확인
    bool containsId(const std::string& id) const {
        return nodeMap_.find(id) != nodeMap_.end();
    }
    
    /// 모든 요소 삭제
    void clear() {
        destroyTree(root_);
        root_ = nullptr;
        size_ = 0;
        nodeMap_.clear();
    }
    
    // ========================================================================
    // 조회
    // ========================================================================
    
    /// 요소 개수
    size_type size() const { return size_; }
    
    /// 비어있는지 확인
    bool empty() const { return size_ == 0; }
    
    /// 트리 높이
    int height() const { return getHeight(root_); }
    
    /// 최소값 (정렬 순서 기준 첫 번째 = 1등)
    const T* minimum() const {
        if (!root_) return nullptr;
        Node* node = root_;
        while (node->left) node = node->left;
        return &node->data;
    }
    
    /// 최대값 (정렬 순서 기준 마지막)
    const T* maximum() const {
        if (!root_) return nullptr;
        Node* node = root_;
        while (node->right) node = node->right;
        return &node->data;
    }
    
    // ========================================================================
    // Order Statistics 연산
    // ========================================================================
    
    /**
     * @brief k번째 요소 조회 (1-based)
     * @param k 순위 (1 = 첫 번째)
     * @return k번째 요소 포인터, 없으면 nullptr
     * 
     * 시간복잡도: O(log n)
     */
    const T* kthElement(size_type k) const {
        if (k == 0 || k > size_) return nullptr;
        return kthElementHelper(root_, k);
    }
    
    /**
     * @brief 요소의 순위 조회
     * @param value 조회할 요소
     * @return 순위 (1-based), 없으면 0
     * 
     * 시간복잡도: O(log n)
     */
    size_type getRank(const T& value) const {
        return getRankHelper(root_, value, 0);
    }
    
    /**
     * @brief ID로 요소의 순위 조회
     * @param id 조회할 요소의 ID
     * @return 순위 (1-based), 없으면 0
     */
    size_type getRankById(const std::string& id) const {
        auto it = nodeMap_.find(id);
        if (it == nodeMap_.end()) return 0;
        return getRank(it->second->data);
    }
    
    // ========================================================================
    // Top-K 결과 반환
    // ========================================================================
    
    /// 정렬된 벡터 반환 (중위 순회)
    std::vector<T> toSortedVector() const {
        std::vector<T> result;
        result.reserve(size_);
        inorderTraversal(root_, result);
        return result;
    }
    
    /// 상위 K개 반환
    std::vector<T> topK(size_type k) const {
        std::vector<T> result;
        result.reserve(std::min(k, size_));
        inorderTopK(root_, result, k);
        return result;
    }
    
    // ========================================================================
    // 순회 (콜백 기반)
    // ========================================================================
    
    /// 중위 순회 (정렬 순서)
    template <typename Func>
    void inorder(Func&& func) const {
        inorderVisit(root_, std::forward<Func>(func));
    }

private:
    Node* root_;
    size_type size_;
    Compare comp_;
    KeyExtractor keyExtractor_;
    std::unordered_map<std::string, Node*> nodeMap_;  // ID → 노드 포인터
    
    // ========================================================================
    // 유틸리티 함수
    // ========================================================================
    
    int getHeight(Node* node) const {
        return node ? node->height : 0;
    }
    
    int getSubtreeSize(Node* node) const {
        return node ? node->subtreeSize : 0;
    }
    
    int getBalance(Node* node) const {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }
    
    void updateNode(Node* node) {
        if (node) {
            node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
            node->subtreeSize = 1 + getSubtreeSize(node->left) + getSubtreeSize(node->right);
        }
    }
    
    // ========================================================================
    // 회전 연산
    // ========================================================================
    
    Node* rightRotate(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;
        
        x->right = y;
        y->left = T2;
        
        updateNode(y);
        updateNode(x);
        
        return x;
    }
    
    Node* leftRotate(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;
        
        y->left = x;
        x->right = T2;
        
        updateNode(x);
        updateNode(y);
        
        return y;
    }
    
    // ========================================================================
    // 균형 조정
    // ========================================================================
    
    Node* balance(Node* node) {
        updateNode(node);
        int balanceFactor = getBalance(node);
        
        // Left Heavy
        if (balanceFactor > 1) {
            if (getBalance(node->left) < 0) {
                node->left = leftRotate(node->left);
            }
            return rightRotate(node);
        }
        
        // Right Heavy
        if (balanceFactor < -1) {
            if (getBalance(node->right) > 0) {
                node->right = rightRotate(node->right);
            }
            return leftRotate(node);
        }
        
        return node;
    }
    
    // ========================================================================
    // 삽입
    // ========================================================================
    
    Node* insertNode(Node* node, const T& value) {
        if (!node) {
            Node* newNode = new Node(value);
            if (keyExtractor_) {
                nodeMap_[keyExtractor_(value)] = newNode;
            }
            return newNode;
        }
        
        if (comp_(value, node->data)) {
            node->left = insertNode(node->left, value);
        } else {
            node->right = insertNode(node->right, value);
        }
        
        return balance(node);
    }
    
    Node* insertNode(Node* node, T&& value) {
        if (!node) {
            Node* newNode = new Node(std::move(value));
            if (keyExtractor_) {
                nodeMap_[keyExtractor_(newNode->data)] = newNode;
            }
            return newNode;
        }
        
        if (comp_(value, node->data)) {
            node->left = insertNode(node->left, std::move(value));
        } else {
            node->right = insertNode(node->right, std::move(value));
        }
        
        return balance(node);
    }
    
    // ========================================================================
    // 삭제
    // ========================================================================
    
    Node* findMin(Node* node) const {
        while (node && node->left) node = node->left;
        return node;
    }
    
    Node* removeNode(Node* node, const T& value) {
        if (!node) return nullptr;
        
        if (comp_(value, node->data)) {
            node->left = removeNode(node->left, value);
        } else if (comp_(node->data, value)) {
            node->right = removeNode(node->right, value);
        } else {
            // 삭제할 노드 찾음
            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                delete node;
                return temp;
            }
            
            // 두 자식이 있는 경우: 후계자로 대체
            Node* successor = findMin(node->right);
            
            // 후계자의 ID 맵 업데이트
            if (keyExtractor_) {
                std::string oldId = keyExtractor_(node->data);
                std::string successorId = keyExtractor_(successor->data);
                nodeMap_.erase(oldId);
                // 현재 노드가 후계자의 데이터를 갖게 됨
                nodeMap_[successorId] = node;
            }
            
            node->data = successor->data;
            node->right = removeNode(node->right, successor->data);
        }
        
        return balance(node);
    }
    
    // ========================================================================
    // 검색
    // ========================================================================
    
    Node* findNode(Node* node, const T& value) const {
        if (!node) return nullptr;
        
        if (comp_(value, node->data)) {
            return findNode(node->left, value);
        } else if (comp_(node->data, value)) {
            return findNode(node->right, value);
        }
        return node;
    }
    
    // ========================================================================
    // Order Statistics 헬퍼
    // ========================================================================
    
    const T* kthElementHelper(Node* node, size_type k) const {
        if (!node) return nullptr;
        
        int leftSize = getSubtreeSize(node->left);
        
        if (k <= static_cast<size_type>(leftSize)) {
            // k번째 요소가 왼쪽 서브트리에 있음
            return kthElementHelper(node->left, k);
        } else if (k == static_cast<size_type>(leftSize) + 1) {
            // 현재 노드가 k번째
            return &node->data;
        } else {
            // k번째 요소가 오른쪽 서브트리에 있음
            return kthElementHelper(node->right, k - leftSize - 1);
        }
    }
    
    size_type getRankHelper(Node* node, const T& value, size_type accum) const {
        if (!node) return 0;  // 못 찾음
        
        int leftSize = getSubtreeSize(node->left);
        
        if (comp_(value, node->data)) {
            // 왼쪽 서브트리로
            return getRankHelper(node->left, value, accum);
        } else if (comp_(node->data, value)) {
            // 오른쪽 서브트리로, 왼쪽 서브트리 + 현재 노드 개수 누적
            return getRankHelper(node->right, value, accum + leftSize + 1);
        } else {
            // 찾음: 누적 + 왼쪽 서브트리 크기 + 1
            return accum + leftSize + 1;
        }
    }
    
    // ========================================================================
    // 순회
    // ========================================================================
    
    void inorderTraversal(Node* node, std::vector<T>& result) const {
        if (!node) return;
        inorderTraversal(node->left, result);
        result.push_back(node->data);
        inorderTraversal(node->right, result);
    }
    
    void inorderTopK(Node* node, std::vector<T>& result, size_type k) const {
        if (!node || result.size() >= k) return;
        inorderTopK(node->left, result, k);
        if (result.size() < k) {
            result.push_back(node->data);
        }
        if (result.size() < k) {
            inorderTopK(node->right, result, k);
        }
    }
    
    template <typename Func>
    void inorderVisit(Node* node, Func&& func) const {
        if (!node) return;
        inorderVisit(node->left, func);
        func(node->data);
        inorderVisit(node->right, func);
    }
    
    // ========================================================================
    // 복사/삭제
    // ========================================================================
    
    void copyFrom(Node* node) {
        if (!node) return;
        insert(node->data);
        copyFrom(node->left);
        copyFrom(node->right);
    }
    
    void destroyTree(Node* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
};

} // namespace DataStructure

// ============================================================================
// 전역 네임스페이스에서 사용 가능하도록
// ============================================================================
using DataStructure::AVLTree;
using DataStructure::MaxAVLTree;
using DataStructure::MinAVLTree;
using DataStructure::RankAVLTree;

