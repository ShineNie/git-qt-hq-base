#include "frm_dbgrid_double.h"
#include "ui_frm_dbgrid_double.h"
#include "QMessageBox"
#include "QKeyEvent"
#include "hq_base.h"
#include "hq_base_db.h"
#include "hq_base_db_model.h"

frm_DbGrid_Double *g_wGridDouble = nullptr;

frm_DbGrid_Double::frm_DbGrid_Double(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frm_DbGrid_Double)
{
    ui->setupUi(this);
    //this->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    ui->txtModelMain->setText("Base_Model");
    ui->txtModelSub->setText("Base_Model_Sub");
    connect(ui->oGridMain, SIGNAL(sigRowChanged(int)), this, SLOT(onGridMain_RowChanged(int)));
    connect(ui->oGridMain, SIGNAL(sigRowDeleteQuery(QKeyEvent*)),
            this, SLOT(onGridMain_RowDeleteQuery(QKeyEvent*)), Qt::UniqueConnection);
    connect(ui->oGridMain, SIGNAL(sigRowDeleted()), this, SLOT(onGridMain_RowDeleted()));
    connect(ui->oGridSub, SIGNAL(sigRowDeleted()), this, SLOT(onGridSub_RowDeleted()));
    connect(ui->oGridMain, SIGNAL(sigSave()), this, SLOT(on_btnSave_clicked()));
    connect(ui->oGridSub, SIGNAL(sigSave()), this, SLOT(on_btnSave_clicked()));

    on_btnHeaderLoad_clicked();
    on_btnDataLoad_clicked();
}

frm_DbGrid_Double::~frm_DbGrid_Double()
{
    g_wGridDouble = nullptr;
    delete ui;
}

