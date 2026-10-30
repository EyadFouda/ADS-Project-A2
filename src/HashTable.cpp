
#include "HashTable.h"

// Initializes a node with a word, frequency, and null next pointer.
HashTable::Node::Node(const string& word, int frequency) {
    this->word = word;
    this->frequency = frequency;
    next = nullptr;
}

// Creates the bucket array and initializes every bucket as empty.
HashTable::HashTable(int initialCapacity) {
    if (initialCapacity <= 0) {
        initialCapacity = 101;
    }

    capacity = initialCapacity;
    size = 0;

    table = new Node*[capacity];

    for (int i = 0; i < capacity; i++) {
        table[i] = nullptr;
    }
}

// Calculates a hash value using each character and the multiplier 31.
int HashTable::hashFunction(
    const string& word,
    int tableCapacity
) const {
    unsigned long long hash = 0;

    for (int i = 0; i < word.length(); i++) {
        hash = (hash * 31 + word[i]) % tableCapacity;
    }

    return static_cast<int>(hash);
}

// Rejects empty words and words containing characters outside a-z.
bool HashTable::isValidWord(const string& word) const {
    if (word.empty()) {
        return false;
    }

    for (int i = 0; i < word.length(); i++) {
        if (word[i] < 'a' || word[i] > 'z') {
            return false;
        }
    }

    return true;
}

// Inserts a word into its bucket or updates an existing word's frequency.
void HashTable::insert(const string& word, int frequency) {
    if (!isValidWord(word) || frequency <= 0) {
        return;
    }

    int index = hashFunction(word, capacity);

    Node* current = table[index];

    while (current != nullptr) {
        if (current->word == word) {
            if (frequency > current->frequency) {
                current->frequency = frequency;
            }

            return;
        }

        current = current->next;
    }

    Node* newNode = new Node(word, frequency);

    newNode->next = table[index];
    table[index] = newNode;

    size++;

    if (getLoadFactor() > 0.75) {
        resize();
    }
}

// Finds a word and returns its frequency through the reference parameter.
bool HashTable::search(
    const string& word,
    int& frequency
) const {
    if (!isValidWord(word)) {
        return false;
    }

    int index = hashFunction(word, capacity);

    Node* current = table[index];

    while (current != nullptr) {
        if (current->word == word) {
            frequency = current->frequency;
            return true;
        }

        current = current->next;
    }

    return false;
}

// Returns true if the word exists and false otherwise.
bool HashTable::contains(const string& word) const {
    int frequency;

    return search(word, frequency);
}

// Removes a word from its bucket using previous and current pointers.
bool HashTable::remove(const string& word) {
    if (!isValidWord(word)) {
        return false;
    }

    int index = hashFunction(word, capacity);

    Node* current = table[index];
    Node* previous = nullptr;

    while (current != nullptr) {
        if (current->word == word) {
            if (previous == nullptr) {
                table[index] = current->next;
            }
            else {
                previous->next = current->next;
            }

            delete current;
            size--;

            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

// Creates a larger bucket array and redistributes existing nodes.
void HashTable::resize() {
    int oldCapacity = capacity;
    Node** oldTable = table;

    capacity = capacity * 2 + 1;

    table = new Node*[capacity];

    for (int i = 0; i < capacity; i++) {
        table[i] = nullptr;
    }

    for (int i = 0; i < oldCapacity; i++) {
        Node* current = oldTable[i];

        while (current != nullptr) {
            Node* nextNode = current->next;

            int newIndex = hashFunction(current->word, capacity);

            current->next = table[newIndex];
            table[newIndex] = current;

            current = nextNode;
        }
    }

    delete[] oldTable;
}

// Returns the number of unique words in the hash table.
int HashTable::getSize() const {
    return size;
}

// Returns the current number of buckets in the array.
int HashTable::getCapacity() const {
    return capacity;
}

// Calculates how full the hash table currently is.
double HashTable::getLoadFactor() const {
    return static_cast<double>(size) / capacity;
}

// Deletes every linked-list node and releases the bucket array.
HashTable::~HashTable() {
    for (int i = 0; i < capacity; i++) {
        Node* current = table[i];

        while (current != nullptr) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }

    delete[] table;
}
