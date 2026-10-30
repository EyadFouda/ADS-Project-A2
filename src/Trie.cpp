
#include "Trie.h"

// Initializes an empty Trie node with no children or completions.
TrieNode::TrieNode() {
    isEndOfWord = false;
    frequency = 0;
    topKSize = 0;

    for (int i = 0; i < 26; i++) {
        children[i] = nullptr;
    }
}

// Creates the Trie and initializes its root node.
Trie::Trie() {
    root = new TrieNode();
}

// Converts a lowercase character into its corresponding array index.
int Trie::getIndex(char c) const {
    if (c < 'a' || c > 'z') {
        return -1;
    }

    return c - 'a';
}

// Checks whether a word contains only lowercase English letters.
bool Trie::isValidWord(const string& word) const {
    if (word.empty()) {
        return false;
    }

    for (int i = 0; i < word.length(); i++) {
        if (getIndex(word[i]) == -1) {
            return false;
        }
    }

    return true;
}

// Inserts a word and updates its cached top-five completions along its path.
void Trie::insert(const string& word, int frequency) {
    if (!isValidWord(word) || frequency <= 0) {
        return;
    }

    TrieNode* current = root;

    for (int i = 0; i < word.length(); i++) {
        int index = getIndex(word[i]);

        if (current->children[index] == nullptr) {
            current->children[index] = new TrieNode();
        }

        current = current->children[index];
    }

    if (!current->isEndOfWord) {
        current->isEndOfWord = true;
        current->frequency = frequency;
    }
    else if (frequency > current->frequency) {
        current->frequency = frequency;
    }

    int actualFrequency = current->frequency;

    current = root;

    updateTopK(current, word, actualFrequency);

    for (int i = 0; i < word.length(); i++) {
        int index = getIndex(word[i]);
        current = current->children[index];
        updateTopK(current, word, actualFrequency);
    }
}

// Checks whether a complete word exists in the Trie.
bool Trie::search(const string& word) const {
    if (!isValidWord(word)) {
        return false;
    }

    const TrieNode* current = root;

    for (int i = 0; i < word.length(); i++) {
        int index = getIndex(word[i]);

        if (current->children[index] == nullptr) {
            return false;
        }

        current = current->children[index];
    }

    return current->isEndOfWord;
}

// Checks whether the Trie contains any word starting with the given prefix.
bool Trie::startsWith(const string& prefix) const {
    if (prefix.empty()) {
        return true;
    }

    const TrieNode* current = root;

    for (int i = 0; i < prefix.length(); i++) {
        int index = getIndex(prefix[i]);

        if (index == -1 || current->children[index] == nullptr) {
            return false;
        }

        current = current->children[index];
    }

    return true;
}

// Maintains the five highest-frequency completions in descending order.
void Trie::updateTopK(
    TrieNode* node,
    const string& word,
    int frequency
) {
    int position = 0;

    while (position < node->topKSize) {
        if (node->topK[position].word == word) {
            if (frequency > node->topK[position].frequency) {
                node->topK[position].frequency = frequency;

                while (position > 0 &&
                       node->topK[position].frequency >
                       node->topK[position - 1].frequency) {

                    Completion temp = node->topK[position];
                    node->topK[position] = node->topK[position - 1];
                    node->topK[position - 1] = temp;

                    position--;
                }
            }

            return;
        }

        position++;
    }

    position = 0;

    while (position < node->topKSize &&
           node->topK[position].frequency >= frequency) {
        position++;
    }

    if (position >= 5) {
        return;
    }

    if (node->topKSize < 5) {
        node->topKSize++;
    }

    for (int i = node->topKSize - 1; i > position; i--) {
        node->topK[i] = node->topK[i - 1];
    }

    node->topK[position].word = word;
    node->topK[position].frequency = frequency;
}

// Finds the prefix node and copies its highest-ranked completions into results.
int Trie::autocomplete(
    const string& prefix,
    Completion results[],
    int maxResults
) const {
    if (maxResults <= 0) {
        return 0;
    }

    const TrieNode* current = root;

    for (int i = 0; i < prefix.length(); i++) {
        int index = getIndex(prefix[i]);

        if (index == -1 || current->children[index] == nullptr) {
            return 0;
        }

        current = current->children[index];
    }

    int count = current->topKSize;

    if (count > maxResults) {
        count = maxResults;
    }

    for (int i = 0; i < count; i++) {
        results[i] = current->topK[i];
    }

    return count;
}

// Recursively removes a word and identifies nodes that can safely be deleted.
bool Trie::deleteHelper(
    TrieNode* node,
    const string& word,
    int depth
) {
    if (node == nullptr) {
        return false;
    }

    if (depth == word.length()) {
        if (!node->isEndOfWord) {
            return false;
        }

        node->isEndOfWord = false;

        for (int i = 0; i < 26; i++) {
            if (node->children[i] != nullptr) {
                return false;
            }
        }

        return true;
    }

    int index = getIndex(word[depth]);

    if (index == -1 || node->children[index] == nullptr) {
        return false;
    }

    bool shouldDeleteChild = deleteHelper(
        node->children[index],
        word,
        depth + 1
    );

    if (shouldDeleteChild) {
        delete node->children[index];
        node->children[index] = nullptr;
    }

    if (node->isEndOfWord) {
        return false;
    }

    for (int i = 0; i < 26; i++) {
        if (node->children[i] != nullptr) {
            return false;
        }
    }

    return true;
}

// Recalculates cached top-five completions for a node and its descendants.
void Trie::rebuildTopK(TrieNode* node, string& prefix) {
    if (node == nullptr) {
        return;
    }

    node->topKSize = 0;

    if (node->isEndOfWord) {
        updateTopK(node, prefix, node->frequency);
    }

    for (int i = 0; i < 26; i++) {
        if (node->children[i] != nullptr) {
            prefix += char('a' + i);

            rebuildTopK(node->children[i], prefix);

            for (int j = 0; j < node->children[i]->topKSize; j++) {
                updateTopK(
                    node,
                    node->children[i]->topK[j].word,
                    node->children[i]->topK[j].frequency
                );
            }

            prefix.pop_back();
        }
    }
}

// Removes a word and rebuilds cached completions to reflect the deletion.
void Trie::remove(const string& word) {
    if (!isValidWord(word)) {
        return;
    }

    deleteHelper(root, word, 0);

    string prefix = "";
    rebuildTopK(root, prefix);
}

// Recursively releases all dynamically allocated nodes in the Trie.
void Trie::destroyTrie(TrieNode* node) {
    if (node == nullptr) {
        return;
    }

    for (int i = 0; i < 26; i++) {
        destroyTrie(node->children[i]);
    }

    delete node;
}

// Releases the entire Trie when the object is destroyed.
Trie::~Trie() {
    destroyTrie(root);
}