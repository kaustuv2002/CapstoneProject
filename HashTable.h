/*
 * File: HashTable.h
 * Project: Academic Information and Management System
 * Purpose: Defines a custom templated hash table with linked collision chains.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include <utility>
// Hash table implemented from scratch: each bucket is a custom singly linked chain.
// std::vector is only used as the fixed bucket array; collision handling is our own code.
// CLASS TEMPLATE + CUSTOM MAP/HASH TABLE ADT: supports storing different value types.
template <typename T>
class HashTable {
    // CUSTOM SINGLY LINKED LIST: each collision-chain node points to the next node.
    struct Node {
        int key;
        std::shared_ptr<T> value;
        std::unique_ptr<Node> next;
        Node(int k, std::shared_ptr<T> v) : key(k), value(std::move(v)) {}
    };
    std::vector<std::unique_ptr<Node>> buckets_;
    std::size_t size_ = 0;
    // Rebuild bucket chains when the table grows to maintain practical lookup speed.
    void rehash(std::size_t newCount) {
        std::vector<std::unique_ptr<Node>> next(newCount);
        for (auto& head : buckets_) while (head) {
            auto node = std::move(head);
            head = std::move(node->next);
            const std::size_t slot = static_cast<std::size_t>(node->key) % newCount;
            node->next = std::move(next[slot]);
            next[slot] = std::move(node);
        }
        buckets_ = std::move(next);
    }
    std::size_t index(int key) const { return static_cast<std::size_t>(key) % buckets_.size(); }
public:
    explicit HashTable(std::size_t bucketCount = 101) : buckets_(bucketCount ? bucketCount : 101) {}
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    bool insert(int key, std::shared_ptr<T> value) {
        if (find(key)) return false;
        if (size_ >= buckets_.size()) rehash(buckets_.size() * 2 + 1);
        auto node = std::make_unique<Node>(key, std::move(value));
        node->next = std::move(buckets_[index(key)]);
        buckets_[index(key)] = std::move(node);
        ++size_;
        return true;
    }
    std::shared_ptr<T> find(int key) const {
        for (const Node* p = buckets_[index(key)].get(); p; p = p->next.get())
            if (p->key == key) return p->value;
        return nullptr;
    }
    bool erase(int key) {
        auto* link = &buckets_[index(key)];
        while (*link) {
            if ((*link)->key == key) {
                auto removed = std::move(*link);
                *link = std::move(removed->next);
                --size_;
                return true;
            }
            link = &((*link)->next);
        }
        return false;
    }
    void clear() { for (auto& head : buckets_) head.reset(); size_ = 0; }
    std::size_t size() const { return size_; }
    template <typename F> void forEach(F callback) const {
        for (const auto& head : buckets_)
            for (const Node* p = head.get(); p; p = p->next.get()) callback(static_cast<const T&>(*p->value));
    }
};
