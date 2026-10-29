
#ifndef TRIE_H
#define TRIE_H

#include <string>

using namespace std;

struct Completion {
    string word;
    int frequency;

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

    TrieNode();
};

class Trie {
private:
    TrieNode* root;

    int getIndex(char c) const;
    bool isValidWord(const string& word) const;
    void destroyTrie(TrieNode* node);

    bool deleteHelper(
        TrieNode* node,
        const string& word,
        int depth
    );

    void updateTopK(
        TrieNode* node,
        const string& word,
        int frequency
    );

    void rebuildTopK(
        TrieNode* node,
        string& prefix
    );

public:
    Trie();
    ~Trie();

    void insert(const string& word, int frequency = 1);
    bool search(const string& word) const;
    bool startsWith(const string& prefix) const;
    void remove(const string& word);

    int autocomplete(
        const string& prefix,
        Completion results[],
        int maxResults
    ) const;
};

#endif
