#include "hq_base_db_page.h"
#include "ui_hq_base_db_page.h"
#include "hq_base.h"
#include "hq_base_db.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>

HQ_Base_DB_page::HQ_Base_DB_page(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HQ_Base_DB_page)
{
    ui->setupUi(this);
}

HQ_Base_DB_page::~HQ_Base_DB_page()
{
    delete ui;
}
void HQ_Base_DB_page::f_Init(QString sTable, QString sSort_Field)
{
    ui->txtDebug->setVisible(false);
    m_sTableName = sTable;
    m_sSortField = sSort_Field;

    m_bIsFilling = true;
    f_Query();
    m_bIsFilling = false;
}
void HQ_Base_DB_page::f_Query()
{
    if (m_bIsFilling)
    {
        m_iPageNum = 1;
    }

    //Get the sql by given info.
    QString sSQL = f_GetSQL_ByPageinfo(m_sTableName, m_iPageSize, m_iPageNum - 1, m_sSortField);
    ui->txtDebug->setText(sSQL);

    //Get the record set by sql.
    QString sErr;
    QString sJson = HQ_Base_DB::f_GetQueryAsJsonStr(sSQL, &sErr);
    if (!sErr.isEmpty())
    {
        QMessageBox::warning(this, "Quering failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    //ui->txtDebug->setText(sJson);

    //Send the result to outside.
    emit sigRecordSet(sJson);

    //Get the row count and page count.
    QJsonObject oJson = HQ_Base::f_Json_StringToJsonObj(sJson);
    QJsonArray oArr = oJson.value("Rows").toArray();
    if (oArr.size() < 1)
    {
        QMessageBox::warning(this, "Quering failed.", sErr,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    QJsonObject oJsonRow = oArr.at(0).toObject();
    QString sRowCount = oJsonRow.value("fRowCount").toString();
    QString sPageCount = oJsonRow.value("fPageCount").toString();
    m_iRowCount = sRowCount.toInt();
    m_iPageCount = sPageCount.toInt();

    //Init ui if it is necessary.
    if (m_bIsFilling)
    {
        //Updata ui.
        ui->labRowCount->setText(sRowCount);
        ui->labPageCount->setText(QString::number(m_iPageCount));
        ui->spPageSize->setValue(m_iPageSize);

        //Fill the combobox.
        QStringList sList;
        for (int i = 0; i < m_iPageCount; i++)
        {
            sList << QString::number(i + 1);
        }
        ui->cbPageNum->clear();
        ui->cbPageNum->addItems(sList);
        ui->cbPageNum->setCurrentIndex(0);
    }

    //Set the button enabled.
    ui->btnFirst->setEnabled(true);
    ui->btnPrev->setEnabled(true);
    ui->btnNext->setEnabled(true);
    ui->btnLast->setEnabled(true);
    if (m_iPageNum == 1)
    {
        ui->btnFirst->setEnabled(false);
        ui->btnPrev->setEnabled(false);
    }
    if (m_iPageNum == m_iPageCount)
    {
        ui->btnNext->setEnabled(false);
        ui->btnLast->setEnabled(false);
    }
}
/*===============================================================
 * Return a sql statement string with row number by given table
 * name and orderby string.  The field 'fGUID' is required as a
 * primary key. The row number field is named 'fRow'.
 *
 * The record will likes below:
 *      -----------------------------
 *       fRow | Field1 | Field2 |...
 *      -----------------------------
 *         0  |   v1   |   v2   |...
 *      -----------------------------
 *         1  |   v3   |   v4   |...
 *      -----------------------------
 *         2  |   v5   |   v6   |...
 *      -----------------------------
 *         3  |   v7   |   v8   |...
 *      -----------------------------
 *        ...
 *      -----------------------------
 *===============================================================*/
QString HQ_Base_DB_page::f_GetSQL_WithRowNum(QString sTable, QString sOrderBy)
{
    QString sSQL;
    if (sOrderBy.isEmpty())
    {
        sSQL = "select\n"
               "( select count(*) from %1 t1 where t1.fGUID < t.fGUID ) as fRow, *\n"
               "from %1 t\n"
               "order by fGUID";
        sSQL = sSQL.arg(sTable);
    }
    else
    {
        QString sSQL_Sub;
        QStringList list = sOrderBy.split(",");
        for (int i = 0; i < list.size(); i++)
        {
            QString s = QString("( select count(*) from %1 t%2 where 1=1\n")
                    .arg(sTable).arg(i);
            if (i > 0)
            {
                for (int j = 0; j < i; j++)
                {
                    s += QString("and t%1.%2 = t.%2\n").arg(i).arg(list.at(j));
                }
            }
            s += QString("and t%1.%2 < t.%2 )\n +").arg(i).arg(list.at(i));
            sSQL_Sub += s.replace("\n", "\n" + m_sIndent) + "\n";
        }

        QString s = QString("( select count(*) from %1 t%2 where 1=1\n")
                .arg(sTable).arg(list.size());
        for (int i = 0; i < list.size(); i++)
        {
            s += QString("and t%1.%2 = t.%2\n").arg(list.size()).arg(list.at(i));
        }
        s += QString("and t%1.fGUID < t.fGUID )").arg(list.size());
        sSQL_Sub += s.replace("\n", "\n" + m_sIndent);
        //-----------------------
        sSQL_Sub = m_sIndent + sSQL_Sub.replace("\n", "\n" + m_sIndent);
        sSQL = "select\n"
               "(\n%1\n) as fRow, *\n"
               "from %2 t\n"
               "order by %3,fGUID";
        sSQL = sSQL.arg(sSQL_Sub).arg(sTable).arg(sOrderBy);
    }

    return sSQL;
}
/*===============================================================
 * Return a sql string with page info by given table name and
 * orderby string. The page info fields contains:
 * 'fPageNum', 'fPageCount', 'fRowCount'.
 *===============================================================*/
QString HQ_Base_DB_page::f_GetSQL_WithPageInfo(QString sTable, int iPageSize,
                                               QString sOrderBy)
{
    QString sSQL, sSQL_PageCount, sSQL_RowCount, sSQL_WithRowNum;

    sSQL_WithRowNum = f_GetSQL_WithRowNum(sTable, sOrderBy);
    sSQL_WithRowNum = m_sIndent + sSQL_WithRowNum.replace("\n", "\n" + m_sIndent);

    sSQL_PageCount  = "select (max(fRow) / %1) + 1 from\n(\n%2\n)";
    sSQL_PageCount  = sSQL_PageCount.arg(iPageSize).arg(sSQL_WithRowNum);
    sSQL_PageCount  = m_sIndent + sSQL_PageCount.replace("\n", "\n" + m_sIndent);

    sSQL_RowCount   = "select count(*) from\n(\n%1\n)";
    sSQL_RowCount   = sSQL_RowCount.arg(sSQL_WithRowNum);
    sSQL_RowCount   = m_sIndent + sSQL_RowCount.replace("\n", "\n" + m_sIndent);

    sSQL = "select\n\n"
           "(\n%1\n) as fPageCount,\n\n"
           "( fRow / %2 ) as fPageNum,\n\n"
           "(\n%3\n) as fRowCount, *\n\n"
           "from\n(\n%4\n)\n";
    sSQL = sSQL.arg(sSQL_PageCount).arg(iPageSize).arg(sSQL_RowCount).arg(sSQL_WithRowNum);
    return sSQL;
}
/*===============================================================
 * Return a sql string by given table info and page info.
 * The table info contains table name, orderby string.
 * The page info contains page-size, page-num.
 *===============================================================*/
QString HQ_Base_DB_page::f_GetSQL_ByPageinfo(QString sTable, int iPageSize,
                                             int iPageNum, QString sOrderBy)
{
    QString sSQL_WithPageInfo = f_GetSQL_WithPageInfo(sTable, iPageSize, sOrderBy);
    return sSQL_WithPageInfo + "where fPageNum = " + QString::number(iPageNum);
}
/*===============================================================
 * Slots.
 *===============================================================*/
void HQ_Base_DB_page::on_btnFirst_clicked()
{
    ui->cbPageNum->setCurrentIndex(0);
}

void HQ_Base_DB_page::on_btnPrev_clicked()
{
    int iPageNum_Prev = m_iPageNum - 1;
    ui->cbPageNum->setCurrentIndex(iPageNum_Prev - 1);
}

void HQ_Base_DB_page::on_cbPageNum_currentIndexChanged(const QString &arg1)
{
    if (m_bIsFilling)
    {
        return;
    }
    m_iPageNum = arg1.toInt();
    f_Query();
}

void HQ_Base_DB_page::on_btnNext_clicked()
{
    int iPageNum_Next = m_iPageNum + 1;
    ui->cbPageNum->setCurrentIndex(iPageNum_Next - 1);
}

void HQ_Base_DB_page::on_btnLast_clicked()
{
    ui->cbPageNum->setCurrentIndex(m_iPageCount - 1);
}

void HQ_Base_DB_page::on_spPageSize_valueChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    if (m_bIsFilling)
    {
        return;
    }
    m_iPageSize = ui->spPageSize->value();
    m_bIsFilling = true;
    f_Query();
    m_bIsFilling = false;
}

void HQ_Base_DB_page::on_btnDebug_clicked()
{
    ui->txtDebug->setVisible(!ui->txtDebug->isVisible());
    ui->btnDebug->setText(ui->txtDebug->isVisible() ?
                          QString("Debug%1").arg("<<") :
                          QString("Debug%1").arg(">>"));
}
