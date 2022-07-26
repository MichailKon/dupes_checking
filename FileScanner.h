#ifndef DUPES_CHECKING_FILESCANNER_H
#define DUPES_CHECKING_FILESCANNER_H

#include <QObject>
#include <QDir>

class FolderTrie;

class FileScanner : public QObject {
Q_OBJECT

public:
    explicit FileScanner(const QDir &path);

    void setPath(const QDir &path);

    void setCnt(int cnt);

    void setCheckHashOnly(bool checkHashOnly);

    void setForbiddenFolders(QVector<QDir> &forbidden);

public slots:

    void start();

signals:

    void completed();

    void newEqualBucket(const QStringList &bucket, qint64 bucketSize);

    void newOneFile(const QString &file);

    void processedCount(double cnt);

    void countedFiles(int cnt, qint64 total);

private slots:

    void closeWorker();

private:
    QDir m_root;
    int m_cnt{};
    bool m_checkHashOnly{};
    QVector<QDir> m_forbiddenFolders;
    qint64 m_total{};

    static QByteArray calcFileHash(QFile path);

    static bool checkFiles(QFile f1, QFile f2);

    void travelThroughFolders(const QDir &curDir, QHash<qint64, QStringList> &buckets, FolderTrie *trie, int &total);
};

#endif //DUPES_CHECKING_FILESCANNER_H
