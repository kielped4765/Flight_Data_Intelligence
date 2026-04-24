#include <gtest/gtest.h>
#include "ring_buffer.h"

// -- Test 1: Basic push and pop -----------------
TEST(RingBufferTest, PushPopSingleItem) {
    RingBuffer<int, 4> buf;
    EXPECT_TRUE(buf.empty());
    buf.push(42);
    EXPECT_EQ(buf.size(), 1u);
    EXPECT_FALSE(buf.empty());
    EXPECT_EQ(buf.pop(), 42);
    EXPECT_TRUE(buf.empty());
}

// --- Test 2: FIFO ordering --------------------
TEST(RingBufferTest, FIFOORDER) {
    RingBuffer<int, 8> buf;
    buf.push(1); buf.push(2); buf.push(3);
    EXPECT_EQ(buf.pop(), 1);    // Oldest first
    EXPECT_EQ(buf.pop(), 2);
    EXPECT_EQ(buf.pop(), 3);
}

// --- Test 3: Overflow wraps and overwrites oldest -----------
TEST(RingBufferTest, OverflowOverwriteOldest) {
    RingBuffer<int, 3> buf;
    buf.push(1);    // [1, _, _]
    buf.push(2); // [1, 2, _]
    buf.push(3); // [1, 2, 3] <- full
    buf.push(4); // [4, 2, 3] <- 1 overwritten
    EXPECT_EQ(buf.size(), 3u);
    EXPECT_EQ(buf.pop(), 2); // 1 was lost
    EXPECT_EQ(buf.pop(), 3);
    EXPECT_EQ(buf.pop(), 4);
}

// -- Test 4: Pop on empty throws --------------------
TEST(RingBufferTest, PopOnEmptyThrows) {
    RingBuffer<int, 4> buf;
    EXPECT_THROW(buf.pop(), std::underflow_error);
}

// Test 5: Capacity is correct -----------------------
TEST(RingBufferTest, CapacityReported) {
    RingBuffer<float, 16> buf;
    EXPECT_EQ(buf.capacity(), 16u);
}