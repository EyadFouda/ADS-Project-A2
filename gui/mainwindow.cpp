#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include "FileParser.h"

using namespace std;

MainWindow::MainWindow(Trie* trie, HashTable* table, CircularQueue* history, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      trie(trie),
      table(table),
      history(history),
      historyCursor(-1),
      recalling(false)
{
    ui->setupUi(this);

    // Typing updates suggestions; Enter submits the search.
    connect(ui->searchBox, &QLineEdit::textChanged,
            this, &MainWindow::updateSuggestions);
    connect(ui->searchBox, &QLineEdit::returnPressed,
            this, &MainWindow::submitSearch);

    // Queued so the list is not cleared while Qt is still handling the click.
    connect(ui->suggestionList, &QListWidget::itemClicked,
            this, &MainWindow::selectSuggestion, Qt::QueuedConnection);
    connect(ui->historyList, &QListWidget::itemClicked,
            this, &MainWindow::selectHistoryItem, Qt::QueuedConnection);

    connect(ui->clearHistoryButton, &QPushButton::clicked,
            this, &MainWindow::clearHistory);

    // Let the window see key presses in the search box (for Up/Down).
    ui->searchBox->installEventFilter(this);

    refreshHistoryList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Shows up to five ranked completions for the current text.
void MainWindow::updateSuggestions(const QString& text)
{
    // Typing by hand stops history browsing, so the next Up starts at the newest search.
    if (!recalling) {
        historyCursor = -1;
    }

    ui->suggestionList->clear();

    string prefix = text.toStdString();

    if (prefix.empty()) {
        ui->statusLabel->setText("Start typing to see suggestions.");
        return;
    }

    Completion results[5];
    int count = rankedSearch(*trie, prefix, results, 5);

    for (int i = 0; i < count; i++) {
        ui->suggestionList->addItem(QString::fromStdString(results[i].word));
    }

    if (count == 0) {
        ui->statusLabel->setText("No suggestions found. Press Enter to check spelling.");
    }
    else {
        ui->statusLabel->setText(QString::number(count) + " suggestions found.");
    }
}

// Clicking a suggestion puts it in the search box and searches for it.
void MainWindow::selectSuggestion(QListWidgetItem* item)
{
    if (item == nullptr) {
        return;
    }

    QString word = item->text();
    ui->searchBox->setText(word);
    submitSearch();
}

// Enter: saves the search, then checks the word in the Hash Table.
// If the word is not in the dictionary, shows fuzzy matches from the Trie.
void MainWindow::submitSearch()
{
    string word;

    if (!normalizeWord(ui->searchBox->text().toStdString(), word)) {
        ui->statusLabel->setText("Use letters a-z only.");
        return;
    }

    recordSearch(word);
    refreshHistoryList();

    int frequency = 0;

    if (table->search(word, frequency)) {
        ui->statusLabel->setText(
            "\"" + QString::fromStdString(word) + "\" is in the dictionary (frequency "
            + QString::number(frequency) + ")."
        );
        return;
    }

    // Not found: replace the suggestions with the closest spellings.
    FuzzyMatch candidates[100];
    FuzzyMatch results[5];

    int maxAllowed = getMaxEditDistance((int)word.length());
    int candidateCount = trie->getFuzzyMatches(word, candidates, 100, maxAllowed);
    int resultCount = rankFuzzyMatches(candidates, candidateCount, results, 5);

    ui->suggestionList->clear();

    for (int i = 0; i < resultCount; i++) {
        ui->suggestionList->addItem(QString::fromStdString(results[i].word));
    }

    if (resultCount == 0) {
        ui->statusLabel->setText("Not in the dictionary. No close spellings found.");
    }
    else {
        ui->statusLabel->setText("Not in the dictionary. Did you mean:");
    }
}

// Clicking a past search runs it again.
void MainWindow::selectHistoryItem(QListWidgetItem* item)
{
    if (item == nullptr) {
        return;
    }

    QString word = item->text();
    ui->searchBox->setText(word);
    submitSearch();
}

// Empties the history queue and the list on screen.
void MainWindow::clearHistory()
{
    history->clear();
    historyCursor = -1;
    refreshHistoryList();
}

// Saves a search unless it is empty or repeats the newest one.
void MainWindow::recordSearch(const string& search)
{
    historyCursor = -1;

    string newest;
    if (search.empty() || (history->rear(newest) && newest == search)) {
        return;
    }

    history->enqueue(search);
}

// Redraws the history list from the queue, newest first.
void MainWindow::refreshHistoryList()
{
    ui->historyList->clear();

    for (int i = 0; i < history->getSize(); i++) {
        string word;
        history->getFromNewest(i, word);
        ui->historyList->addItem(QString::fromStdString(word));
    }
}

// Puts a past search in the box; `recalling` stops updateSuggestions from resetting the cursor.
void MainWindow::showRecalled(const string& search)
{
    recalling = true;
    ui->searchBox->setText(QString::fromStdString(search));
    recalling = false;
}

// Up = older search, Down = newer search, past the newest = empty box.
bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->searchBox && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        if (key == Qt::Key_Up) {
            if (historyCursor + 1 < history->getSize()) {
                historyCursor++;
                string recalled;
                history->getFromNewest(historyCursor, recalled);
                showRecalled(recalled);
            }
            return true;  // Handled: do not pass Up to the search box.
        }

        if (key == Qt::Key_Down) {
            if (historyCursor >= 0) {
                historyCursor--;
                string recalled;  // Stays empty when cursor reaches -1.
                if (historyCursor >= 0) {
                    history->getFromNewest(historyCursor, recalled);
                }
                showRecalled(recalled);
            }
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
