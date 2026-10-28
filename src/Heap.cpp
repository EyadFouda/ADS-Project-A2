#include "Heap.h"
#include <iostream>
using namespace std;

// Constructor allocates dynamic array
Heap::Heap(int initial_capacity){
	if(initial_capacity == 0)
		initial_capacity = 1; // Zero capacity may cause errors when resizing so make sure it can't be the capacity
	capacity = initial_capacity;
        size = 0;
        data = new HeapNode[capacity];
}

// Destructor frees up memory and prevents leaks
Heap::~Heap(){
        delete[] data;
}

// Resize doubles array capacity when full
void Heap::resize(){
        capacity *= 2;
        HeapNode* newData = new HeapNode[capacity];

	// Copying elements into a larger heap
        for(int i = 0; i < size; i++){
                newData[i] = data[i];
        }

        delete[] data; // Frees old memeory
        data = newData; // Pointer points to the new array
}

// Heapify up moves element up to fix order after insertion
void Heap::heapifyUp(int index){
        while(index > 0){
                int parent = (index - 1) / 2; // Parent index formula based on binary tree

		// Swap child with parent if the child has greater frequency
		// Note that equal frequencies won't trigger a swap and likewise with heapifyDown
                if(data[index].frequency > data[parent].frequency){
                        HeapNode temp = data[index];
                        data[index] = data[parent];
                        data[parent] = temp;
                        index = parent; // Move to the parents index
                } else{
                        break; // Max heap property satisfied
                }
        }
}

// Heapify down moves element down to fix order after extraction
void Heap::heapifyDown(int index){
        while(true){
                int left = 2 * index + 1; // Left child index formula
                int right = 2 * index + 2; // Right child index formula
                int largest = index;

		// Check if left child exists and is more frequent than the current largest
                if(left < size && data[left].frequency > data[largest].frequency){
                        largest = left;
                }

		// Check if right child exists and is more frequent than the current largest
                if(right < size && data[right].frequency > data[largest].frequency){
                        largest = right;
                }

		// If the largest isnt the current node then swap and continue
                if(largest != index){
                        HeapNode temp = data[index];
                        data[index] = data[largest];
                        data[largest] = temp;
                        index = largest;
                } else{
                        break; // Max heap property satisfied
                }
        }
}

// Insertion into heap
void Heap::insert(const string& word, int frequency){
        if(size >= capacity){
                resize(); // Double capacity if array is full
        }
        data[size] = {word, frequency}; // Place new element at the end
        heapifyUp(size); // Move upwards to resatisfy max heap property
        size++;
}

// Extraction of highest priority element
HeapNode Heap::extractMax(){
        if(isEmpty()){
                return {"", -1}; // Return empty node if heap is empty, an empty string and a frequency of -1 which is impossible so it is a good signal to use for errors
        }
        HeapNode maxNode = data[0];
        data[0] = data[size - 1]; // Take the last element as the root
        size--; // Reduce size
        heapifyDown(0); // Move the root (the last element) downwards until max heap property is satisfied
        return maxNode;
}

// Check if heap is empty
bool Heap::isEmpty() const{
        return size == 0;
}

// Get size of the heap
int Heap::getSize() const{
        return size;
}


