#include "OptionsSelector.h"
#include "ui_OptionsSelector.h"
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>


OptionsSelector::OptionsSelector(QWidget *parent) :
        QDialog(parent), ui(new Ui::OptionsSelector) {
    ui->setupUi(this);
    setModal(true);
    connect(ui->pushButton_save, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &QDialog::reject);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &OptionsSelector::customContextMenuRequested, this, &OptionsSelector::showContextMenu);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &OptionsSelector::delString);
}

OptionsSelector::~OptionsSelector() {
    delete ui;
}

void OptionsSelector::setCheckHash(bool checkHash) {
    ui->checkBox->setChecked(checkHash);
}

void OptionsSelector::setLines(const QVector<QDir> &lines) {
    for (auto &i : lines) {
        ui->listWidget->addItem(i.path());
    }
}

QPair<bool, QVector<QDir>>
OptionsSelector::getSettings(bool checkHash, const QVector<QDir> &lines, bool *ok, QWidget *parent) {
    auto dialog = new OptionsSelector(parent);
    dialog->setCheckHash(checkHash);
    dialog->setLines(lines);
    const int ret = dialog->exec();
    if (ok) {
        *ok = (bool) ret;
    }
    QPair<bool, QVector<QDir>> res;
    if (ret) {
        res.first = dialog->ui->checkBox->isChecked();
        for (int i = 0; i < dialog->ui->listWidget->count(); i++) {
            res.second << QDir(dialog->ui->listWidget->item(i)->text()).path();
        }
    }
    dialog->deleteLater();
    return res;
}

void OptionsSelector::showContextMenu(const QPoint &pos) {
    QMenu contextMenu("Изменить папки", this);

    QAction actionAdd("Добавить", this);
    connect(&actionAdd, &QAction::triggered, this, &OptionsSelector::addString);
    contextMenu.addAction(&actionAdd);

    contextMenu.exec(mapToGlobal(pos));
}

void OptionsSelector::addString() {
    QString folder = QFileDialog::getExistingDirectory(this, "Выберите папку");
    if (folder.isEmpty()) {
        return;
    }
    ui->listWidget->addItem(folder);
}

void OptionsSelector::delString() {
    auto selected = ui->listWidget->selectedItems();
    if (selected.isEmpty()) {
        return addString();
    }
    QMessageBox box(this);
    auto btnNo = box.addButton(QMessageBox::No);
    box.addButton(QMessageBox::Yes);
    box.setWindowTitle("Удаление");
    box.setText(tr("Вы действительно хотите удалить выбранные директории? (%1 выбрано)").arg(selected.size()));
    box.exec();
    if (box.clickedButton() == btnNo) {
        return;
    }
    qDeleteAll(selected);
}
