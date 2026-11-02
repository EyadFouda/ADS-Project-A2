#include "CircularQueue.h"

// Creates the fixed array and starts with an empty queue.
CircularQueue::CircularQueue(int capacity) {
    // Ensures the capacity is not 0
    if (capacity <= 0) {
        capacity = 10;
    }

    this->capacity = capacity;
    head = 0;
    count = 0;
    data = new string[capacity];
}

// Destructor
CircularQueue::~CircularQueue() {
    delete[] data;
}

// Adds the item at the first free slot after the newest item.
// The modulus operator makes sure the index cant be out of range.
void CircularQueue::enqueue(const string& item) {
    if (count == capacity) {
        // Checks if the queue is full and moves the head forward so the next oldest item becomes the front.
        data[head] = item;
        head = (head + 1) % capacity;
    }
    else {
        int tail = (head + count) % capacity;
        data[tail] = item;
        count++;
    }
}

// Returns the oldest item and removes it.
bool CircularQueue::dequeue(string& item) {
    if (isEmpty()) {
        return false;
    }

    item = data[head];
    data[head] = "";  // Remove the data.
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

// Gets the newest item just before the first free slot.
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

// Check if the queue is empty.
bool CircularQueue::isEmpty() const {
    return count == 0;
}

// Checks if the queue is full.
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

// Resets the queue and resets the stored strings to empty strings.
void CircularQueue::clear() {
    for (int i = 0; i < capacity; i++) {
        data[i] = "";
    }

    head = 0;
    count = 0;
}
