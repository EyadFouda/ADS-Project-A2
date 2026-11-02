// main.cpp
// Test driver for the Autocomplete + Ranked Search Engine (CSCE 2200, Milestone 1).
//
// Runs a unit test suite for every data structure (CircularQueue, Heap,
// HashTable, Trie) and the FileParser functions, then loads dictionary.txt
// and shows real ranked autocomplete results and timings.
//
// Usage:
//   ./autocomplete                 run all tests (uses dictionary.txt)
//   ./autocomplete words.txt       run all tests with another dictionary file
//   ./autocomplete -i              run all tests, then an interactive demo
//
// No STL containers or algorithms are used; only iostream, fstream, string
// and chrono (for timing) from the standard library.

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include "CircularQueue.h"
#include "Heap.h"
#include "HashTable.h"
#include "Trie.h"
#include "FileParser.h"

using namespace std;

// ---------------------------------------------------------------------------
// Minimal test framework
// ---------------------------------------------------------------------------

static int g_passed = 0;   // Number of checks that passed
static int g_failed = 0;   // Number of checks that failed

// Records one check and prints PASS/FAIL with a description.
static void check(bool condition, const string& description) {
    if (condition) {
        g_passed++;
        cout << "  [PASS] " << description << "\n";
    } else {
        g_failed++;
        cout << "  [FAIL] " << description << "\n";
    }
}

// Prints a section header.
static void section(const string& title) {
    cout << "\n==================================================\n";
    cout << " " << title << "\n";
    cout << "==================================================\n";
}

// Prints an array of completions as "word (freq)".
static void printCompletions(const Completion results[], int count) {
    if (count == 0) {
        cout << "    (no suggestions)\n";
        return;
    }
    for (int i = 0; i < count; i++) {
        cout << "    " << (i + 1) << ". " << results[i].word
             << " (" << results[i].frequency << ")\n";
    }
}

