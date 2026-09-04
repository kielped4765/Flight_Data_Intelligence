#pragma once
#include <array>
#include <stdexcept>
#include <cstddef>

// -- RingBuffer<T, N> ---------------------
//
// A fixed-capacity circular buffer that overwrites the oldest entry when full.
// This is 0(1) push AND pop wih zero heap allocation - the entire buffer lives on the stack.

// Template parameters:
// T = the type stored (e.g. TelemetryFrame)
// N = maximum number of elements (must be known at compile time)

template<typename T, size_t N>
class RingBuffer {
public:
    // -- Push an item into the buffer -------------------------
    // If full, overwrites the oldest item (head advances too).
    void push(const T& item) {
        buf_[head_] = item;
        head_ = (head_ + 1) % N;        // Wrap around at N
        if (size_ <N) {
            ++size_;
        } else {
            // Buffer was full: oldest item just got overwritten.
            // Advance tail so it still points at the oldest valid item.
            tail_ = (tail_ + 1) % N;
        }
    }

    // -- Pop the oldest item -----------------------------------
    // Throws std::underflow_error if empty.
    T pop() {
        if (size_ == 0) {
            throw std::underflow_error("RingBuffer: pop() on empty buffer");
        }
        T item = buf_[tail_];
        tail_ = (tail_ + 1) % N;
        --size_;
        return item;
    }

    // -- Query methods -----------------------------------------
    bool empty()        const { return size_ == 0;}
    size_t size()       const { return size_;}
    size_t capacity()   const { return N;}
    
private: 
    std::array<T, N> buf_;      // Fixed-rate array - lives on stack
    size_t head_ = 0;           // Where the NEXT push writes
    size_t tail_ = 0;           // Where the NEXT pop reads
    size_t size_ = 0;           // Current number of valid items
};
