#include "FileScanner.h"
#include "FolderTrie.h"
#include <QThread>
#include <QCryptographicHash>
#include <QFile>

FileScanner::FileScanner(const QDir &path) {
    setPath(path);
}

void FileScanner::setPath(const QDir &path) {
    m_root = path;
}

void FileScanner::start() {
    QHash<qint64, QStringList> buckets;

    auto trie = new FolderTrie;
    for (const auto &path: m_forbiddenFolders) {
        QStringList cur = QDir::toNativeSeparators(path.path()).split(QDir::separator(),
                                                                      Qt::SkipEmptyParts);
        trie->addPath(cur);
    }
    QStringList rootPath = QDir::toNativeSeparators(m_root.path()).split(QDir::separator(),
                                                                         Qt::SkipEmptyParts);
    for (auto &nextThing: rootPath) {
        if (trie->checkHaveFolder(nextThing)) {
            trie->goNext(nextThing);
        } else {
            break;
        }
    }
    int trash = 0;
    travelThroughFolders(QDir(m_root), buckets, trie, trash);
    emit countedFiles(trash, m_total);
    delete trie;

    int cnt = 0;
    emit processedCount(cnt);
    for (auto &i: buckets) {
        cnt += (int) i.size();
        emit processedCount((int) (cnt / (double) m_cnt * 100));

        if (QThread::currentThread()->isInterruptionRequested()) {
            closeWorker();
            return;
        }
        if (i.isEmpty()) {
            Q_UNREACHABLE();
        }
        if (i.size() < 2) {
            emit newOneFile(i[0]);
            continue;
        }
        QHash<QByteArray, QStringList> curBuckets;
        for (auto &file: i) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                closeWorker();
                return;
            }
            curBuckets[calcFileHash(file)].emplace_back(file);
        }
        for (auto &bucket: curBuckets) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                closeWorker();
                return;
            }
            if (bucket.isEmpty()) {
                Q_UNREACHABLE();
            }
            if (bucket.size() < 2) {
                emit newOneFile(bucket[0]);
                continue;
            }

            if (m_checkHashOnly) {
                emit newEqualBucket(bucket, QFileInfo(bucket[0]).size());
                continue;
            }

            QHash<QString, QStringList> realBuckets;
            for (auto &file: bucket) {
                if (QThread::currentThread()->isInterruptionRequested()) {
                    closeWorker();
                    return;
                }
                bool found = false;
                for (auto &curBucket: realBuckets) {
                    if (QThread::currentThread()->isInterruptionRequested()) {
                        closeWorker();
                        return;
                    }
                    if (checkFiles(file, curBucket[0])) {
                        found = true;
                        curBucket.append(file);
                        break;
                    }
                }
                if (!found) {
                    realBuckets[file].push_back(file);
                }
            }

            for (auto &curBucket: realBuckets) {
                if (QThread::currentThread()->isInterruptionRequested()) {
                    closeWorker();
                    return;
                }
                if (curBucket.isEmpty()) {
                    Q_UNREACHABLE();
                }
                if (curBuckets.size() == 1) {
                    emit newOneFile(curBucket[0]);
                    continue;
                }
                emit newEqualBucket(curBucket, QFileInfo(curBucket[0]).size());
            }
        }
    }

    emit completed();
    QThread::currentThread()->quit();
}

QByteArray FileScanner::calcFileHash(QFile path) {
    if (path.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&path)) {
            return hash.result();
        }
    }
    return {};
}

void FileScanner::setCnt(int cnt) {
    m_cnt = cnt;
}

void FileScanner::setCheckHashOnly(bool checkHashOnly) {
    m_checkHashOnly = checkHashOnly;
}

void FileScanner::closeWorker() {
    emit completed();
    QThread::currentThread()->quit();
}

bool FileScanner::checkFiles(QFile f1, QFile f2) {
    if (!f1.open(QFile::ReadOnly) || !f2.open(QFile::ReadOnly)) {
        return false;
    }
    QByteArray data1, data2;
    data1 = f1.readAll(), data2 = f2.readAll();
    return data1 == data2;
}

void FileScanner::setForbiddenFolders(QVector<QDir> &forbidden) {
    m_forbiddenFolders = forbidden;
}

void FileScanner::travelThroughFolders(const QDir &curDir, QHash<qint64, QStringList> &buckets, FolderTrie *trie,
                                       int &total) {
    if (QThread::currentThread()->isInterruptionRequested()) {
        return;
    }
    for (const auto &nextThing: curDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
        QDir nextDir = QDir(curDir.path() + QDir::separator() + nextThing);
        if (QFileInfo(nextDir.path()).isFile()) {
            buckets[QFileInfo(nextDir.path()).size()].append(nextDir.path());
            total++;
            m_total += QFileInfo(nextDir.path()).size();
            emit processedCount((int) (total / (double) m_cnt * 100));
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

        travelThroughFolders(nextDir, buckets, trie, total);
        trie->setAnotherFolder(was);
    }
}
