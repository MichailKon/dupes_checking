#ifndef DUPES_CHECKING_FILECOUNTER_H
#define DUPES_CHECKING_FILECOUNTER_H

#include <QObject>
#include <QDir>

class FolderTrie;

class FileCounter : public QObject {
Q_OBJECT

public:
    explicit FileCounter(const QString &path);

public:
    void setRoot(const QString &root);

    void setBadPaths(const QVector<QDir> &badPaths);

signals:

    void completed(int cnt, qint64 sum);

public slots:

    void start();

private:
    QString m_root{};
    QVector<QDir> m_forbiddenFolders;

    QPair<int, qint64> travelThroughFolders(const QDir &curDir, FolderTrie *trie);

    void closeWorker();
};

#endif //DUPES_CHECKING_FILECOUNTER_H
