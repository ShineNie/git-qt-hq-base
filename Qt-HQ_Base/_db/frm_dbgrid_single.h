#ifndef FRM_DBGRID_SINGLE_H
#define FRM_DBGRID_SINGLE_H

#include <QWidget>
class QTableWidgetItem;

namespace Ui {
class frm_DbGrid_Single;
}

class frm_DbGrid_Single : public QWidget
{
    Q_OBJECT

public:
    explicit frm_DbGrid_Single(QWidget *parent = 0);
    ~frm_DbGrid_Single();

private slots:
    void on_btnInsert_clicked();
    void on_btnAdd_clicked();
    void on_btnSave_clicked();
    void on_btnLoad_clicked();
    void on_btnGetHeader_clicked();
    void on_btnGetData_clicked();
    void on_btnGetModify_clicked();
    void on_btnDelete_clicked();
    void on_oGridMain_itemChanged(QTableWidgetItem *item);

private:
    Ui::frm_DbGrid_Single *ui;

    bool m_bIsLoading = false;
};

extern frm_DbGrid_Single *g_wGridSingle;

#endif // FRM_DBGRID_SINGLE_H
