#include "FileParser.h"
#include "Heap.h"
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

// Converts the raw words to the form that the Trie accepts which is lowercase alphabet only
// Returns true if the word is valid, and stores the normalized word in out
bool normalizeWord(const string& raw, string& out) {
    out.clear();

    if (raw.empty()) {
        return false;
    }

    for (int i = 0; i < (int)raw.size(); i++) {
        char c = raw[i];

        // Convert uppercase to lowercase
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }

        // The Trie has exactly 26 children per node, so anything that is not a lowercase letter at this point cannot be stored
        if (c < 'a' || c > 'z') {
            return false;
        }

        out += c;
    }

    return true;
}

// Reads dictionary entries and inserts valid words into the Trie
// Each line contains a word and an optional positive frequency
// Returns the number of loaded entries, or -1 if the file fails
// SkippedLines counts invalid non-empty lines
int loadDictionary(const string& filename, Trie& trie, int& skippedLines) {
    skippedLines = 0;

    ifstream file(filename);

    // Checks if the file was opened successfully
    if (!file.is_open()) {
        return -1;
    }

    int loaded = 0;
    string line;

    // Read each word-frequency pair
    while (getline(file, line)) {
        istringstream lineStream(line);

        string rawWord;
        // Ignore blank or whitespace-only lines, but don't count them as skipped
        if (!(lineStream >> rawWord)) {
            continue;
        }

    // A missing frequency defaults to 1, which lets the same loader handle plain word lists.
        int frequency = 1;
        string freqToken;
        if (lineStream >> freqToken) {
            int value = 0;
            bool isNumber = true;

            // Convert the frequency string to an integer, checking for invalid characters and overflow
            for (int i = 0; i < (int)freqToken.size(); i++) {
                char d = freqToken[i];
                // Check if the character is a digit
                if (d < '0' || d > '9') {
                    isNumber = false;
                    break;
                }
                // Prevent int overflow
                if (value > (2147483647 - (d - '0')) / 10) {
                    isNumber = false;
                    break;
                }
                value = value * 10 + (d - '0');
            }

            // Frequency must be a positive integer, so reject zero or negative values
            if (!isNumber || value <= 0) {
                skippedLines++;
                continue;
            }
            frequency = value;

            // Reject lines with extra tokens after the frequency
            string extra;
            if (lineStream >> extra) {
                skippedLines++;
                continue;
            }
        }

        // Normalize and validate word before inserting into the Trie
        string word;
        if (!normalizeWord(rawWord, word)) {
            skippedLines++;
            continue;
        }

        trie.insert(word, frequency);
        loaded++;
    }

    // Detect a file-reading error before closing the stream
    bool readError = file.bad();

    file.close();

    if (readError) {
        return -1;
    }

    return loaded;
}


// Finds autocomplete suggestions using the Trie's top-5 cache, then uses a Max-Heap to return them in descending frequency order.
// Returns the number of suggestions written to results.
int rankedSearch(
    const Trie& trie,
    const string& prefix,
    Completion results[],
    int maxResults
) {
    // Reject invalid output requests
    if (maxResults <= 0 || results == nullptr) {
        return 0;
    }

    // Normalize the user's prefix so it matches the Trie's format
    // An empty prefix is allowed and searches from the root of the Trie, but any invalid prefix is rejected
    string normalized;

    if (!prefix.empty() &&
        !normalizeWord(prefix, normalized)) {
        return 0;
    }

    // The Trie caches at most 5 suggestions for each prefix, so we can use that to avoid traversing the Trie for every search
    const int TOP_K = 5;
    Completion candidates[TOP_K];

    // Get cached suggestions for this prefix without traversing the subtree
    int count = trie.autocomplete(
        normalized,
        candidates,
        TOP_K
    );

    // Insert the cached suggestions into the Max-Heap.
    Heap heap;

    for (int i = 0; i < count; i++) {
        heap.insert(
            candidates[i].word,
            candidates[i].frequency
        );
    }

    // Extract the highest-frequency suggestions first.
    int resultCount = 0;

    while (!heap.isEmpty() && resultCount < maxResults) {
        HeapNode best = heap.extractMax();

        results[resultCount].word = best.word;
        results[resultCount].frequency = best.frequency;

        resultCount++;
    }

    return resultCount;
}