#ifndef DUPES_CHECKING_FOLDERTRIE_H
#define DUPES_CHECKING_FOLDERTRIE_H

#include <QObject>
#include <QHash>

class FolderTrie {

    struct FolderTrieNode {
        QHash<QString, FolderTrieNode *> to{};
        FolderTrieNode *parent{};
        bool isTerm{};
    };

public:
    FolderTrie();

    explicit FolderTrie(const QVector<QStringList> &path);

    ~FolderTrie();

    void setAnotherFolder(bool anotherFolder);

    [[nodiscard]] bool anotherFolder() const;

    void addPath(const QStringList &path);

    bool checkHaveFolder(const QString &folder);

    bool isTerm();

    void goNext(const QString &next);

    void goToParent();

private:
    FolderTrieNode *m_root;
    bool m_anotherFolder;

    void addPath(FolderTrieNode *root, const QStringList &path, qsizetype ind);
};

#endif //DUPES_CHECKING_FOLDERTRIE_H
