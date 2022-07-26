#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "FileCounter.h"
#include "FileScanner.h"
#include "OptionsSelector.h"
#include <QFileDialog>
#include <QThread>
#include <QShortcut>
#include <QToolButton>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow), m_worker(nullptr) {
    ui->setupUi(this);

    connect(ui->pushButton_start, &QPushButton::clicked, this, &MainWindow::startSearch);
    ui->pushButton_start->setDisabled(true);
    ui->treeWidget->setHeaderHidden(true);

    auto selectFolderMenu = ui->menuBar->addMenu(tr("Выбрать папку"));
    auto selectFolderMenuAction = selectFolderMenu->addAction("Выбрать папку");
    auto settingsMenu = ui->menuBar->addMenu(tr("Настройки"));
    auto settingsAction = settingsMenu->addAction("Настройки");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::getSettings);
    connect(selectFolderMenuAction, &QAction::triggered, this, &MainWindow::selectDirectory);

    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MainWindow::deleteFile);

    m_deleteShortcut = new QShortcut(Qt::Key_Delete, this);
    connect(m_deleteShortcut, &QShortcut::activated, this, &MainWindow::deleteFile);
}

MainWindow::~MainWindow() {
    stopWorker();
    delete ui;
    delete m_deleteShortcut;
}

void MainWindow::selectDirectory() {
    QString path = QFileDialog::getExistingDirectory(this, "Choose folder", "",
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (path.isEmpty()) {
        return;
    }
    ui->label->setText("Подсчет количества файлов");
    ui->progressBar->setValue(0);
    stopWorker();
    m_worker = new QThread;

    ui->treeWidget->clear();
    ui->textBrowser->clear();

    ui->pushButton_start->setDisabled(true);
    m_root = path;
    auto counter = new FileCounter(path);
    counter->setBadPaths(m_forbiddenFolders);
    counter->moveToThread(m_worker);
    connect(m_worker, &QThread::started, counter, &FileCounter::start);
    connect(counter, &FileCounter::completed, this, &MainWindow::displayCountSum);
    connect(m_worker, &QThread::finished, counter, &QObject::deleteLater);
    connect(counter, &FileCounter::completed, [&]() {
        ui->pushButton_start->setEnabled(true);
        ui->label->setText("");
    });
    m_worker->start();

    ui->textBrowser->clear();
    ui->textBrowser->append(tr("Selected dir: %1").arg(path));
}

void MainWindow::stopWorker() {
    if (m_worker != nullptr && m_worker->isRunning()) {
        m_worker->requestInterruption();
        m_worker->wait();
    }
    delete m_worker;
    m_worker = nullptr;
}

void MainWindow::displayCountSum(int cnt, qint64 sum) {
    ui->textBrowser->append(tr("Total files: %1").arg(cnt));
    ui->textBrowser->append(tr("Total sum: %1").arg(bytes2human(sum)));
    m_cnt = cnt;
}

void MainWindow::startSearch() {
    ui->treeWidget->clear();
    ui->textBrowser->clear();
    ui->textBrowser->append(tr("Selected directory: %1").arg(m_root.path()));

    stopWorker();
    ui->progressBar->setValue(0);
    auto scanner = new FileScanner(m_root);
    scanner->setCnt(m_cnt);
    scanner->setCheckHashOnly(m_checkHashOnly);
    scanner->setForbiddenFolders(m_forbiddenFolders);
    m_worker = new QThread;
    scanner->moveToThread(m_worker);
    connect(m_worker, &QThread::started, scanner, &FileScanner::start);
    connect(m_worker, &QThread::finished, scanner, &QObject::deleteLater);
    connect(scanner, &FileScanner::newOneFile, this, &MainWindow::fileProcessed);
    connect(scanner, &FileScanner::newEqualBucket, this, &MainWindow::bucketProcessed);
    connect(scanner, &FileScanner::countedFiles, [&](int cnt, qint64 total){
        ui->label->setText(tr("Files: %1; Total sum: %2").arg(cnt).arg(bytes2human(total)));
    });
    connect(scanner, &FileScanner::completed, [&]() {
        ui->pushButton_start->setEnabled(true);
        ui->label->setText("Завершено");
    });
    connect(scanner, &FileScanner::processedCount, ui->progressBar, &QProgressBar::setValue);
    m_worker->start();
}

void MainWindow::fileProcessed(const QString &file) {
    ui->textBrowser->append(file);
}

void MainWindow::bucketProcessed(const QStringList &bucket, qint64 bucketSize) {
    auto item = new QTreeWidgetItem({tr("%1 files; %2").arg(bucket.size()).arg(bytes2human(bucketSize))});
    for (auto &i: bucket) {
        item->addChild(new QTreeWidgetItem({i}));
    }
    ui->treeWidget->addTopLevelItem(item);
}

void MainWindow::getSettings() {
    bool *ok = new bool;
    auto res = OptionsSelector::getSettings(m_checkHashOnly, m_forbiddenFolders, ok, this);
    if (!*ok) {
        return;
    }
    m_checkHashOnly = res.first;
    m_forbiddenFolders = res.second;
}

QString MainWindow::bytes2human(qint64 bytes) {
    QString res = "B";
    auto total = (qreal) bytes;
    const int NEXT_STEP = (1 << 10);
    if (total >= NEXT_STEP) {
        total /= NEXT_STEP;
        res = "KB";
    }
    if (total >= NEXT_STEP) {
        total /= NEXT_STEP;
        res = "MB";
    }
    if (total >= NEXT_STEP) {
        total /= NEXT_STEP;
        res = "GB";
    }
    if (total >= NEXT_STEP) {
        total /= NEXT_STEP;
        res = "TB";
    }

    return QString::number(total, 'f', 2) + " " + res;
}

void MainWindow::deleteFile() {
    auto cur = (QTreeWidget*) sender();
    auto items = cur->selectedItems();
    if (items.empty()) {
        return;
    }

    QMessageBox box(this);
    auto btnNo = box.addButton(QMessageBox::No);
    box.addButton(QMessageBox::Yes);
    box.setWindowTitle("Удаление");
    box.setText(tr("Вы действительно хотите удалить выбранные файлы? (%1 выбрано)").arg(items.size()));
    box.exec();
    if (box.clickedButton() == btnNo) {
        return;
    }
    for (auto &i : items) {
        i->setDisabled(true);
        QFile(i->text(0)).remove();
    }
}