void frm_DbGrid_Double::on_btnHeaderLoad_clicked()
{
    QString sErr;
    QString sModelMain = HQ_Base_Db_Model::f_GetModel(ui->txtModelMain->text(), &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "ModelMain gotting failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    QString sModelSub = HQ_Base_Db_Model::f_GetModel(ui->txtModelSub->text(), &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "ModelSub gotting failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    ui->oGridMain->f_FillHeader(sModelMain, &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "The main grid filling its header failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    ui->oGridSub->f_FillHeader(sModelSub, &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "The sub grid filling its header failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    ui->txtJsonMain->setText(sModelMain);
    ui->txtJsonSub->setText(sModelSub);
}

void frm_DbGrid_Double::on_btnDataLoad_clicked()
{
    m_bIsLoading = true;
    QString sErr;
    QString sSQL = HQ_Base_Db_Model::f_GetSqlByFk(ui->txtModelMain->text(), &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "Sql statement gotting failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    HQ_Base_DB *hq_base_db = new HQ_Base_DB();
    QString sContents = hq_base_db->f_GetQueryAsJsonStr(sSQL, &sErr);
    delete hq_base_db;
    if (sErr != "")
    {
        QMessageBox::warning(this, "GridMain loading failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    ui->oGridMain->f_FillContents(sContents, &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "The main grid filling its contents failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    ui->oGridSub->clearContents();
    ui->btnSave->setEnabled(false);
    m_bIsLoading = false;
}

void frm_DbGrid_Double::onGridMain_RowChanged(int iRow)
{
    QString sCurID = ui->oGridMain->f_GetCellValue(iRow, "fGUID");
    QString sErr;
    QString sSQL = HQ_Base_Db_Model::f_GetSqlByFk(ui->txtModelSub->text(), sCurID, &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "Sql statement gotting failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    HQ_Base_DB *hq_base_db = new HQ_Base_DB();
    QString sContents = hq_base_db->f_GetQueryAsJsonStr(sSQL, &sErr);
    delete hq_base_db;
    if (sErr != "")
    {
        QMessageBox::warning(this, "GridSub loading failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    m_bIsLoading = true;
    ui->oGridSub->f_FillContents(sContents, &sErr);
    ui->oGridSub->f_SetForeignKeyValue(sCurID);
    m_bIsLoading = false;
    if (sErr != "")
    {
        QMessageBox::warning(this, "The sub grid filling its contents failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
}
void frm_DbGrid_Double::onGridMain_RowDeleteQuery(QKeyEvent *event)
{
    if (ui->oGridSub->rowCount() > 0)
    {
        QMessageBox::warning(this, "The main grid deleting its row failed.",
                             "Delete rows in sub gird at first and try again.",
                                 QMessageBox::Ok,QMessageBox::NoButton);
        event->ignore();
    }
    else
    {
        event->accept();
    }
}

void frm_DbGrid_Double::on_btnClear_clicked()
{
    ui->oGridMain->clear();
    ui->oGridSub->clear();
    ui->txtJsonMain->clear();
    ui->txtJsonSub->clear();
}

void frm_DbGrid_Double::on_btnClearContents_clicked()
{
    ui->oGridMain->clearContents();
    ui->oGridSub->clearContents();
}

void frm_DbGrid_Double::on_btnGetHeader_clicked()
{
    ui->txtJsonMain->setText(ui->oGridMain->f_GetHeader());
    ui->txtJsonSub->setText(ui->oGridSub->f_GetHeader());
}

void frm_DbGrid_Double::on_btnGetData_clicked()
{
    ui->txtJsonMain->setText(ui->oGridMain->f_GetData());
    ui->txtJsonSub->setText(ui->oGridSub->f_GetData());
}

void frm_DbGrid_Double::on_btnGetModified_clicked()
{
    ui->txtJsonMain->setText(ui->oGridMain->f_GetModified());
    ui->txtJsonSub->setText(ui->oGridSub->f_GetModified());
}

void frm_DbGrid_Double::on_btnSave_clicked()
{
    QString sErr;

    QString sModified_Main = ui->oGridMain->f_GetModified();
    QString sModified_Sub = ui->oGridSub->f_GetModified();
    QJsonArray oArr;
    oArr.append(HQ_Base::f_Json_StringToJsonObj(sModified_Main));
    oArr.append(HQ_Base::f_Json_StringToJsonObj(sModified_Sub));
    QString sModified_All = HQ_Base::f_Json_ToString(oArr, &sErr);
    ui->txtJsonMain->setText(sModified_All);
    if (!sErr.isEmpty())
    {
        QMessageBox::warning(this, "Data saving failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        ui->txtJsonSub->setText(sErr);
        return;
    }

    HQ_Base_DB *hq_base_db = new HQ_Base_DB();
    if (!hq_base_db->f_SaveAll(sModified_All, &sErr))
    {
        QMessageBox::warning(this, "Data saving failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        delete hq_base_db;
        ui->txtJsonSub->setText(sErr);
        return;
    }
    delete hq_base_db;

    int iCurRowMain = ui->oGridMain->currentRow();
    int iCurColMain = ui->oGridMain->currentColumn();
    int iCurRowSub = ui->oGridSub->currentRow();
    int iCurColSub = ui->oGridSub->currentColumn();
    on_btnDataLoad_clicked();
    ui->oGridMain->setCurrentCell(iCurRowMain, iCurColMain);
    ui->oGridSub->setCurrentCell(iCurRowSub, iCurColSub);
}

void frm_DbGrid_Double::on_btnInsert_clicked()
{
    if (ui->splitter->focusWidget() == ui->oGridMain)
    {
        int iCurRow = ui->oGridMain->currentRow();
        ui->txtJsonMain->setText(QString::number(iCurRow));
        ui->oGridMain->insertRow();//insert row at current row.
    }
    if (ui->splitter->focusWidget() == ui->oGridSub)
    {
        if (ui->oGridSub->f_GetForeignKeyValue().isEmpty())
        {
            QMessageBox::warning(this, "Sub data inserting failed.", "Select a row in the main gird first.",
                              QMessageBox::Ok,QMessageBox::NoButton);
            return;
        }
        int iCurRow = ui->oGridSub->currentRow();
        ui->txtJsonSub->setText(QString::number(iCurRow));
        ui->oGridSub->insertRow();//insert row at current row.
    }
}

void frm_DbGrid_Double::on_btnAdd_clicked()
{

    if (ui->splitter->focusWidget() == ui->oGridMain)
    {
        int iCurRow = ui->oGridMain->currentRow();
        ui->txtJsonMain->setText(QString::number(iCurRow));
        //ui->oGridMain->insertRow(ui->oGridMain->rowCount());//insert row at the end.
        ui->oGridMain->addRow();
    }
    if (ui->splitter->focusWidget() == ui->oGridSub)
    {
        if (ui->oGridSub->f_GetForeignKeyValue().isEmpty())
        {
            QMessageBox::warning(this, "Sub data adding failed.", "Select a row in the main gird first.",
                              QMessageBox::Ok,QMessageBox::NoButton);
            return;
        }
        int iCurRow = ui->oGridSub->currentRow();
        ui->txtJsonSub->setText(QString::number(iCurRow));
        //ui->oGridSub->insertRow(ui->oGridSub->rowCount());//insert row at the end.
        ui->oGridSub->addRow();
    }
}

void frm_DbGrid_Double::on_btnModelCopy_clicked()
{
    /* Before continue the model copying, a row in main grid must to be selected.
     * Then the sub gird will has its contents.
     */
    int iCurRow = ui->oGridMain->currentRow();
    if (iCurRow < 0 || iCurRow > (ui->oGridMain->rowCount() - 1))
    {
        QMessageBox::warning(this, "Model copying failed.", "Select a row in the main gird first.",
                          QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }

    //Copy the selected row.
    QJsonArray oArrRows = ui->oGridMain->f_GetRows(iCurRow, 1);
    ui->oGridMain->addRow(oArrRows);

    //Get the index of new row.
    iCurRow = ui->oGridMain->rowCount() - 1;

    /* This two rows of code were used for the model manager only.
     * If this class will used for other working, the field name
     * 'fModeName' may be not appropriate.
     */
    QString sCurModelName = ui->oGridMain->f_GetCellValue(iCurRow, "fModelName");
    ui->oGridMain->f_SetCellValue(iCurRow, "fModelName", sCurModelName + " copy", false);

    //Get the foreignkey of sub grid.
    QString sID = ui->oGridMain->f_GetCellValue(iCurRow, "fGUID");

    //Get the sub datas of the foreignkey used for copying.
    oArrRows = ui->oGridSub->f_GetRows(0, ui->oGridSub->rowCount());

    //Let the main grid select the new row added before.
    //And the sub grid will be empty.
    ui->oGridMain->selectRow(iCurRow);

    //Give the foreignkey to sub grid, and add the new rows copied before to the sub grid.
    ui->oGridSub->f_SetForeignKeyValue(sID);
    ui->oGridSub->addRow(oArrRows);

    //Save the data modified, and refresh the ui.
    on_btnSave_clicked();
}

void frm_DbGrid_Double::on_btnFillModel_clicked()
{
    QString sErr;

    int iCurRow_Main = ui->oGridMain->currentRow();
    if (iCurRow_Main < 0)
    {
        QMessageBox::warning(this, "Sub grid generating failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }

    QString sModelMain = ui->oGridMain->f_GetCellValue(iCurRow_Main, "fModelName");

    QJsonArray oArr = HQ_Base_Db_Model::f_GetColModel(sModelMain, &sErr);
    if (oArr.isEmpty())
    {
        QMessageBox::warning(this, "Sub grid generating failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }

    QString sID = ui->oGridMain->f_GetCellValue(iCurRow_Main, "fGUID");
    ui->oGridSub->f_SetForeignKeyValue(sID);
    ui->oGridSub->addRow(oArr);

    //Save the data modified, and refresh the ui.
    on_btnSave_clicked();
}

void frm_DbGrid_Double::on_oGridMain_itemChanged(QTableWidgetItem *item)
{
    Q_UNUSED(item);
    if (!m_bIsLoading)
    {
        ui->btnSave->setEnabled(true);
    }
}

void frm_DbGrid_Double::on_oGridSub_itemChanged(QTableWidgetItem *item)
{
    Q_UNUSED(item);
    if (!m_bIsLoading)
    {
        ui->btnSave->setEnabled(true);
    }
}
void frm_DbGrid_Double::onGridMain_RowDeleted()
{
    ui->btnSave->setEnabled(true);
}
void frm_DbGrid_Double::onGridSub_RowDeleted()
{
    ui->btnSave->setEnabled(true);
}
