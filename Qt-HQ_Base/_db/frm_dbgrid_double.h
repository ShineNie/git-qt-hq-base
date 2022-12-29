#ifndef FRM_DBGRID_DOUBLE_H
#define FRM_DBGRID_DOUBLE_H

#include <QWidget>
class QKeyEvent;
class QTableWidgetItem;

namespace Ui {
class frm_DbGrid_Double;
}

class frm_DbGrid_Double : public QWidget
{
    Q_OBJECT

public:
    explicit frm_DbGrid_Double(QWidget *parent = 0);
    ~frm_DbGrid_Double();

private slots:
    void on_btnHeaderLoad_clicked();
    void on_btnDataLoad_clicked();
    void on_btnClear_clicked();
    void on_btnClearContents_clicked();
    void on_btnGetData_clicked();
    void on_btnGetModified_clicked();
    void on_btnSave_clicked();
    void on_btnInsert_clicked();
    void on_btnAdd_clicked();
    void on_btnGetHeader_clicked();
    void on_btnModelCopy_clicked();
    void on_btnFillModel_clicked();
    void onGridMain_RowChanged(int iRow);
    void onGridMain_RowDeleteQuery(QKeyEvent *event);
    void onGridMain_RowDeleted();
    void onGridSub_RowDeleted();
    void on_oGridMain_itemChanged(QTableWidgetItem *item);
    void on_oGridSub_itemChanged(QTableWidgetItem *item);

private:
    Ui::frm_DbGrid_Double *ui;

    bool m_bIsLoading = false;
};

extern frm_DbGrid_Double *g_wGridDouble;

#endif // FRM_DBGRID_DOUBLE_H
