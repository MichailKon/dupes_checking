#include "FolderTrie.h"

void FolderTrie::addPath(const QVector<QString> &path) {
    addPath(m_root, path, 0);
}

FolderTrie::FolderTrie(const QVector<QStringList> &path) {
    m_root = new FolderTrieNode;
    for (const auto &list: path) {
        addPath(list);
    }
}

FolderTrie::FolderTrie() {
    m_root = new FolderTrieNode;
}

FolderTrie::~FolderTrie() {
    delete m_root;
}

void FolderTrie::addPath(FolderTrie::FolderTrieNode *root, const QStringList &path, qsizetype ind) {
    if (ind == path.size()) {
        root->isTerm = true;
        return;
    }
    if (!root->to.contains(path[ind])) {
        root->to[path[ind]] = new FolderTrieNode;
        root->to[path[ind]]->parent = root;
    }
    addPath(root->to[path[ind]], path, ind + 1);
}

bool FolderTrie::anotherFolder() const {
    return m_anotherFolder;
}

void FolderTrie::setAnotherFolder(bool anotherFolder) {
    m_anotherFolder = anotherFolder;
}

bool FolderTrie::checkHaveFolder(const QString &folder) {
    return m_root->to.contains(folder);
}

bool FolderTrie::isTerm() {
    return m_root->isTerm;
}

void FolderTrie::goNext(const QString &next) {
    assert(checkHaveFolder(next));
    m_root = m_root->to[next];
}

void FolderTrie::goToParent() {
    m_root = m_root->parent;
    assert(m_root != nullptr);
}
