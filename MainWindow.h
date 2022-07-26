#ifndef DUPES_CHECKING_MAINWINDOW_H
#define DUPES_CHECKING_MAINWINDOW_H

#include <QWidget>
#include <QDir>

class FileCounter;

class QShortcut;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    void selectDirectory();

    void displayCountSum(int cnt, qint64 sum);

    void startSearch();

    void fileProcessed(const QString &file);

    void bucketProcessed(const QStringList &bucket, qint64 bucketSize);

    void getSettings();

    void deleteFile();

private:
    Ui::MainWindow *ui;
    QThread *m_worker;
    QDir m_root;
    int m_cnt{};
    bool m_checkHashOnly{};
    QVector<QDir> m_forbiddenFolders;

    QShortcut *m_deleteShortcut;

    void stopWorker();

    static QString bytes2human(qint64 bytes);
};


#endif //DUPES_CHECKING_MAINWINDOW_H
