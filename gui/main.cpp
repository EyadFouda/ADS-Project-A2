#include <QApplication>
#include <QMessageBox>
#include <fstream>
#include <sstream>
#include <string>
#include "mainwindow.h"
#include "Trie.h"
#include "HashTable.h"
#include "CircularQueue.h"
#include "FileParser.h"

using namespace std;

// Checks whether a frequency contains a valid positive integer.
bool parseFrequency(const string& token, int& frequency) {
    if (token.empty()) {
        return false;
    }

    int value = 0;

    for (int i = 0; i < (int)token.length(); i++) {
        if (token[i] < '0' || token[i] > '9') {
            return false;
        }

        int digit = token[i] - '0';

        if (value > (2147483647 - digit) / 10) {
            return false;
        }

        value = value * 10 + digit;
    }

    if (value <= 0) {
        return false;
    }

    frequency = value;
    return true;
}

// Loads the dictionary into the Hash Table.
bool loadHashTable(const string& filename, HashTable& table) {
    ifstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    string line;

    while (getline(file, line)) {
        istringstream stream(line);

        string rawWord;
        string frequencyToken;

        if (!(stream >> rawWord)) {
            continue;
        }

        int frequency = 1;

        if (stream >> frequencyToken) {
            if (!parseFrequency(frequencyToken, frequency)) {
                continue;
            }

            string extra;

            if (stream >> extra) {
                continue;
            }
        }

        string word;

        if (normalizeWord(rawWord, word)) {
            table.insert(word, frequency);
        }
    }

    file.close();
    return true;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Trie trie;
    HashTable table;
    CircularQueue history(5);

    // CMake copies dictionary.txt next to the .exe, so this works from Qt Creator or a double-click.
    string filename = (QCoreApplication::applicationDirPath() + "/dictionary.txt").toStdString();
    int skipped = 0;

    if (loadDictionary(filename, trie, skipped) == -1 ||
        !loadHashTable(filename, table)) {
        QMessageBox::critical(nullptr, "Error",
                              "Could not open dictionary file:\n" + QString::fromStdString(filename));
        return 1;
    }

    MainWindow window(&trie, &table, &history);
    window.show();

    return app.exec();
}
