#ifndef DUPES_CHECKING_OPTIONSSELECTOR_H
#define DUPES_CHECKING_OPTIONSSELECTOR_H

#include <QDialog>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class OptionsSelector; }
QT_END_NAMESPACE

class OptionsSelector : public QDialog {
Q_OBJECT

public:
    explicit OptionsSelector(QWidget *parent = nullptr);

    ~OptionsSelector() override;

    static QPair<bool, QVector<QDir>>
    getSettings(bool checkHash, const QVector<QDir> &lines, bool *ok, QWidget *parent = nullptr);

    void setCheckHash(bool checkHash);

    void setLines(const QVector<QDir> &lines);

private slots:

    void showContextMenu(const QPoint &pos);

    void addString();

    void delString();

private:
    Ui::OptionsSelector *ui;
};


#endif //DUPES_CHECKING_OPTIONSSELECTOR_H
