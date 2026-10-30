
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>

using namespace std;

class HashTable {
private:
    struct Node {
        string word;
        int frequency;
        Node* next;

        Node(const string& word, int frequency);
    };

    Node** table;
    int capacity;
    int size;

    // Calculates the array index for a given word.
    int hashFunction(const string& word, int tableCapacity) const;

    // Checks whether a word contains only lowercase letters.
    bool isValidWord(const string& word) const;

    // Doubles the table capacity and redistributes its nodes.
    void resize();

public:
    // Creates an empty hash table with the given initial capacity.
    HashTable(int initialCapacity = 101);

    // Releases all dynamically allocated nodes and the table.
    ~HashTable();

    // Inserts a word or updates its frequency if it already exists.
    void insert(const string& word, int frequency);

    // Searches for a word and stores its frequency through a reference.
    bool search(const string& word, int& frequency) const;

    // Checks whether a word exists in the hash table.
    bool contains(const string& word) const;

    // Removes a word and returns whether the removal succeeded.
    bool remove(const string& word);

    // Returns the number of unique words stored.
    int getSize() const;

    // Returns the current number of buckets.
    int getCapacity() const;

    // Returns the ratio of stored words to available buckets.
    double getLoadFactor() const;
};

#endif
