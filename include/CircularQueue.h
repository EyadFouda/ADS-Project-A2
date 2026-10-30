#ifndef CIRCULARQUEUE_H
#define CIRCULARQUEUE_H

#include <string>

using namespace std;

// Fixed-size circular queue of strings, used to store search history.
// When the queue is full, enqueue overwrites the oldest entry.
class CircularQueue {
private:
    string* data;   // Dynamic array holding the entries
    int capacity;   // Maximum number of entries
    int head;       // Index of the oldest entry
    int count;      // Current number of entries

    CircularQueue(const CircularQueue&) = delete;
    CircularQueue& operator=(const CircularQueue&) = delete;

public:
    // Creates an empty queue with the given capacity (default 10).
    CircularQueue(int capacity = 10);

    // Releases the dynamic array.
    ~CircularQueue();

    // Adds an item at the back, overwriting the oldest item if full.
    void enqueue(const string& item);

    // Removes the oldest item and returns whether the removal succeeded.
    bool dequeue(string& item);

    // Reads the oldest item without removing it.
    bool front(string& item) const;

    // Reads the newest item without removing it.
    bool rear(string& item) const;

    // Reads an item by age: offset 0 is the newest, 1 is the one before it.
    // Used for up/down arrow navigation through history.
    bool getFromNewest(int offset, string& item) const;

    // Checks whether the queue has no items.
    bool isEmpty() const;

    // Checks whether the queue has reached its capacity.
    bool isFull() const;

    // Returns the current number of items.
    int getSize() const;

    // Returns the maximum number of items.
    int getCapacity() const;

    // Removes all items.
    void clear();
};

#endif

