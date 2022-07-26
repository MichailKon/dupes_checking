#include "FileCounter.h"
#include "FolderTrie.h"
#include <QDirIterator>
#include <QThread>

FileCounter::FileCounter(const QString &path) {
    setRoot(path);
}

void FileCounter::setRoot(const QString &root) {
    m_root = root;
}

void FileCounter::start() {
    auto trie = new FolderTrie;
    for (const auto &path: m_forbiddenFolders) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            closeWorker();
            return;
        }
        QStringList cur = QDir::toNativeSeparators(path.path()).split(QDir::separator(),
                                                                      Qt::SkipEmptyParts);
        trie->addPath(cur);
    }
    QStringList rootPath = QDir::toNativeSeparators(m_root).split(QDir::separator(),
                                                                  Qt::SkipEmptyParts);
    for (auto &nextThing: rootPath) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            closeWorker();
            return;
        }
        if (trie->checkHaveFolder(nextThing)) {
            trie->goNext(nextThing);
        } else {
            break;
        }
    }
    if (QThread::currentThread()->isInterruptionRequested()) {
        closeWorker();
        return;
    }
    QPair<int, qint64> res = travelThroughFolders(QDir(m_root), trie);

    emit completed(res.first, res.second);
    QThread::currentThread()->quit();
}

void FileCounter::setBadPaths(const QVector<QDir> &badPaths) {
    m_forbiddenFolders = badPaths;
}

QPair<int, qint64> FileCounter::travelThroughFolders(const QDir &curDir, FolderTrie *trie) {
    if (QThread::currentThread()->isInterruptionRequested()) {
        return {};
    }
    QPair<int, qint64> res;

    for (const auto &nextThing: curDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return res;
        }
        QDir nextDir = QDir(curDir.path() + QDir::separator() + nextThing);
        if (QFileInfo(nextDir.path()).isFile()) {
            res.first++, res.second += QFileInfo(nextDir.path()).size();
            continue;
        }

        bool was = trie->anotherFolder(), switched = false;
        if (!trie->anotherFolder()) {
            if (!trie->checkHaveFolder(nextThing)) {
                trie->setAnotherFolder(true);
            } else {
                trie->goNext(nextThing);
                if (trie->isTerm()) {
                    trie->goToParent();
                    continue;
                } else {
                    switched = true;
                }
            }
        }
        if (switched) {
            trie->goToParent();
        }

        auto tempRes = travelThroughFolders(nextDir, trie);
        res.first += tempRes.first, res.second += tempRes.second;
        trie->setAnotherFolder(was);
    }

    return res;
}

void FileCounter::closeWorker() {
    emit completed(0, 0);
    QThread::currentThread()->quit();
}
