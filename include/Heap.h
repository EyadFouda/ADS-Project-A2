#ifndef HEAP_H
#define HEAP_H

#include <string>
using namespace std;

// Single heap node struct holding word and rank
struct HeapNode{
	string word;
	int frequency;
};

// Binary Max-Heap for ranking and sorting suggestions
class Heap{
private:
	HeapNode* data; // Dynamic array pointer
	int capacity; // Total allocated memory size
	int size; // Current number of elements
	
	void resize(); // Doubles array capacity when full
	void heapifyUp(int index); // Moves element up to fix order after insertion
	void heapifyDown(int index); // Moves element down to fix order after extraction

public:
	Heap(int initial_capacity = 100); // Constructor
	~Heap(); // Destructor
	void insert(const string& word, int frequency); // Insertion
	HeapNode extractMax(); // Extracts highest priority item and returns it
	bool isEmpty() const; // Checks if heap is empty
	int getSize() const; // Gets size of the heap
};

#endif
