
#ifndef TRIE_H
#define TRIE_H

#include <string>

using namespace std;

struct Completion {
    string word;
    int frequency;

    // Initializes a completion with a frequency of zero.
    Completion() {
        frequency = 0;
    }
};

struct TrieNode {
    TrieNode* children[26];
    bool isEndOfWord;
    int frequency;
    Completion topK[5];
    int topKSize;

    // Initializes an empty Trie node.
    TrieNode();
};

class Trie {
private:
    TrieNode* root;

    // Converts a lowercase character into an array index.
    int getIndex(char c) const;

    // Checks whether a word contains only lowercase letters.
    bool isValidWord(const string& word) const;

    // Recursively deletes all nodes in the Trie.
    void destroyTrie(TrieNode* node);

    // Recursively removes a word and deletes unnecessary nodes.
    bool deleteHelper(
        TrieNode* node,
        const string& word,
        int depth
    );

    // Updates the top five completions stored in a Trie node.
    void updateTopK(
        TrieNode* node,
        const string& word,
        int frequency
    );

    // Rebuilds cached top-five completions throughout the Trie.
    void rebuildTopK(
        TrieNode* node,
        string& prefix
    );

public:
    // Creates an empty Trie with a root node.
    Trie();

    // Releases all dynamically allocated Trie nodes.
    ~Trie();

    // Inserts a word or updates its frequency if it already exists.
    void insert(const string& word, int frequency = 1);

    // Checks whether a complete word exists in the Trie.
    bool search(const string& word) const;

    // Checks whether any stored word starts with the given prefix.
    bool startsWith(const string& prefix) const;

    // Removes a word from the Trie if it exists.
    void remove(const string& word);

    // Returns autocomplete suggestions for a prefix, up to maxResults.
    int autocomplete(
        const string& prefix,
        Completion results[],
        int maxResults
    ) const;
};

#endif