// Returns true if results are sorted by non-increasing frequency.
static bool isDescending(const Completion results[], int count) {
    for (int i = 1; i < count; i++) {
        if (results[i].frequency > results[i - 1].frequency) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// CircularQueue tests (search history ring buffer)
// ---------------------------------------------------------------------------

static void testCircularQueue() {
    section("CircularQueue (search history ring buffer)");

    CircularQueue q(3);
    string item;

    check(q.isEmpty(), "new queue is empty");
    check(!q.isFull(), "new queue is not full");
    check(q.getSize() == 0, "new queue size is 0");
    check(q.getCapacity() == 3, "capacity is 3");
    check(!q.dequeue(item), "dequeue on empty queue fails");
    check(!q.front(item), "front on empty queue fails");
    check(!q.rear(item), "rear on empty queue fails");
    check(!q.getFromNewest(0, item), "getFromNewest on empty queue fails");

    q.enqueue("apple");
    q.enqueue("banana");
    q.enqueue("cherry");
    check(q.isFull(), "queue full after 3 enqueues");
    check(q.front(item) && item == "apple", "front is oldest item (apple)");
    check(q.rear(item) && item == "cherry", "rear is newest item (cherry)");

    // Overwrite the oldest item when full
    q.enqueue("date");
    check(q.getSize() == 3, "size stays 3 after overwrite");
    check(q.front(item) && item == "banana", "oldest (apple) was overwritten; front is banana");
    check(q.rear(item) && item == "date", "rear is date");

    // History navigation: offset 0 = newest
    check(q.getFromNewest(0, item) && item == "date", "getFromNewest(0) = date");
    check(q.getFromNewest(1, item) && item == "cherry", "getFromNewest(1) = cherry");
    check(q.getFromNewest(2, item) && item == "banana", "getFromNewest(2) = banana");
    check(!q.getFromNewest(3, item), "getFromNewest(3) is out of range");
    check(!q.getFromNewest(-1, item), "getFromNewest(-1) is rejected");

    // Dequeue order is FIFO
    check(q.dequeue(item) && item == "banana", "dequeue returns banana");
    check(q.dequeue(item) && item == "cherry", "dequeue returns cherry");
    check(q.getSize() == 1, "size is 1 after two dequeues");

    // Wrap-around after dequeues
    q.enqueue("elder");
    q.enqueue("fig");
    check(q.isFull(), "full again after wrap-around enqueues");
    check(q.front(item) && item == "date", "front after wrap-around is date");
    check(q.rear(item) && item == "fig", "rear after wrap-around is fig");

    q.clear();
    check(q.isEmpty() && q.getSize() == 0, "clear() empties the queue");

    // Invalid capacity falls back to default 10
    CircularQueue def(0);
    check(def.getCapacity() == 10, "capacity <= 0 falls back to 10");

    // Stress: many overwrites keep only the last N
    CircularQueue s(5);
    for (int i = 0; i < 1000; i++) {
        s.enqueue(to_string(i));
    }
    check(s.getSize() == 5, "after 1000 enqueues size is still 5");
    check(s.front(item) && item == "995", "oldest kept item is 995");
    check(s.rear(item) && item == "999", "newest item is 999");
}

// ---------------------------------------------------------------------------
// Heap tests (max-heap for ranking)
// ---------------------------------------------------------------------------

static void testHeap() {
    section("Heap (binary max-heap)");

    Heap h(2);  // Small capacity to force resize()
    check(h.isEmpty(), "new heap is empty");
    check(h.getSize() == 0, "new heap size is 0");

    HeapNode empty = h.extractMax();
    check(empty.word == "" && empty.frequency == -1, "extractMax on empty heap returns {\"\", -1}");

    h.insert("low", 5);
    h.insert("high", 100);
    h.insert("mid", 50);
    h.insert("top", 1000);
    h.insert("tiny", 1);
    check(h.getSize() == 5, "size is 5 after 5 inserts (resize worked)");

    HeapNode n1 = h.extractMax();
    HeapNode n2 = h.extractMax();
    HeapNode n3 = h.extractMax();
    HeapNode n4 = h.extractMax();
    HeapNode n5 = h.extractMax();
    check(n1.word == "top" && n1.frequency == 1000, "1st extract = top (1000)");
    check(n2.word == "high" && n2.frequency == 100, "2nd extract = high (100)");
    check(n3.word == "mid" && n3.frequency == 50, "3rd extract = mid (50)");
    check(n4.word == "low" && n4.frequency == 5, "4th extract = low (5)");
    check(n5.word == "tiny" && n5.frequency == 1, "5th extract = tiny (1)");
    check(h.isEmpty(), "heap is empty after extracting everything");

    // Stress: insert values in a scrambled order, check output is sorted
    Heap big;
    const int N = 2000;
    for (int i = 0; i < N; i++) {
        int value = (i * 7919) % N;  // permutation of 0..N-1 (7919 is prime)
        big.insert("w" + to_string(value), value);
    }
    check(big.getSize() == N, "2000 inserts stored");

    bool sorted = true;
    int previous = N;
    int extracted = 0;
    while (!big.isEmpty()) {
        HeapNode node = big.extractMax();
        if (node.frequency > previous) sorted = false;
        previous = node.frequency;
        extracted++;
    }
    check(sorted && extracted == N, "2000 extracts come out in descending order");

    // Invalid capacity is handled
    Heap zero(0);
    zero.insert("a", 1);
    zero.insert("b", 2);
    check(zero.extractMax().word == "b", "capacity 0 heap still works");
}

// ---------------------------------------------------------------------------
// HashTable tests (separate chaining)
// ---------------------------------------------------------------------------

static void testHashTable() {
    section("HashTable (separate chaining)");

    HashTable ht(5);  // Small capacity to force resize()
    int freq = 0;

    check(ht.getSize() == 0, "new table size is 0");
    check(ht.getCapacity() == 5, "initial capacity is 5");

    ht.insert("hello", 10);
    ht.insert("world", 20);
    ht.insert("data", 30);
    check(ht.getSize() == 3, "size is 3 after 3 inserts");
    check(ht.search("hello", freq) && freq == 10, "search(hello) finds frequency 10");
    check(ht.search("world", freq) && freq == 20, "search(world) finds frequency 20");
    check(ht.contains("data"), "contains(data) is true");
    check(!ht.contains("missing"), "contains(missing) is false");

    // Duplicate insert keeps the larger frequency, size unchanged
    ht.insert("hello", 5);
    check(ht.search("hello", freq) && freq == 10, "re-insert with lower freq keeps 10");
    ht.insert("hello", 99);
    check(ht.search("hello", freq) && freq == 99, "re-insert with higher freq updates to 99");
    check(ht.getSize() == 3, "duplicates do not increase size");

    // Invalid input is rejected
    ht.insert("Hello", 1);
    ht.insert("abc1", 1);
    ht.insert("", 1);
    ht.insert("valid", 0);
    ht.insert("valid", -3);
    check(ht.getSize() == 3, "uppercase/digit/empty words and freq <= 0 rejected");

    // Resize: load factor should stay <= 0.75
    int capBefore = ht.getCapacity();
    ht.insert("alpha", 1);
    ht.insert("beta", 2);
    ht.insert("gamma", 3);
    check(ht.getCapacity() > capBefore, "table resized when load factor exceeded 0.75");
    check(ht.getLoadFactor() <= 0.75, "load factor <= 0.75 after resize");
    check(ht.contains("hello") && ht.contains("world") && ht.contains("data"),
          "old entries still found after resize");

    // Removal
    check(ht.remove("world"), "remove(world) succeeds");
    check(!ht.contains("world"), "world is gone after removal");
    check(!ht.remove("world"), "removing world again fails");
    check(!ht.remove("nothere"), "removing a missing word fails");
    check(ht.getSize() == 5, "size is 5 after one removal");

    // Stress: 5000 generated words
    HashTable big;
    const int N = 5000;
    for (int i = 0; i < N; i++) {
        // Build a unique lowercase word from i (base-26 digits)
        string w = "";
        int x = i;
        do { w += char('a' + x % 26); x /= 26; } while (x > 0);
        w += "z";
        big.insert(w, i + 1);
    }
    check(big.getSize() == N, "5000 unique words inserted");

    bool allFound = true;
    for (int i = 0; i < N; i++) {
        string w = "";
        int x = i;
        do { w += char('a' + x % 26); x /= 26; } while (x > 0);
        w += "z";
        int f = 0;
        if (!big.search(w, f) || f != i + 1) { allFound = false; break; }
    }
    check(allFound, "all 5000 words found with correct frequencies");
    cout << "    capacity = " << big.getCapacity()
         << ", load factor = " << big.getLoadFactor() << "\n";
}

// ---------------------------------------------------------------------------
// Trie tests (prefix tree with cached top-5 completions)
// ---------------------------------------------------------------------------

static void testTrie() {
    section("Trie (prefix tree with cached top-5)");

    Trie t;
    Completion results[10];

    check(!t.search("car"), "empty trie: search(car) is false");
    check(t.startsWith(""), "empty prefix always matches");
    check(t.autocomplete("ca", results, 5) == 0, "empty trie: no completions");

    t.insert("car", 50);
    t.insert("card", 30);
    t.insert("care", 80);
    t.insert("careful", 10);
    t.insert("cart", 20);
    t.insert("cat", 90);
    t.insert("dog", 40);

    check(t.search("car"), "search(car) is true");
    check(t.search("careful"), "search(careful) is true");
    check(!t.search("ca"), "search(ca) is false (prefix only, not a word)");
    check(!t.search("cars"), "search(cars) is false");
    check(t.startsWith("ca"), "startsWith(ca) is true");
    check(t.startsWith("dog"), "startsWith(dog) is true");
    check(!t.startsWith("x"), "startsWith(x) is false");
    check(!t.search("CAR"), "uppercase word rejected by search");

    int n = t.autocomplete("ca", results, 5);
    cout << "  autocomplete(\"ca\", 5):\n";
    printCompletions(results, n);
    check(n == 5, "ca has 6 matching words but cache returns top 5");
    check(isDescending(results, n), "completions are in descending frequency");
    check(results[0].word == "cat" && results[0].frequency == 90, "top completion for ca is cat (90)");
    check(results[4].word == "cart" && results[4].frequency == 20, "5th completion is cart (20); careful (10) excluded");

    n = t.autocomplete("care", results, 5);
    cout << "  autocomplete(\"care\", 5):\n";
    printCompletions(results, n);
    check(n == 2 && results[0].word == "care" && results[1].word == "careful",
          "care -> [care, careful]");

    n = t.autocomplete("ca", results, 2);
    check(n == 2 && results[0].word == "cat" && results[1].word == "care",
          "maxResults = 2 limits output to cat, care");
    check(t.autocomplete("ca", results, 0) == 0, "maxResults = 0 returns nothing");
    check(t.autocomplete("zz", results, 5) == 0, "unknown prefix returns nothing");

    // Frequency update moves a word up the cached ranking
    t.insert("careful", 1000);
    n = t.autocomplete("ca", results, 5);
    check(n == 5 && results[0].word == "careful" && results[0].frequency == 1000,
          "raising careful to 1000 puts it at the top of ca");
    check(isDescending(results, n), "order still descending after update");

    // Lower frequency re-insert does not decrease it
    t.insert("careful", 1);
    n = t.autocomplete("careful", results, 5);
    check(n == 1 && results[0].frequency == 1000, "lower re-insert keeps frequency 1000");

    // Invalid inserts are ignored
    t.insert("Bad", 5);
    t.insert("x1", 5);
    t.insert("", 5);
    t.insert("zero", 0);
    check(!t.search("bad") && !t.search("zero") && !t.startsWith("x"),
          "invalid words and freq <= 0 are ignored");

    // Removal: word that is a prefix of other words
    t.remove("car");
    check(!t.search("car"), "remove(car): car no longer a word");
    check(t.search("card") && t.search("care") && t.search("cart"),
          "card, care, cart still exist after removing car");
    n = t.autocomplete("car", results, 5);
    bool carListed = false;
    for (int i = 0; i < n; i++) if (results[i].word == "car") carListed = true;
    check(!carListed, "car no longer suggested for prefix car");
    cout << "  autocomplete(\"car\", 5) after removing car:\n";
    printCompletions(results, n);

    // Removal: leaf word frees its branch
    t.remove("dog");
    check(!t.search("dog") && !t.startsWith("d"), "remove(dog) frees the d branch");

    // Removal: cache refills with a word that was previously excluded
    Trie r;
    r.insert("aa", 6); r.insert("ab", 5); r.insert("ac", 4);
    r.insert("ad", 3); r.insert("ae", 2); r.insert("af", 1);
    n = r.autocomplete("a", results, 5);
    check(n == 5 && results[4].word == "ae", "before removal: af (1) not in top 5");
    r.remove("aa");
    n = r.autocomplete("a", results, 5);
    check(n == 5 && results[0].word == "ab" && results[4].word == "af",
          "after removing aa: cache rebuilt, af enters top 5");

    // Removing a missing word does nothing harmful
    r.remove("zzz");
    r.remove("a");
    check(r.search("ab") && r.search("af"), "removing missing words leaves trie intact");
}

// ---------------------------------------------------------------------------
// FileParser tests (normalizeWord, loadDictionary, rankedSearch)
// ---------------------------------------------------------------------------

static void testFileParser() {
    section("FileParser (normalizeWord, loadDictionary, rankedSearch)");

    string out;
    check(normalizeWord("Hello", out) && out == "hello", "normalizeWord(Hello) -> hello");
    check(normalizeWord("ABC", out) && out == "abc", "normalizeWord(ABC) -> abc");
    check(!normalizeWord("don't", out), "normalizeWord(don't) rejected");
    check(!normalizeWord("abc123", out), "normalizeWord(abc123) rejected");
    check(!normalizeWord("", out), "normalizeWord(empty) rejected");

    // Write a small test file covering valid and invalid lines
    const string testFile = "test_dictionary_tmp.txt";
    {
        ofstream f(testFile);
        f << "apple 100\n";        // valid
        f << "Apply 80\n";         // valid, uppercase normalized
        f << "apt\n";              // valid, missing freq -> 1
        f << "\n";                 // blank, ignored (not skipped)
        f << "   \n";              // whitespace only, ignored
        f << "app 0\n";            // freq 0 -> skipped
        f << "ape -5\n";           // negative -> skipped
        f << "apex 12abc\n";       // non-numeric freq -> skipped
        f << "april 10 extra\n";   // extra token -> skipped
        f << "can't 50\n";         // invalid word -> skipped
        f << "apron 99999999999\n";// overflow -> skipped
        f << "applet 60\n";        // valid
        f << "apple 200\n";        // duplicate, raises frequency
    }

    Trie trie;
    int skipped = 0;
    int loaded = loadDictionary(testFile, trie, skipped);
    cout << "  loaded = " << loaded << ", skipped = " << skipped << "\n";
    check(loaded == 5, "5 valid lines loaded (apple, apply, apt, applet, apple dup)");
    check(skipped == 6, "6 invalid lines skipped");
    check(trie.search("apply"), "Apply was normalized and inserted as apply");
    check(trie.search("apt"), "apt inserted with default frequency");
    check(!trie.search("app") && !trie.search("april") && !trie.search("apron"),
          "invalid lines were not inserted");

    Completion results[10];
    int n = rankedSearch(trie, "ap", results, 10);
    cout << "  rankedSearch(\"ap\", 10):\n";
    printCompletions(results, n);
    check(n == 4, "ap returns 4 words");
    check(isDescending(results, n), "rankedSearch output is descending");
    check(results[0].word == "apple" && results[0].frequency == 200,
          "duplicate raised apple to 200 and it ranks first");
    check(results[3].word == "apt" && results[3].frequency == 1, "apt (1) ranks last");

    n = rankedSearch(trie, "AP", results, 10);
    check(n == 4 && results[0].word == "apple", "uppercase prefix AP is normalized");
    check(rankedSearch(trie, "ap!", results, 10) == 0, "invalid prefix returns 0");
    check(rankedSearch(trie, "ap", results, 0) == 0, "maxResults 0 returns 0");
    check(rankedSearch(trie, "ap", nullptr, 5) == 0, "null results array returns 0");
    n = rankedSearch(trie, "ap", results, 2);
    check(n == 2 && results[0].word == "apple" && results[1].word == "apply",
          "maxResults 2 returns apple, apply");
    n = rankedSearch(trie, "", results, 10);
    check(n == 4, "empty prefix returns overall top words");

    int dummy = 0;
    check(loadDictionary("file_that_does_not_exist.txt", trie, dummy) == -1,
          "missing file returns -1");

    remove(testFile.c_str());
}

// ---------------------------------------------------------------------------
// Full dictionary demo: load real data, query, time it
// ---------------------------------------------------------------------------

static bool testRealDictionary(const string& filename, Trie& trie, HashTable& table) {
    section("Real dictionary: " + filename);

    int skipped = 0;
    auto start = chrono::high_resolution_clock::now();
    int loaded = loadDictionary(filename, trie, skipped);
    auto end = chrono::high_resolution_clock::now();
    double loadMs = chrono::duration<double, milli>(end - start).count();

    if (loaded < 0) {
        check(false, "could not open " + filename + " (place it next to the executable)");
        return false;
    }

    cout << "  loaded " << loaded << " words, skipped " << skipped
         << " lines in " << loadMs << " ms\n";
    check(loaded > 0, "dictionary loaded at least one word");

    // Load the same file into the HashTable for O(1) exact lookups
    {
        ifstream f(filename);
        string raw, word;
        int freq;
        while (f >> raw >> freq) {
            if (normalizeWord(raw, word)) table.insert(word, freq);
        }
    }
    cout << "  hash table: " << table.getSize() << " words, capacity "
         << table.getCapacity() << ", load factor " << table.getLoadFactor() << "\n";
    check(table.getSize() > 0, "hash table filled from dictionary");

    // Sample prefixes
    const int NUM = 6;
    string prefixes[NUM] = {"th", "pro", "comp", "eng", "z", "qwx"};
    Completion results[5];
    for (int p = 0; p < NUM; p++) {
        int n = rankedSearch(trie, prefixes[p], results, 5);
        cout << "  rankedSearch(\"" << prefixes[p] << "\"):\n";
        printCompletions(results, n);
        check(isDescending(results, n), "results for \"" + prefixes[p] + "\" are descending");

        // Every suggestion must exist in the trie, start with the prefix,
        // and have the same frequency as stored in the hash table
        bool consistent = true;
        for (int i = 0; i < n; i++) {
            int f = 0;
            if (!trie.search(results[i].word) ||
                results[i].word.compare(0, prefixes[p].size(), prefixes[p]) != 0 ||
                !table.search(results[i].word, f) || f != results[i].frequency) {
                consistent = false;
            }
        }
        check(consistent, "suggestions for \"" + prefixes[p] + "\" match prefix and hash table");
    }

    int fYou = 0;
    check(table.search("you", fYou) && fYou == 28787591, "hash table: you -> 28787591");
    int n = rankedSearch(trie, "", results, 1);
    check(n == 1 && results[0].word == "you", "most frequent word overall is you");

    // Timing: average query time with the cached top-K
    const int QUERIES = 100000;
    start = chrono::high_resolution_clock::now();
    int total = 0;
    for (int i = 0; i < QUERIES; i++) {
        total += rankedSearch(trie, prefixes[i % 4], results, 5);
    }
    end = chrono::high_resolution_clock::now();
    double usPerQuery = chrono::duration<double, micro>(end - start).count() / QUERIES;
    cout << "  " << QUERIES << " ranked searches: average "
         << usPerQuery << " microseconds per query (" << total << " results)\n";
    check(total > 0, "timed queries returned results");

    return true;
}

// ---------------------------------------------------------------------------
// Interactive demo: autocomplete with history kept in a CircularQueue
// ---------------------------------------------------------------------------

static void interactive(Trie& trie, HashTable& table) {
    section("Interactive demo");
    cout << "  Type a prefix to get the top 5 suggestions.\n"
         << "  Commands: :history  :lookup <word>  :add <word> <freq>  "
            ":remove <word>  :quit\n";

    CircularQueue history(5);
    string line;

    while (true) {
        cout << "\n> ";
        if (!getline(cin, line)) break;
        if (line.empty()) continue;

        if (line == ":quit") break;

        if (line == ":history") {
            if (history.isEmpty()) cout << "  (empty)\n";
            for (int i = 0; i < history.getSize(); i++) {
                string h;
                history.getFromNewest(i, h);
                cout << "  " << i << ": " << h << "\n";
            }
            continue;
        }

        if (line.compare(0, 8, ":lookup ") == 0) {
            string w;
            int f = 0;
            if (normalizeWord(line.substr(8), w) && table.search(w, f))
                cout << "  " << w << " -> " << f << "\n";
            else
                cout << "  not found\n";
            continue;
        }

        if (line.compare(0, 5, ":add ") == 0) {
            string rest = line.substr(5);
            size_t sp = rest.find(' ');
            string w;
            if (sp != string::npos && normalizeWord(rest.substr(0, sp), w)) {
                int f = atoi(rest.substr(sp + 1).c_str());
                if (f > 0) {
                    trie.insert(w, f);
                    table.insert(w, f);
                    cout << "  added " << w << " (" << f << ")\n";
                    continue;
                }
            }
            cout << "  usage: :add <word> <positive freq>\n";
            continue;
        }

        if (line.compare(0, 8, ":remove ") == 0) {
            string w;
            if (normalizeWord(line.substr(8), w) && trie.search(w)) {
                trie.remove(w);
                table.remove(w);
                cout << "  removed " << w << "\n";
            } else {
                cout << "  not found\n";
            }
            continue;
        }

        history.enqueue(line);
        Completion results[5];
        int n = rankedSearch(trie, line, results, 5);
        printCompletions(results, n);
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    string dictFile = "dictionary.txt";
    bool interactiveMode = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-i") interactiveMode = true;
        else dictFile = arg;
    }

    cout << "Autocomplete + Ranked Search Engine - test driver\n";

    testCircularQueue();
    testHeap();
    testHashTable();
    testTrie();
    testFileParser();

    Trie trie;
    HashTable table;
    bool dictOk = testRealDictionary(dictFile, trie, table);

    section("Summary");
    cout << "  Passed: " << g_passed << "\n";
    cout << "  Failed: " << g_failed << "\n";
    cout << (g_failed == 0 ? "  ALL TESTS PASSED\n" : "  SOME TESTS FAILED\n");

    if (interactiveMode && dictOk) {
        interactive(trie, table);
    }

    return g_failed == 0 ? 0 : 1;
}
