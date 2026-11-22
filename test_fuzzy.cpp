#include <iostream>
#include <string>
#include "Trie.h"

namespace {
    void printResults(const std::string& label,
                      const std::string& query,
                      int maxAllowed,
                      int candidateCount,
                      const FuzzyMatch results[],
                      int rankedCount) {
        std::cout << "=== " << label << " ===\n";
        std::cout << "Query: \"" << query << "\" | maxAllowed: " << maxAllowed
                  << " | candidates: " << candidateCount
                  << " | ranked: " << rankedCount << "\n";

        for (int i = 0; i < rankedCount; i++) {
            std::cout << "  " << (i + 1) << ". " << results[i].word
                      << " (dist=" << results[i].editDistance
                      << ", freq=" << results[i].frequency << ")\n";
        }
        std::cout << "\n";
    }
}

int main() {
    Trie trie;
    trie.insert("apple", 50);
    trie.insert("apply", 30);
    trie.insert("maple", 80);
    trie.insert("apricot", 40);
    trie.insert("banana", 100);
    trie.insert("band", 25);

    const int MAX_CANDIDATES = 128;
    const int MAX_RESULTS = 5;
    FuzzyMatch candidates[MAX_CANDIDATES];
    FuzzyMatch results[MAX_RESULTS];

    // 1. Typo query (len 4 -> maxAllowed = 1): expects "apple" (dist 1) and "maple" (dist 2 if included)
    {
        std::string q = "aple";
        int maxAllowed = getMaxEditDistance((int)q.length());
        int candCount = trie.getFuzzyMatches(q, candidates, MAX_CANDIDATES, maxAllowed);
        int rankedCount = rankFuzzyMatches(candidates, candCount, results, MAX_RESULTS);
        printResults("1-Edit Typo (len=4)", q, maxAllowed, candCount, results, rankedCount);
    }

    // 2. Exact match test (dist 0 comes first)
    {
        std::string q = "apple";
        int maxAllowed = getMaxEditDistance((int)q.length());
        int candCount = trie.getFuzzyMatches(q, candidates, MAX_CANDIDATES, maxAllowed);
        int rankedCount = rankFuzzyMatches(candidates, candCount, results, MAX_RESULTS);
        printResults("Exact Match", q, maxAllowed, candCount, results, rankedCount);
    }

    // 3. Short query (len 2 -> maxAllowed = 0): no fuzzy matches allowed
    {
        std::string q = "ba";
        int maxAllowed = getMaxEditDistance((int)q.length());
        int candCount = trie.getFuzzyMatches(q, candidates, MAX_CANDIDATES, maxAllowed);
        int rankedCount = rankFuzzyMatches(candidates, candCount, results, MAX_RESULTS);
        printResults("Short Query (len=2, 0 edits)", q, maxAllowed, candCount, results, rankedCount);
    }

    // 4. Longer query (len 6 -> maxAllowed = 2)
    {
        std::string q = "bannaa";
        int maxAllowed = getMaxEditDistance((int)q.length());
        int candCount = trie.getFuzzyMatches(q, candidates, MAX_CANDIDATES, maxAllowed);
        int rankedCount = rankFuzzyMatches(candidates, candCount, results, MAX_RESULTS);
        printResults("Longer Query (len=6, max 2 edits)", q, maxAllowed, candCount, results, rankedCount);
    }

    // 5. Invalid / empty query edge cases
    {
        int countEmpty = trie.getFuzzyMatches("", candidates, MAX_CANDIDATES, 1);
        int countInvalid = trie.getFuzzyMatches("Apple!", candidates, MAX_CANDIDATES, 2);
        std::cout << "=== Edge Cases ===\n";
        std::cout << "Empty query candidates: " << countEmpty << " (expected 0)\n";
        std::cout << "Invalid chars query candidates: " << countInvalid << " (expected 0)\n";
    }

    return 0;
}
