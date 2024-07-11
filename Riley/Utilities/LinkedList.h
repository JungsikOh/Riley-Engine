#pragma once
#ifndef RILEY_LINKED_LIST
#define RILEY_LINKED_LIST

#include <type_traits>
#include <vector>

// this could be usefull for class pointers.
// not suitable for structures such as vectors

template <class T> class LinkedList {
  public:
    typedef void (*IterateFunc)(T*);

    template <class UserClass> struct ClassIterator {
        typedef void (*_Func)(UserClass*, T*);
        UserClass* userClass;
        _Func func;
        inline ClassIterator(UserClass* _class, _Func _func)
            : userClass(_class), func(_func) {}
        inline void Invoke(T* data) { func(userClass, data); }
    };

    struct Node {
        T* data;
        Node* next;
        Node(T* _data, Node* _next) : data(_data), next(_next) {}
    };

    Node *rootNode, *endNode;
    int nodeCount;

    LinkedList() : rootNode(nullptr), endNode(nullptr), nodeCount(0) {}

    LinkedList(T* firstClass)
        : rootNode(new Node(firstClass, nullptr)), nodeCount(1) {
        endNode = rootNode;
    }

    LinkedList(Node* _rootNode)
        : rootNode(_rootNode), endNode(_rootNode), nodeCount(1) {}

    /**
     *@brief  Converts to linkedlist from std::vector
     */
    LinkedList(const std::vector<T*>& vector) : nodeCount(vector.size()) {
        for (int i = 0; i < vector.size(); ++i) {
            AddFront(vector[i]);
        }
    }

    ~LinkedList() { IterateRecDestroy(rootNode); }

    T* operator[](int index) const {
        int startIndex = 0;
        return FindDataByIndexRec(rootNode, index, startIndex);
    }

#define _Template                                                              \
    template <typename Derived,                                                \
              typename std::enable_if<std::is_base_of<T, Derived>{} ||         \
                                          std::is_same<T, Derived>{},          \
                                      bool>::type = true>

    _Template void AddFront(Derived* data) {
        Node* newNode = new Node(dynamic_cast<T*>(data), nullptr);
        if (endNode) {
            endNode->next = newNode;
        } else {
            endNode = newNode;
            rootNode = newNode;
        }
        endNode = newNode;
        nodeCount++;
    }

    /**
     * @brief Sets data as first node(root node)
     */
    _Template void AddBack(Derived* data) {
        Node* oldRoot = rootNode;
        rootNode = new Node(dynamic_cast<T*>(data), oldRoot);
        nodeCount++;
    }

    /**
     * @brief Remove the data Ptr in LinkedList
     */
    _Template void Remove(Derived* ptr) {
        Node* currentNode = rootNode;

        // rootNode의 data가 ptr과 같으면, rootNode를 next에 있는걸로 임명하고
        // 현재 node는 삭제한다.
        if (rootNode->data == ptr) {
            rootNode = rootNode->next;
            delete currentNode;
            currentNode = rootNode;
            return;
        }

        do {
            if (currentNode->next->data == ptr) {
                Node* removedNode = currentNode->next;
                currentNode->next = removedNode->next;
                delete removedNode;
                break;
            }
            currentNode = currentNode->next;
        } while (currentNode->next != nullptr);
    }

    _Template bool TryGetData(Derived** component) {
        *component = FindNodeByType<Derived>();
        return component[0];
    }

#undef _Template // #define으로 만든 이름을 정의 해제한다.

    // returns removed endNode data
    T* RemoveFront() {
        if (nodeCount == 0)
            return nullptr;

        T* oldEndNodeData = endNode->data;
        Node* oldNode = endNode;
        --nodeCount;
        endNode = FindeSecondEndNodeRec(rootNode); // new endNode
        endNode->next = nullptr;
        delete oldNode;

        return oldEndNodeData;
    }

    // returns removed rootNode data
    T* RemoveBack() {
        if (!rootNode)
            return nullptr;

        T* oldRootNodeData = rootNode->data;
        Node* oldNode = rootNode;
        --nodeCount;
        rootNode = rootNode->next;
        delete oldNode;
        return oldRootNodeData;
    }

    template <class Desired> Desired* FindNodeByType() const {
        return FindNodeByTypeRec<Desired>(rootNode);
    }

    T* FindeNodeFromPtr(T* ptr) const {
        Node* currentNode = rootNode;
        if (currentNode->data == ptr) {
            return currentNode->data;
        }

        while (currentNode->next != nullptr) {
            currentNode = currentNode->next;
            if (currentNode->data == ptr) {
                return currentNode->data;
            }
        }
        return nullptr;
    }

    void Iterate(IterateFunc func) const {
        Node* currentNode = rootNode;
        func(currentNode->data);

        while (currentNode->next != nullptr) {
            currentNode = currentNode->next;
            func(currentNode->data);
        }
    }

    template <class UserClass>
    void IterateClass(ClassIterator<UserClass> iterator) const {
        Node* currentNode = rootNode;
        iterator.Invoke(currentNode->data);

        while (currentNode->next != nullptr) {
            currentNode = currentNode->next;
            iterator.Invoke(currentNode->data);
        }
    }

  private:
    bool HasNodeFindByPtrRec(Node* node, T* data) {
        if (node->next == nullptr)
            return false;
        if (node->data == data)
            return true;
        return HasNodeFindByPtrRec(node->next);
    }

    // finds this[nodeCount-2] not last one but one before lastNode
    Node* FindSecondEndNodeRec(Node* node) {
        if (node->next == nullptr)
            return nullptr;
        if (node->next->next == nullptr)
            return node;
        return FindSecondEndNodeRec(node->next);
    }

    Node* FindNodeByIndexRec(Node* node, int targetIndex, int& index) const {
        if (index == targetIndex || node->next == nullptr)
            return node;
        return FindNodeByIndexRec(node->next, targetIndex, ++index);
    }

    T* FindDataByIndexRec(Node* node, int targetIndex, int& index) const {
        if (index == targetIndex || node->next == nullptr)
            return node->data;
        return FindDataByIndexRec(node->next, targetIndex, ++index);
    }

    template <class Desired> Desired* FindNodeByTypeRec(Node* node) const {
        if (node->data != nullptr && dynamic_cast<Desired*>(node->data))
            return dynamic_cast<Desired*>(node->data);

        if (node->next != nullptr)
            return FindNodeByTypeRec<Desired>(node->next);

        return nullptr;
    }

    void IterateRecDestroy(Node* node) {
        if (!node)
            return;
        if (node->next != nullptr)
            IterateRecDestroy(node->next);
        node->data->~T();
        free(node->data);
        free(node);
    }
};

#endif // RILEY_LINKED_LIST