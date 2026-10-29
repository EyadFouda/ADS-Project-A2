
#include "Trie.h"

TrieNode::TrieNode() {
    isEndOfWord = false;
    frequency = 0;
    topKSize = 0;

    for (int i = 0; i < 26; i++) {
        children[i] = nullptr;
    }
}

Trie::Trie() {
    root = new TrieNode();
}

int Trie::getIndex(char c) const {
    if (c < 'a' || c > 'z') {
        return -1;
    }

    return c - 'a';
}

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

void Trie::remove(const string& word) {
    if (!isValidWord(word)) {
        return;
    }

    deleteHelper(root, word, 0);

    string prefix = "";
    rebuildTopK(root, prefix);
}

void Trie::destroyTrie(TrieNode* node) {
    if (node == nullptr) {
        return;
    }

    for (int i = 0; i < 26; i++) {
        destroyTrie(node->children[i]);
    }

    delete node;
}

Trie::~Trie() {
    destroyTrie(root);
}
