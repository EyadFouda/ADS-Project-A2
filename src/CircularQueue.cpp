#include "CircularQueue.h"

// Allocates the fixed array and starts with an empty queue.
CircularQueue::CircularQueue(int capacity) {
    // A capacity of zero or less cannot hold anything, so fall back to 10
    if (capacity <= 0) {
        capacity = 10;
    }

    this->capacity = capacity;
    head = 0;
    count = 0;
    data = new string[capacity];
}

// Frees the array to prevent memory leaks.
CircularQueue::~CircularQueue() {
    delete[] data;
}

// Writes the item at the first free slot after the newest item.
// The modulo operator wraps the index back to 0 at the end of the array.
void CircularQueue::enqueue(const string& item) {
    if (count == capacity) {
        // Full: the slot at head is the oldest item, so overwrite it
        // and move head forward so the next-oldest item becomes the front
        data[head] = item;
        head = (head + 1) % capacity;
    }
    else {
        int tail = (head + count) % capacity;
        data[tail] = item;
        count++;
    }
}

// Returns the oldest item through the reference and removes it.
bool CircularQueue::dequeue(string& item) {
    if (isEmpty()) {
        return false;
    }

    item = data[head];
    data[head] = "";  // Release the string's memory
    head = (head + 1) % capacity;
    count--;

    return true;
}

// The oldest item sits at head.
bool CircularQueue::front(string& item) const {
    if (isEmpty()) {
        return false;
    }

    item = data[head];
    return true;
}

// The newest item sits just before the first free slot.
bool CircularQueue::rear(string& item) const {
    if (isEmpty()) {
        return false;
    }

    item = data[(head + count - 1) % capacity];
    return true;
}

// Counts backwards from the newest item by the given offset.
// head + count - 1 - offset is never negative because offset < count.
bool CircularQueue::getFromNewest(int offset, string& item) const {
    if (offset < 0 || offset >= count) {
        return false;
    }

    item = data[(head + count - 1 - offset) % capacity];
    return true;
}

// The queue is empty when it holds no items.
bool CircularQueue::isEmpty() const {
    return count == 0;
}

// The queue is full when every slot is used.
bool CircularQueue::isFull() const {
    return count == capacity;
}

// Returns the current number of items.
int CircularQueue::getSize() const {
    return count;
}

// Returns the fixed capacity.
int CircularQueue::getCapacity() const {
    return capacity;
}

// Resets the queue and releases the stored strings.
void CircularQueue::clear() {
    for (int i = 0; i < capacity; i++) {
        data[i] = "";
    }

    head = 0;
    count = 0;
}
