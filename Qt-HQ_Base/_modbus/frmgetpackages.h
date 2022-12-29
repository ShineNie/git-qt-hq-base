#ifndef FRMGETPACKAGES_H
#define FRMGETPACKAGES_H

#include <QWidget>
class QStandardItemModel;

namespace Ui {
class frmGetPackages;
}

class frmGetPackages : public QWidget
{
    Q_OBJECT

public:
    explicit frmGetPackages(QWidget *parent = 0);
    ~frmGetPackages();

private slots:
    void on_btnGo_clicked();
    void on_btnClear_clicked();
    void on_listCmd_activated(const QModelIndex &index);
    void on_listCmd_clicked(const QModelIndex &index);

private:
    Ui::frmGetPackages *ui;

    QStandardItemModel* m_ModelCmd = nullptr;
};

extern frmGetPackages *g_wPackages;

#endif // FRMGETPACKAGES_H
