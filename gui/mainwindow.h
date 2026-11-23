#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <string>
#include "Trie.h"
#include "HashTable.h"
#include "CircularQueue.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class QListWidgetItem;
QT_END_NAMESPACE

// Main window: shows live suggestions while typing, checks a word on Enter,
// offers "did you mean" matches, and keeps a recent-search history.
// The window only calls the data structures; it does not change how they work.
class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    Trie* trie;              // Prefix search and fuzzy matching.
    HashTable* table;        // Exact word lookup.
    CircularQueue* history;  // Last few searches.

    int historyCursor;       // Offset from the newest search while browsing with Up/Down, -1 = not browsing.
    bool recalling;          // True while the search box is being filled from history.

    // Saves a search unless it is empty or repeats the newest one.
    void recordSearch(const std::string& search);

    // Redraws the history list, newest search first.
    void refreshHistoryList();

    // Fills the search box with a past search without resetting navigation.
    void showRecalled(const std::string& search);

protected:
    // Catches Up/Down in the search box to browse history.
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void updateSuggestions(const QString& text);
    void selectSuggestion(QListWidgetItem* item);
    void submitSearch();
    void selectHistoryItem(QListWidgetItem* item);
    void clearHistory();

public:
    MainWindow(Trie* trie, HashTable* table, CircularQueue* history, QWidget *parent = nullptr);
    ~MainWindow();
};

#endif
