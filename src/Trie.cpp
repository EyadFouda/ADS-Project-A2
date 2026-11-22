
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

    for (size_t i = 0; i < word.length(); i++) {
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

    for (size_t i = 0; i < word.length(); i++) {
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

    for (size_t i = 0; i < word.length(); i++) {
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

    for (size_t i = 0; i < word.length(); i++) {
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

    for (size_t i = 0; i < prefix.length(); i++) {
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

    for (int i = 0; i < static_cast<int>(prefix.length()); i++) {
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

    if (depth == static_cast<int>(word.length())) {
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

// Recalculates cached top-five completions for a node and its descendants using the Heap class
void Trie::rebuildTopK(TrieNode* node, string& prefix) {
    if (node == nullptr) {
        return;
    }

    Heap subtreeHeap;
    string currentPrefix = prefix;
    collectSubtreeWords(node, currentPrefix, subtreeHeap);

    node->topKSize = 0;
    while (!subtreeHeap.isEmpty() && node->topKSize < 5) {
        HeapNode maxNode = subtreeHeap.extractMax();
        node->topK[node->topKSize].word = maxNode.word;
        node->topK[node->topKSize].frequency = maxNode.frequency;
        node->topKSize++;
    }

    // Recursively rebuild for all children
    for (int i = 0; i < 26; i++) {
        if (node->children[i] != nullptr) {
            prefix.push_back('a' + i);
            rebuildTopK(node->children[i], prefix);
            prefix.pop_back();
        }
    }
}

// Helper function: Recursively collects all words and frequencies from a subtree into a Heap
void Trie::collectSubtreeWords(TrieNode* node, string& currentPrefix, Heap& heap) const {
    if (node == nullptr) {
        return;
    }

    if (node->isEndOfWord) {
        heap.insert(currentPrefix, node->frequency);
    }

    for (int i = 0; i < 26; i++) {
        if (node->children[i] != nullptr) {
            currentPrefix.push_back('a' + i);
            collectSubtreeWords(node->children[i], currentPrefix, heap);
            currentPrefix.pop_back();
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


// Eyad: Function that chooses the maximum allowed edit distance
int getMaxEditDistance(int queryLength){
    if(queryLength <= 2)
            return 0;
    if(queryLength <= 5)
            return 1;
    return 2;
}

// Eyad: Function that adds possible words into the candidates array, whenever the trie finds a full word
void recordFuzzyCandidate(const std::string& word, int frequency, int editDistance, int maxAllowed, FuzzyMatch candidates[], int& candidateCount, int maxCandidates) {
    if (word.empty()) 
        return;
    if (candidateCount >= maxCandidates) 
        return;
    if (editDistance > maxAllowed) 
        return;
    candidates[candidateCount] = FuzzyMatch{word, frequency, editDistance};
    candidateCount++;
}

// Eyad: Function that compares fuzzy matches by edit distance then by frequency then by alphabetically
bool isBetterFuzzyMatch(const FuzzyMatch& a, const FuzzyMatch& b) {
    if (a.editDistance != b.editDistance) 
        return a.editDistance < b.editDistance;
    if (a.frequency != b.frequency)
        return a.frequency > b.frequency;
    return a.word < b.word;
}

// Eyad: Function that ranks the top maxResults matches and puts them into the results
int rankFuzzyMatches(FuzzyMatch candidates[], int candidateCount, FuzzyMatch results[], int maxResults) {
    if (!candidates || !results || candidateCount <= 0 || maxResults <= 0)
        return 0;
    int maxDistFound = 0;
    for (int i = 0; i < candidateCount; i++) { // Loop that finds maximum edit distance in the candidates
        if (candidates[i].editDistance > maxDistFound) {
            maxDistFound = candidates[i].editDistance;
        }
    }
    int resultCount = 0;

    // Loop that processes edit distance in ascending order
    for (int dist = 0; dist <= maxDistFound && resultCount < maxResults; dist++) {
        Heap tierHeap(candidateCount > 0 ? candidateCount : 1); /// Use heap to rank candidates

        for (int i = 0; i < candidateCount; i++) {
            if (candidates[i].editDistance == dist) {
                tierHeap.insert(candidates[i].word, candidates[i].frequency);
            }
        }

        // Loop that extracts top element from heap and puts it in results
        while (!tierHeap.isEmpty() && resultCount < maxResults) {
            HeapNode topNode = tierHeap.extractMax();
            results[resultCount].word = topNode.word;
            results[resultCount].frequency = topNode.frequency;
            results[resultCount].editDistance = dist;
            resultCount++;
        }
    }
    return resultCount;
}

//SARA
void Trie::fuzzyTraversal(
   const TrieNode* node,
   char letter,
   const string& query,
   const int previousRow[],
   int maxAllowed,
   string& currentWord,
   FuzzyMatch candidates[],
   int& candidateCount,
   int maxCandidates
) const {
   if (node == nullptr) {
       return;
   }


   int queryLen = (int)query.length();


   int* currentRow = new int[queryLen + 1];


   currentRow[0] = previousRow[0] + 1;
   int rowMinimum = currentRow[0];


   for (int j = 1; j <= queryLen; j++) {
       int cost = (query[j - 1] == letter) ? 0 : 1;


       int deletion = previousRow[j] + 1;
       int insertion = currentRow[j - 1] + 1;
       int substitution = previousRow[j - 1] + cost;


       int best = deletion;


      
       if (insertion < best) {
           best = insertion;
       }


       if (substitution < best) {
           best = substitution;
       }


       currentRow[j] = best;


       if (best < rowMinimum) {
           rowMinimum = best;
       }
   }


   currentWord.push_back(letter);


   if (node->isEndOfWord &&
       currentRow[queryLen] <= maxAllowed) {


       recordFuzzyCandidate(
           currentWord,
           node->frequency,
           currentRow[queryLen],
           maxAllowed,
           candidates,
           candidateCount,
           maxCandidates
       );
   }


   if (rowMinimum <= maxAllowed) {
       for (int i = 0; i < 26; i++) {
           if (node->children[i] != nullptr) {
               fuzzyTraversal(
                   node->children[i],
                   'a' + i,
                   query,
                   currentRow,
                   maxAllowed,
                   currentWord,
                   candidates,
                   candidateCount,
                   maxCandidates
               );
           }
       }
   }


   currentWord.pop_back();


   delete[] currentRow;
}


int Trie::getFuzzyMatches(
   const string& query,
   FuzzyMatch candidates[],
   int maxCandidates,
   int maxAllowed
) const {
   if (!isValidWord(query) ||
       candidates == nullptr ||
       maxCandidates <= 0 ||
       maxAllowed < 0 ||
       root==nullptr) {
       return 0;
   }


   int queryLen = (int)query.length();


   int* initialRow = new int[queryLen + 1];


   for (int j = 0; j <= queryLen; j++) {
       initialRow[j] = j;
   }


   int candidateCount = 0;
   string currentWord = "";


   for (int i = 0; i < 26; i++) {
       if (root->children[i] != nullptr) {
           fuzzyTraversal(
               root->children[i],
               'a' + i,
               query,
               initialRow,
               maxAllowed,
               currentWord,
               candidates,
               candidateCount,
               maxCandidates
           );
       }
   }


   delete[] initialRow;


   return candidateCount;
}
