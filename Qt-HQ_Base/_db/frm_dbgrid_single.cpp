#include "frm_dbgrid_single.h"
#include "ui_frm_dbgrid_single.h"
#include "QMessageBox"
#include "QKeyEvent"
#include "hq_base.h"
#include "hq_base_db.h"
#include "hq_base_db_model.h"

frm_DbGrid_Single *g_wGridSingle = nullptr;

frm_DbGrid_Single::frm_DbGrid_Single(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frm_DbGrid_Single)
{
    ui->setupUi(this);
    //this->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    connect(ui->oGridMain, SIGNAL(sigSave()), this, SLOT(on_btnSave_clicked()));
    ui->txtModelName->setText("Base_Model");
    on_btnLoad_clicked();
}

frm_DbGrid_Single::~frm_DbGrid_Single()
{
    g_wGridSingle = nullptr;
    delete ui;
}

void frm_DbGrid_Single::on_btnInsert_clicked()
{
    ui->oGridMain->insertRow();//insert row at current row.
}

void frm_DbGrid_Single::on_btnAdd_clicked()
{
    ui->oGridMain->addRow();
}

void frm_DbGrid_Single::on_btnDelete_clicked()
{
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    QCoreApplication::sendEvent(ui->oGridMain, event);
}

void frm_DbGrid_Single::on_btnSave_clicked()
{
    QString sErr;

    QString sModified_Main = ui->oGridMain->f_GetModified();
    QJsonArray oArr;
    oArr.append(HQ_Base::f_Json_StringToJsonObj(sModified_Main));
    QString sModified_All = HQ_Base::f_Json_ToString(oArr, &sErr);
    if (!sErr.isEmpty())
    {
        QMessageBox::warning(this, "Data saving failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        ui->txtDebug->setText(sErr);
        return;
    }

    HQ_Base_DB *hq_base_db = new HQ_Base_DB();
    if (!hq_base_db->f_SaveAll(sModified_All, &sErr))
    {
        QMessageBox::warning(this, "Data saving failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        delete hq_base_db;
        ui->txtDebug->setText(sErr);
        return;
    }
    delete hq_base_db;

    int iCurRowMain = ui->oGridMain->currentRow();
    int iCurCol = ui->oGridMain->currentColumn();
    QString sGUID = ui->oGridMain->f_GetCellValue(iCurRowMain, "fGUID");
    on_btnLoad_clicked();
    ui->oGridMain->setCurrentCell(sGUID, iCurCol);
}

void frm_DbGrid_Single::on_btnLoad_clicked()
{
    m_bIsLoading = true;
    QString sErr;

    //Load model.
    QString sModelMain = HQ_Base_Db_Model::f_GetModel(ui->txtModelName->text(), &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "Model gotting failed.", sErr,
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

    //Load data.
    QString sSQL = HQ_Base_Db_Model::f_GetSqlByFk(ui->txtModelName->text(), ui->txtFkValue->text(), &sErr);
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
        QMessageBox::warning(this, "GridMain load failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    ui->oGridMain->f_FillContents(sContents, &sErr);
    if (sErr != "")
    {
        QMessageBox::warning(this, "The main grid fill its contents failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    ui->btnSave->setEnabled(false);
    m_bIsLoading = false;
}

void frm_DbGrid_Single::on_btnGetHeader_clicked()
{
    ui->txtDebug->setText(ui->oGridMain->f_GetHeader());
}

void frm_DbGrid_Single::on_btnGetData_clicked()
{
    ui->txtDebug->setText(ui->oGridMain->f_GetData());
}

void frm_DbGrid_Single::on_btnGetModify_clicked()
{
    ui->txtDebug->setText(ui->oGridMain->f_GetModified());
}

void frm_DbGrid_Single::on_oGridMain_itemChanged(QTableWidgetItem *item)
{
    Q_UNUSED(item);
    if (!m_bIsLoading)
    {
        ui->btnSave->setEnabled(true);
    }
}
