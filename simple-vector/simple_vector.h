#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <utility>

#include "array_ptr.h"

struct ReserveProxyObj {
    ReserveProxyObj(size_t c): capacity(c) {}
    size_t capacity = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;
    
    SimpleVector(ReserveProxyObj rpo) {
        Reserve(rpo.capacity);
    }

    explicit SimpleVector(size_t size) {
        ArrayPtr<Type> tmp(size);
        ptr_.swap(tmp);
        size_ = size;
        capacity_ = size;
    }
    
    SimpleVector(const SimpleVector& sv) {
        ArrayPtr<Type> tmp(sv.size_);
        ptr_.swap(tmp);
        std::copy(sv.begin(), sv.end(), ptr_.Get());
        size_ = sv.size_;
        capacity_ = sv.size_;
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        ArrayPtr<Type> tmp(rhs.size_);
        std::copy(rhs.begin(), rhs.end(), tmp.Get());
        ptr_.swap(tmp);
        size_ = rhs.size_;
        capacity_ = rhs.size_;
        return *this;
    }
    
    SimpleVector(SimpleVector&& sv) {
        ptr_.swap(sv.ptr_);
        size_ = std::exchange(sv.size_, 0);
        capacity_ = std::exchange(sv.size_, 0);
    }
    
    SimpleVector& operator=(SimpleVector&& rhs) {
        ptr_.swap(rhs.ptr_);
        size_ = std::exchange(rhs.size_, 0);
        capacity_ = std::exchange(rhs.size_, 0);
        return *this;
    }

    SimpleVector(size_t size, const Type& value) {
        ArrayPtr<Type> tmp(size);
        ptr_.swap(tmp);
        for(size_t i = 0; i < size; i++) {
            ptr_[i] = value;
        }
        size_ = size;
        capacity_ = size;
    }

    SimpleVector(std::initializer_list<Type> init) {
        ArrayPtr<Type> tmp(init.size());
        ptr_.swap(tmp);
        size_t i = 0;
        for(const Type& v: init) {
            ptr_[i] = v;
            i++;
        }
        size_ = init.size();
        capacity_ = init.size();
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }
    
    void PushBack(const Type& v) {
        if(size_ == capacity_) {
            Reserve((capacity_ == 0) ? 1 : (capacity_ * 2));
        }
        ptr_[size_++] = v;
    }
    
    void PushBack(Type&& v) {
        if(size_ == capacity_) {
            Reserve((capacity_ == 0) ? 1 : (capacity_ * 2));
        }
        ptr_[size_++] = std::move(v);
    }
    
    void PopBack() noexcept {
        assert(size_ != 0);
        Resize(size_ - 1);
    }

    Iterator Insert(ConstIterator pos, const Type& v) {
        assert(pos >= begin() && pos <= end());
        size_t pos_i = pos - begin();
        if(size_ == capacity_) {
            Reserve((capacity_ == 0) ? 1 : (capacity_ * 2));
        }
        size_++;
        Iterator it = std::copy_backward(begin() + pos_i, end() - 1, end()) - 1;
        *(it) = v;
        return it;
    }
    
    Iterator Insert(ConstIterator pos, Type&& v) {
        assert(pos >= begin() && pos <= end());
        size_t pos_i = pos - begin();
        if(size_ == capacity_) {
            Reserve((capacity_ == 0) ? 1 : (capacity_ * 2));
        }
        size_++;
        Iterator it = std::move_backward(begin() + pos_i, end() - 1, end()) - 1;
        *(it) = std::move(v);
        return it;
    }
    
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        std::move(const_cast<Iterator>(pos + 1), end(), const_cast<Iterator>(pos));
        size_--;
        return const_cast<Iterator>(pos);
    }
    
    void swap(SimpleVector& sv) noexcept {
        ptr_.swap(sv.ptr_);
        std::swap(size_, sv.size_);
        std::swap(capacity_, sv.capacity_);
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return ptr_[index];
    }

    Type& At(size_t index) {
        if(index >= size_) {
            throw std::out_of_range("Index >= size");
        }
        return ptr_[index];
    }

    const Type& At(size_t index) const {
        if(index >= size_) {
            throw std::out_of_range("Index >= size");
        }
        return ptr_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if(new_size > capacity_) {
            Reserve(new_size);
        }
        if(new_size > size_) {
            for(auto it = end(); it != ptr_.Get() + new_size; it++) {
                Type* tmp = new Type();
                *(it) = std::move(*tmp);
                delete tmp;
            }
        }
        size_ = new_size;
    }
    
    void Reserve(size_t new_capacity) {
        if(new_capacity > capacity_) {
            ArrayPtr<Type> copy(new_capacity);
            std::move(begin(), end(), copy.Get());
            ptr_.swap(copy);
            capacity_ = new_capacity;
        }
    }

    Iterator begin() noexcept {
        return ptr_.Get();
    }

    Iterator end() noexcept {
        return ptr_.Get() == nullptr ? nullptr : (ptr_.Get() + size_);
    }

    ConstIterator begin() const noexcept {
        return ptr_.Get();
    }

    ConstIterator end() const noexcept {
        return ptr_.Get() == nullptr ? nullptr : (ptr_.Get() + size_);
    }

    ConstIterator cbegin() const noexcept {
        return ptr_.Get();
    }

    ConstIterator cend() const noexcept {
        return ptr_.Get() + size_;
    }
    
private:
    ArrayPtr<Type> ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (&lhs == &rhs) || (lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type> rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}