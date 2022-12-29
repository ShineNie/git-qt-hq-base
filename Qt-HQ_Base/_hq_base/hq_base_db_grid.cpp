#include "hq_base_db_grid.h"
#include <QUuid>
#include <QKeyEvent>
#include <QDebug>
#include "hq_base.h"

/*================================================================
 * Overload the constructor
 *===============================================================*/
HQ_Base_Db_Grid::HQ_Base_Db_Grid(QWidget *parent) : QTableWidget(parent)
{
    /* It is not necessary to set the style-sheet here. The style-sheet should be set
     * in the constructor of the main window of the application.
     * setStyleSheet("QTableWidget:!hover{selection-background-color:rgb(0, 255, 255);selection-color:black;}"
     *               "QTableWidget::item:hover{background-color:rgb(170, 255, 255);color:black;}");
     */

    setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(this, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(on_itemChanged(QTableWidgetItem*)));
    connect(this, SIGNAL(currentCellChanged(int,int,int,int)),
            this, SLOT(on_currentCellChanged(int,int,int,int)));
}
/*================================================================
 * Clear all the columns and rows.(override)
 *===============================================================*/
void HQ_Base_Db_Grid::clear()
{
    //Free memory of all the item objects of every column.
    int iColCount = this->columnCount();
    for (int iCol = 0; iCol < iColCount; iCol++)
    {
        QTableWidgetItem *item = this->horizontalHeaderItem(iCol);
        delete item;
    }

    //Free memory of all the item objects of contents.
    int iRowCount = this->rowCount();
    for (int iRow = 0; iRow < iRowCount; iRow++)
    {
        int iColCount = this->columnCount();
        for (int iCol = 0; iCol < iColCount; iCol++)
        {
            QTableWidgetItem *item = this->item(iRow, iCol);
            delete item;
        }
    }

    //Update ui.
    QTableWidget::clear();
    this->setRowCount(0);
    this->setColumnCount(0);
}
/*================================================================
 * Clear all the rows except header.(override)
 *===============================================================*/
void HQ_Base_Db_Grid::clearContents()
{
    //Free memory of all the item objects of contents.
    int iRowCount = this->rowCount();
    for (int iRow = 0; iRow < iRowCount; iRow++)
    {
        int iColCount = this->columnCount();
        for (int iCol = 0; iCol < iColCount; iCol++)
        {
            QTableWidgetItem *item = this->item(iRow, iCol);
            delete item;
        }
    }
    m_oJsonContents = QJsonObject();
    m_sForeignKeyValue = "";

    //Update ui.
    QTableWidget::clearContents();
    this->setRowCount(0);
}
/*================================================================
 * Remove row.(override and overload)
 *===============================================================*/
void HQ_Base_Db_Grid::removeRow(int row)
{
    //Free memory of all the items of every column.
    int iColCount = this->columnCount();
    for (int iCol = 0; iCol < iColCount; iCol++)
    {
        QTableWidgetItem *item = this->item(row, iCol);
        delete item;
    }

    //Update ui.
    QTableWidget::removeRow(row);

    //Update the json data.
    f_SetRowState(row, "-1");

    emit sigRowDeleted();
}
void HQ_Base_Db_Grid::removeRow(QString sGUID)
{
    for (int iRow = 0; iRow < this->rowCount(); iRow++)
    {
        QString sCurID = this->f_GetCellValue(iRow, "fGUID");
        if (HQ_Base::f_GUID_GetCommon(sCurID) == HQ_Base::f_GUID_GetCommon(sGUID))
        {
            removeRow(iRow);
        }
    }
}
/*================================================================
 * insert row.(override and overload)
 *
 * If the given row index is valid, It will insert a new row at the
 * position of the given row index. Otherwise, it will add a new
 * row at the end of the grid.
 *
 * If there is no given row index, it will insert a new row at the
 * current row index.
 *
 * If you will add a new row at the end of the grid, provide an
 * invalid row index or a value equal to the row count.
 * Such as :
 *
 *  insertRow(this->rowCount());
 *
 * The above code can add a new row at the end.
 *===============================================================*/
void HQ_Base_Db_Grid::insertRow(int row)
{
    int iCurRow = row;

    if (this->rowCount() > 0)
    {
        if (row > 0 && row <= this->rowCount())
        {
            //Given row index is valid, or add a row at the end.
            iCurRow = row;
        }
        else
        {
            //Given row index is invalid.
            iCurRow = 0;
        }
    }
    else//rowCount = 0
    {
        iCurRow = 0;
    }

    QTableWidget::insertRow(iCurRow);
    f_SetRowState(iCurRow, "0");
    this->selectRow(iCurRow);
    this->setFocus();
}
void HQ_Base_Db_Grid::insertRow()
{
    insertRow(this->currentRow());
}
/*===============================================================
 * Create a new row as json object by given row index, and fill
 * it to the grid.
 *===============================================================*/
QJsonObject HQ_Base_Db_Grid::f_InsertJsonRow(int iRow)
{
    QJsonObject oJsonNewRow;
    f_MadeNewRowJson(&oJsonNewRow, iRow);
    f_FillRowByJsonObject(iRow, oJsonNewRow);
    return oJsonNewRow;
}
/*===============================================================
 * Made the json object value of a new row.
 * The guid will be created automatically.
 * The fk_value may be not a guid, so it may be not the parent id,
 * it may be any string.
 * The row state will be 0.
 * The argument 'oJsonNewRow' can output the json object pointer,
 * its input value is required.
 *===============================================================*/
void HQ_Base_Db_Grid::f_MadeNewRowJson(QJsonObject *oJsonNewRow, int iRow)
{
    if (nullptr == oJsonNewRow)
    {
        return;
    }

    oJsonNewRow->insert("fState", "0");
    oJsonNewRow->insert("fRow", QString::number(iRow));

    QString sNewGUID = HQ_Base::f_GUID_GetCommon(QUuid::createUuid().toString());
    oJsonNewRow->insert("fGUID", sNewGUID);

    if (!m_sForeignKeyValue.isEmpty())
    {
        QString sFK = m_oJsonHeader.value("FK").toString();
        QString sFK_Value = m_sForeignKeyValue;
        oJsonNewRow->insert(sFK, sFK_Value);
    }
}
/*===============================================================
 * Add a new row.(override and overload)
 * It will add a new row after the given row index.
 *===============================================================*/
void HQ_Base_Db_Grid::addRow(int row)
{
    insertRow(row + 1);
}
void HQ_Base_Db_Grid::addRow()
{
    addRow(this->currentRow());
}
/*===============================================================
 * Add rows to grid by given json array.
 *
 * Json array format:
 *
 *  [
 *      {"Field1":"value","Field2":"value",...},
 *      {"Field1":"value","Field2":"value",...},
 *      ...
 *  ]
 *===============================================================*/
void HQ_Base_Db_Grid::addRow(QJsonArray oArrNewRows)
{
    if (oArrNewRows.isEmpty())
    {
        return;
    }

    //Add the json row object to data.
    QJsonArray oArrRows = m_oJsonContents.value("Rows").toArray();
    int iNewRowIndex = this->rowCount();
    for (int iRow = 0; iRow < oArrNewRows.size(); iRow++)
    {
        QJsonObject oJsonNewRow = oArrNewRows.at(iRow).toObject();
        f_MadeNewRowJson(&oJsonNewRow, iNewRowIndex);
        oArrRows.append(oJsonNewRow);

        //Add the new row to grid.
        QTableWidget::insertRow(iNewRowIndex);
        f_FillRowByJsonObject(iNewRowIndex, oJsonNewRow);

        iNewRowIndex++;
    }

    QJsonObject oJson_New;
    oJson_New.insert("Cols", m_oJsonContents.value("Cols").toString());
    oJson_New.insert("Rows", oArrRows);

    m_oJsonContents = oJson_New;
}
/*===============================================================
 * Return rows as json array by given row index and count.
 *
 * Json array format:
 *
 *  [
 *      {"Field1":"value","Field2":"value",...},
 *      {"Field1":"value","Field2":"value",...},
 *      ...
 *  ]
 *===============================================================*/
QJsonArray HQ_Base_Db_Grid::f_GetRows(int iRowStart, int iCount) const
{
    QJsonArray oArrRows_Return;

    if (iCount < 1)
    {
        return oArrRows_Return;
    }

    if (iRowStart > 0 || iRowStart < this->rowCount())
    {
        QJsonArray oArrRows = m_oJsonContents.value("Rows").toArray();
        for (int iRow = 0; iRow < oArrRows.size(); iRow++)
        {
            QJsonObject oJsonRow = oArrRows.at(iRow).toObject();
            int iCurRow = oJsonRow.value("fRow").toString().toInt();
            if(iCurRow >= iRowStart && iCurRow < (iRowStart + iCount))
            {
                oArrRows_Return.append(oJsonRow);
            }
        }
    }
    return oArrRows_Return;
}
/*================================================================
 * Return the number of columns visibled.
 *===============================================================*/
int HQ_Base_Db_Grid::f_ColumnCount_Visible() const
{
    int iCount = 0;
    for (int i = 0; i < this->columnCount(); i++)
    {
        if (!this->isColumnHidden(i))
        {
            iCount++;
        }
    }
    return iCount;
}
/*================================================================
 * Create the grid header by given json.
 * The json argument is a json object or string type.
 *===============================================================*/
bool HQ_Base_Db_Grid::f_FillHeader(QJsonObject oJsonModel, QString *sErr)
{
    QString sErrPrefix = "Grid header filling failed.";

    if (oJsonModel.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For given model being empty.");
        return false;
    }

    m_bIsRowStateProtected = true;
    bool bReturn;

    try
    {
        m_oJsonHeader = oJsonModel;

        QJsonArray oArr;
        oArr = m_oJsonHeader.value("Rows").toArray();
        int iColCount = oArr.size();

        QTableWidgetItem *item;
        this->clear();
        this->setColumnCount(iColCount);
        for (int iCol = 0; iCol < iColCount; iCol++)
        {
            item = new QTableWidgetItem();

            QString sColText   = oArr.at(iCol).toObject().value("fColText"     ).toString();
            QString sField     = oArr.at(iCol).toObject().value("fColField"    ).toString();
            QString sWidth     = oArr.at(iCol).toObject().value("fColWidth"    ).toString();
            QString sWidthAuto = oArr.at(iCol).toObject().value("fColWidthAuto").toString();
            QString sAlign     = oArr.at(iCol).toObject().value("fColAlign"    ).toString();
            QString sHidden    = oArr.at(iCol).toObject().value("fColHidden"   ).toString();
            QString sReadOnly  = oArr.at(iCol).toObject().value("fColReadOnly" ).toString();

            //Using the rule 'item_ruler_grid' in file 'hq_base_db_grid.h'.

            //Title
            item->setText(sColText);

            //Field name
            item->setData(Qt::UserRole + eColField, QVariant(sField));

            //Width
            if (!sWidth.isEmpty() && sWidth.toInt() > 0)
            {
                this->setColumnWidth(iCol, sWidth.toInt());
            }

            //WidthAuto
            item->setData(Qt::UserRole + eColWidthAuto, QVariant(sWidthAuto));

            //Align
            item->setData(Qt::UserRole + eColAlign, QVariant(sAlign));
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

            //Hidden
            if (!sHidden.isEmpty())
            {
                this->setColumnHidden(iCol, true);
            }

            //ReadOnly
            item->setData(Qt::UserRole + eColReadOnly, QVariant(sReadOnly));

            this->setHorizontalHeaderItem(iCol, item);
        }

        //AllColsWidthAuto
        this->resizeColumnsToContents();

        //GridReadOnly
        QString sGridReadOnly = m_oJsonHeader.value("GridReadOnly").toString();
        if (sGridReadOnly.isEmpty())
        {
            /* Bit operation : or=|
             * For using here, there are some code of the key F2 in slot function keyPressEvent.
             */
            this->setEditTriggers(QAbstractItemView::DoubleClicked
                                  | QAbstractItemView::EditKeyPressed
                                  | QAbstractItemView::AnyKeyPressed);//edit
            m_bIsReadOnly = false;
        }
        else
        {
            this->setEditTriggers(QAbstractItemView::NoEditTriggers);//readonly
            m_bIsReadOnly = true;
        }

        HQ_Base::f_Pointer_SetValue(sErr, "");
        bReturn = true;
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For unknown reason.");
        bReturn = false;
    }
    m_bIsRowStateProtected = false;
    return bReturn;
}
bool HQ_Base_Db_Grid::f_FillHeader(QString sModel, QString *sErr)
{
    QString sErrPrefix = "Grid header filling failed.";

    if (sModel.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For given model being empty.");
        return false;
    }

    QJsonObject oJsonModel = HQ_Base::f_Json_StringToJsonObj(sModel);
    if (oJsonModel.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix +
                              " For given model string can not convert to json object.");
        return false;
    }
    return f_FillHeader(oJsonModel, sErr);
}
/*================================================================
 * Fill the json data to grid.
 * The json argument is a json object or string type.
 *===============================================================*/
bool HQ_Base_Db_Grid::f_FillContents(QJsonObject oJsonContents, QString *sErr)
{
    QString sErrPrefix = "Grid contents filling failed.";

    if (oJsonContents.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For given contents being empty.");
        return false;
    }

    bool bReturn;
    try
    {
        QJsonArray oArrRows = oJsonContents.value("Rows").toArray();

        this->clearContents();
        this->setRowCount(oArrRows.size());

        QJsonArray oArrRows_New;
        for (int iRow = 0; iRow < oArrRows.size(); iRow++)
        {
            QJsonObject oJsonRow = oArrRows.at(iRow).toObject();

            f_FillRowByJsonObject(iRow, oJsonRow);
            oJsonRow.insert("fRow", QString::number(iRow));
            oJsonRow.insert("fState", "");
            oArrRows_New.append(oJsonRow);
        }

        QJsonObject oJson_New;
        oJson_New.insert("Cols", oJsonContents.value("Cols").toString());
        oJson_New.insert("Rows", oArrRows_New);

        m_oJsonContents = oJson_New;

        //Resize columns.
        this->resizeColumnsToContents();

        HQ_Base::f_Pointer_SetValue(sErr, "");
        bReturn = true;
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For unknown reason.");
        bReturn = false;
    }
    return bReturn;
}
bool HQ_Base_Db_Grid::f_FillContents(QString sJsonContents, QString *sErr)
{
    QString sErrPrefix = "Grid contents filling failed.";

    if (sJsonContents.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For given contents being empty.");
        return false;
    }

    QJsonObject oJsonContents = HQ_Base::f_Json_StringToJsonObj(sJsonContents);
    if (oJsonContents.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix +
                              " For given contents string can not convert to json object.");
        return false;
    }
    return f_FillContents(oJsonContents, sErr);
}
void HQ_Base_Db_Grid::f_FillRowByJsonObject(int iRow, QJsonObject oJsonRow)
{
    m_bIsRowStateProtected = true;
    int iColCount = this->columnCount();
    for (int iCol = 0; iCol < iColCount; iCol++)
    {
        //Using the rule 'item_ruler_grid' in file 'hq_base_db_grid.h'.

        QTableWidgetItem *item = this->horizontalHeaderItem(iCol);
        QString sField = item->data(Qt::UserRole + eColField).toString();
        QString sWidthAuto = item->data(Qt::UserRole + eColWidthAuto).toString();
        QString sColAlign = item->data(Qt::UserRole + eColAlign).toString();
        QString sReadOnly = item->data(Qt::UserRole + eColReadOnly).toString();

        item = new QTableWidgetItem();

        //Value
        QString sValue = "";
        if (!oJsonRow.value(sField).isUndefined())
        {
            sValue = oJsonRow.value(sField).toString();
        }
        item->setText(sValue);

        //Align
        if (sColAlign == "left")
        {
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else if (sColAlign == "right")
        {
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        else
        {
            item->setTextAlignment(Qt::AlignCenter);
        }

        this->setItem(iRow, iCol, item);

        //AutoWidth
        if (!sWidthAuto.isEmpty() && sWidthAuto.toInt() > 0)
        {
            this->resizeColumnToContents(iCol);
        }

        //ReadOnly
        //Bit operation: and=&, not=~, or=|, xor=^, <<, >>, ...
        if (sReadOnly.isEmpty())
        {
            item->setFlags(item->flags() | Qt::ItemIsEditable);//edit
            item->setBackground(Qt::white);
        }
        else
        {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);//readonly
            item->setBackground(Qt::lightGray);
        }
    }
    m_bIsRowStateProtected = false;
}
/*================================================================
 * set or get the foreign key
 *===============================================================*/
void HQ_Base_Db_Grid::f_SetForeignKeyValue(QString sForeignKey)
{
    m_sForeignKeyValue = sForeignKey;
}
QString HQ_Base_Db_Grid::f_GetForeignKeyValue() const
{
    return m_sForeignKeyValue;
}
/*================================================================
 * Return the value from Model, with given field and feature.
 * For excample: f_getFieldFeature("FieldName", "Align");
 *===============================================================*/
QString HQ_Base_Db_Grid::f_GetFieldFeature(QString sField, QString sFeature)
{
    QJsonArray oArr;
    oArr = m_oJsonHeader.value("Rows").toArray();
    QString sReturn = "";

    for (int i = 0; i < oArr.size(); i++)
    {
        if (oArr.at(i).toObject().value("fColField").toString() == sField)
        {
            sReturn = oArr.at(i).toObject().value(sFeature).toString();
        }
    }

    return sReturn;
}
/*================================================================
 * Set the row state
 *===============================================================*/
void HQ_Base_Db_Grid::f_SetRowState(int iRow, QString sState, QString *sErr)
{
    if (m_oJsonContents.isEmpty())
    {
        return;
    }

    QJsonArray oArrRows = m_oJsonContents.value("Rows").toArray();
    QJsonArray oArrRows_New;

    int iMaxRowIndex = -1;

    for (int iRowJson = 0; iRowJson < oArrRows.size(); iRowJson++)
    {
        QJsonObject oJsonRow = oArrRows.at(iRowJson).toObject();
        QJsonValue vRow = oJsonRow.value("fRow");
        QString sState_Before = oJsonRow.value("fState").toString();

        if (vRow.isUndefined() || vRow.isNull() || vRow.toString() == "")
        {
            /* If the deleted row exists, its field 'fRow' is empty.
             * But it needs to be stored in the json data.
             */
        }
        else
        {
            int iCurJsonRow = vRow.toString().toInt();
            if (iCurJsonRow > iMaxRowIndex)
            {
                //Used for rememberring the max row index,
                //after a moment, using the max row index add a new row at the end .
                iMaxRowIndex = iCurJsonRow;
            }
            if (iCurJsonRow == iRow)
            {
                //Insert row
                if (sState == "0")
                {
                    QJsonObject oJsonRow_New = f_InsertJsonRow(iRow);
                    oArrRows_New.append(oJsonRow_New);
                    oJsonRow.insert("fRow", QString::number(iRow + 1));
                }

                //Modify row
                //Must to judge the row state, if current row is a new row,its row state is "0",
                //it shouldn't be set to "1",  because there is no record in database.
                if (sState == "1" && sState_Before == "")
                {
                    oJsonRow.insert("fState", sState);
                }

                //Delete row
                if (sState == "-1")
                {
                    //If delete a new row created before without saved. Let it go.
                    if (sState_Before == "0")
                    {
                        continue;
                    }
                    else
                    {
                        oJsonRow.insert("fState", sState);
                        oJsonRow.insert("fRow", "");
                    }
                }
            }
            else if (iCurJsonRow > iRow)
            {
                int iRow_Old = oJsonRow.value("fRow").toString().toInt();

                //Insert row
                if (sState == "0")
                {
                    //After current row be inserted, set the index add 1
                    //for the other rows whose index greater than it
                    QString sRowNewIndex = QString::number(iRow_Old + 1);
                    oJsonRow.insert("fRow", sRowNewIndex);
                }

                //Delete row
                if (sState == "-1")
                {
                    //After current row be deleted, set the index subtract 1
                    //for the other rows whose index greater than it
                    QString sRowNewIndex = QString::number(iRow_Old - 1);
                    oJsonRow.insert("fRow", sRowNewIndex);
                }
            }
        }//if (vRow.isUndefined() || vRow.isNull() || vRow.toString() == "")

        oArrRows_New.append(oJsonRow);
    }//for

    /* Add a new row at the end.
     * At this time, the given row index is greater than the last one in oArrRows.
     * So the new row of json object should be created at last.
     */
    if (sState == "0" && iRow > iMaxRowIndex)
    {
        QJsonObject oJsonRow_New = f_InsertJsonRow(iRow);
        oArrRows_New.append(oJsonRow_New);
    }

    QJsonObject oJson_New;
    oJson_New.insert("Cols", m_oJsonContents.value("Cols").toString());
    oJson_New.insert("Rows", oArrRows_New);

    m_oJsonContents = oJson_New;
    HQ_Base::f_Pointer_SetValue(sErr, "");
}
/*================================================================
 * return the row state
 *===============================================================*/
QString HQ_Base_Db_Grid::f_GetRowState(int iRow, QString *sErr) const
{
    if (iRow < 0)
    {
        return "";
    }

    try
    {
        QJsonArray oArrRows = m_oJsonContents.value("Rows").toArray();
        for (int iRowJson = 0; iRowJson < oArrRows.size(); iRowJson++)
        {
            QJsonObject oJsonRow = oArrRows.at(iRowJson).toObject();
            if (oJsonRow.value("fRow").toString().toInt() == iRow)
            {
                HQ_Base::f_Pointer_SetValue(sErr, "");
                return oJsonRow.value("fState").toString();
            }
        }
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, "Row state getting failed.");
        return "";
    }
    HQ_Base::f_Pointer_SetValue(sErr, "");
    return "";
}
/*===============================================================
 * Return the json header as string
 *===============================================================*/
QString HQ_Base_Db_Grid::f_GetHeader(QString *sErr) const
{
    try
    {
        return HQ_Base::f_Json_ToString(m_oJsonHeader, sErr);
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, "Grid header getting failed.");
        return "";
    }
}
/*================================================================
 * Return the json data as string
 *===============================================================*/
QString HQ_Base_Db_Grid::f_GetData(QString *sErr) const
{
    try
    {
        return HQ_Base::f_Json_ToString(m_oJsonContents, sErr);
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, "Grid data getting failed.");
        return "";
    }
}
/*================================================================
 * Return modified data as json string.
 * -------------------
 *  About modified json string:
 *
 *       {
 *           "Fields": "Field1_Name|Field2_Name|...",//Used for saving
 *           "Cols": "Col1_Name|Col2_Name|...",//Used for displaying
 *           "Rows": [
 *               {
 *                   "fGUID": "ec5aa996-7d1e-577d-f1ff-da79d5288716",
 *                   "Field1_Name": "value",
 *                   "Field2_Name": "value",
 *                   ...,
 *                   "fState": "-1"
 *               },
 *               {
 *                   "fGUID": "ec5aa996-7d1e-577d-f1ff-da79d5288716",
 *                   "Field1_Name": "value",
 *                   "Field2_Name": "value",
 *                   ...,
 *                   "fState": "-1"
 *               },
 *               ...
 *           ],
 *           "TableName": "table_name"
 *       }
 *
 *  About Fields and Cols:
 *
 *       They are different from each other, for saving and displaying.
 *    More desc in the part 'About Fields and Cols' at the first in
 *    header file 'hq_base_db_model.h'.
 *===============================================================*/
QString HQ_Base_Db_Grid::f_GetModified(QString *sErr) const
{
    QString sErrPrefix = "The modified data of grid getting failed.";
    try
    {
        QJsonArray oArrRows = m_oJsonContents.value("Rows").toArray();
        QJsonArray oArrRows_New;
        int iRowCountModified = 0;
        for (int iRowJson = 0; iRowJson < oArrRows.size(); iRowJson++)
        {
            QJsonObject oJsonRow = oArrRows.at(iRowJson).toObject();
            QJsonValue vState = oJsonRow.value("fState");

            if (vState.isUndefined() || vState.isNull() || vState.toString() == "")
            {
                continue;
            }
            else if (vState.toString() == "0" || vState.toString() == "1"
                     || vState.toString() == "-1")
            {
                if (vState.toString() == "0" || vState.toString() == "1")
                {
                    for (int iCol = 0; iCol < this->columnCount(); iCol++)
                    {
                        QString sField = f_GetColNameByIndex(iCol);
                        int iRow = oJsonRow.value("fRow").toString().toInt();
                        QString sCellText = this->item(iRow, iCol)->text();
                        oJsonRow.insert(sField, sCellText);
                    }
                }
                oArrRows_New.append(oJsonRow);
                iRowCountModified++;
            }
        }
        if (iRowCountModified == 0)
        {
            HQ_Base::f_Pointer_SetValue(sErr, "");
            return "";
        }

        QJsonObject oJson_New;
        oJson_New.insert("TableName", m_oJsonHeader.value("TableName").toString());
        oJson_New.insert("Fields", m_oJsonHeader.value("Fields").toString());
        oJson_New.insert("Cols", m_oJsonContents.value("Cols").toString());
        oJson_New.insert("Rows", oArrRows_New);

        return HQ_Base::f_Json_ToString(oJson_New, sErr);
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For unknown reason.");
        return "";
    }
}
/*================================================================
 * Return the cell data as string
 *===============================================================*/
QString HQ_Base_Db_Grid::f_GetCellValue(int iRow, QString sColName, QString *sErr) const
{
    QString sErrPrefix = "The cell data getting failed.";
    try
    {
        if (m_oJsonContents.isEmpty())
        {
            HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For grid data lost.");
            return "";
        }
        QJsonArray oArrRows = m_oJsonContents.value("Rows").toArray();
        for (int iRowJson = 0; iRowJson < oArrRows.size(); iRowJson++)
        {
            QJsonObject oJsonRow = oArrRows.at(iRowJson).toObject();
            QJsonValue vRow = oJsonRow.value("fRow");

            if (vRow.isUndefined() || vRow.isNull() || vRow.toString() == "")
            {
                continue;
            }
            else if (vRow.toString().toInt() == iRow)
            {
                HQ_Base::f_Pointer_SetValue(sErr, "");
                return oJsonRow.value(sColName).toString();
            }
        }
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For unkown reason.");
        return "";
    }
    HQ_Base::f_Pointer_SetValue(sErr, "");
    return "";
}
/*===============================================================
 * Set cell value.
 *
 * If the argument 'bMarkToModified' was given true, the row state
 * will be set to '1'(modified) .  Or the row state will not be
 * changed.
 *===============================================================*/
bool HQ_Base_Db_Grid::f_SetCellValue(int iRow, QString sColName, QString sValue,
                                     bool bMarkToModified, QString *sErr)
{
    QString sErrPrefix = "The cell data setting failed.";
    try
    {
        if (m_oJsonContents.isEmpty())
        {
            HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For the grid data lost.");
            return false;
        }
        QJsonArray oArrRows = m_oJsonContents.value("Rows").toArray();
        QJsonArray oArrNewRows;
        for (int iRowJson = 0; iRowJson < oArrRows.size(); iRowJson++)
        {
            QJsonObject oJsonRow = oArrRows.at(iRowJson).toObject();
            QJsonValue vRow = oJsonRow.value("fRow");

            if (vRow.isUndefined() || vRow.isNull() || vRow.toString() == "")
            {
                /* If the deleted row exists, its field 'fRow' is empty.
                 * But it needs to be stored in the json data.
                 */
            }
            else if (vRow.toString().toInt() == iRow)
            {
                int iCol = f_GetColIndexByName(sColName);
                if (iCol >= 0 && iCol < this->columnCount())
                {
                    /* Here, the row state protecting is required. Otherwise the
                     * 'on_itemChanged' slot will be triggered. In the slot function
                     * the function 'f_SetCellValue' was called again. This case
                     * shouldn't appear.
                     */
                    bool bBefore = m_bIsRowStateProtected;
                    m_bIsRowStateProtected = true;
                    this->item(iRow, iCol)->setText(sValue);
                    m_bIsRowStateProtected = bBefore;
                }

                //Update the json data.
                oJsonRow.insert(sColName, sValue);

                //Mark the row state to modified if necessary.
                if (bMarkToModified)
                {
                    QString sState_Before = oJsonRow.value("fState").toString();
                    if (sState_Before != "0")
                    {
                        oJsonRow.insert("fState", "1");
                    }
                }
            }
            oArrNewRows.append(oJsonRow);
        }
        m_oJsonContents.insert("Rows", oArrNewRows);
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For unknown reason.");
        return false;
    }
}
/*================================================================
 * For the keypress event.(override)
 *===============================================================*/
void HQ_Base_Db_Grid::keyPressEvent(QKeyEvent *event)
{
    if (nullptr == event)
    {
        return;
    }

    //'Ctrl+s' to send the saving signal.
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_S)
    {
        /* Change the current cell to end the editting.
         * If the col was set to -1, it is a invalid value. And the row will be set to -1
         * automatically. So it is all right here to set the row to any value. This time
         * -1 was given. Whaterver, it will trigger the 'on_currentCellChanged' slot. This
         * contains the row changed signal. If the sub gird exists, it will be reloaded by
         * the row changing signal of the main grid, but it is unexpected. And in the slot
         * 'on_currentCellChanged', a new signal 'sigRowChanged' was sent used for instead
         * it.
         */
        int iCurRow = this->currentRow();
        int iCurCol = this->currentColumn();
        this->setCurrentCell(-1, -1);
        this->setCurrentCell(iCurRow, iCurCol);
        emit sigSave();
    }
    else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Insert)
    {
        f_InsertRowByPressKey();
    }
    else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Delete)
    {
        f_RemoveSelectedRow(event);
    }
    else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Up)
    {
        f_SetRowSelectionUpDown(true);
    }
    else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Down)
    {
        f_SetRowSelectionUpDown(false);
    }
    else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Left)
    {
        f_SetCellSelectionLeftRight(true);
    }
    else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Right)
    {
        f_SetCellSelectionLeftRight(false);
    }
    else
    {
        QTableWidget::keyPressEvent(event);
    }
}
/*===============================================================
 * Insert row by press key 'insert'.
 *
 * It is different from function 'insertRow'. Function 'insertRow'
 * should not be called by key press by user directly.
 *===============================================================*/
void HQ_Base_Db_Grid::f_InsertRowByPressKey()
{
    if (m_bIsReadOnly)
    {
        return;
    }

    insertRow();
}
/*================================================================
 * Remove selected rows when press delete key.
 *===============================================================*/
void HQ_Base_Db_Grid::f_RemoveSelectedRow(QKeyEvent *event)
{
    if (m_bIsReadOnly)
    {
        return;
    }

    emit sigRowDeleteQuery(event);
    /* Pay attention!!!
     * Because the following code needs to detecting the value of
     * variable 'event'. So the signal must to be connected with
     * Qt::DirectConnection type to outside.
     * The following code should be executed after the value of
     * variable 'event' is changed. More desc info in the part
     * 'About row deleting' of the header file.
     */
    if (!event->isAccepted())
    {
        return;
    }

    QStringList sIndexList = f_GetSelectedRowList_ID();
    for (QString sRowGUID : sIndexList)
    {
        this->removeRow(sRowGUID);
    }
}
/*================================================================
 * Change the status of row selection, when select mode is by row.
 * The value of argument bUpOrDown is bool type, ture means up,
 * false means down.
 * The value of argument bLoop is bool type too, ture means allow
 * looping, false means not. When looping allowed, press up key
 * when the selected position is the top, then the last row will be
 * selected.
 * An other case, press down key when the selected position is the
 * last, then the top row will be selected.
 *===============================================================*/
void HQ_Base_Db_Grid::f_SetRowSelectionUpDown(bool bUpOrDown, bool bLoop)
{
    if (this->selectionBehavior() != QAbstractItemView::SelectRows)
    {
        return;
    }

    int iTargetRow = -1;
    int iCurCol = this->currentColumn();

    int iColCount = this->f_ColumnCount_Visible();
    QList<QTableWidgetItem*> items = this->selectedItems();
    if (items.empty() || items.count() <= 0)//If no row was selected
    {
        if (this->rowCount() > 0)//up : move to the end
                                 //down : move to the first
        {
            iTargetRow = bUpOrDown ? this->rowCount() - 1 : 0;
        }
    }
    else if ((items.count() % iColCount) == 0)
    {
        //Only reselect the top row of selected rows.
        int iRow = this->row(items.at(0));

        if (bUpOrDown)//press up
        {
            if (iRow > 0)
            {
                iTargetRow = iRow - 1;//move to the last
            }
            else if (iRow == 0 && bLoop)
            {
                iTargetRow = this->rowCount() - 1;//move to the end
            }
        }
        else//press down
        {
            if (iRow < (this->rowCount() - 1))
            {
                iTargetRow = iRow + 1;//move to the next
            }
            else if (iRow == (this->rowCount() - 1) && bLoop)
            {
                iTargetRow = 0;//move to the first
            }
        }
    }

    if (iTargetRow > -1 && iTargetRow < this->rowCount())
    {
        this->selectRow(iTargetRow);
        this->setCurrentCell(this->currentRow(), iCurCol);
    }
}
/*===============================================================
 * Change the cell selection to left or right.
 * The argument bLeftOrRight's means: true=left, false=right.
 *===============================================================*/
void HQ_Base_Db_Grid::f_SetCellSelectionLeftRight(bool bLeftOrRight, bool bLoop)
{
    int iTargetCol = -1;

    if (bLeftOrRight)//to left
    {
        if (this->currentColumn() > 0)
        {
            iTargetCol = this->currentColumn() - 1;//move to the last
        }
        else if (this->currentColumn() <= 0 && bLoop)
        {
            iTargetCol = this->columnCount() - 1;//move to the end
        }
    }
    else if (!bLeftOrRight)//to right
    {
        if (this->currentColumn() < (this->columnCount() - 1))
        {
            iTargetCol = this->currentColumn() + 1;//move to the next
        }
        else if (this->currentColumn() >= (this->columnCount() - 1))
        {
            iTargetCol = 0;//move to the first
        }
    }

    if (iTargetCol > -1)
    {
        this->setCurrentCell(this->currentRow(), iTargetCol);
    }
}
/*===============================================================
 * slot: item changed.
 *===============================================================*/
void HQ_Base_Db_Grid::on_itemChanged(QTableWidgetItem *item)
{
    if (!m_bIsRowStateProtected)
    {
        int iRow = item->row();
        int iCol = item->column();
        f_SetCellValue(iRow, f_GetColNameByIndex(iCol), item->text(), true);
    }
}
/**===============================================================
 ** slot: currentCellChanged
 ** If the row or col was set to a invalid value, the rowchanged
 ** signal will not be sent.
 **===============================================================*/
void HQ_Base_Db_Grid::on_currentCellChanged(int currentRow, int currentColumn,
                                            int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousColumn)
    if (currentRow < 0 || currentColumn < 0 || currentRow == previousRow)
    {
        return;
    }
    emit sigRowChanged(currentRow);
}
/*===============================================================
 * All columns resize width automatically.(override)
 *===============================================================*/
void HQ_Base_Db_Grid::resizeColumnsToContents()
{
    //All columns resize width automatically.
    //The name 'AllColsWidthAuto' is different from 'ColWidthAuto'.
    //'AllColsWidthAuto' means all the columns, 'ColWidthAuto' means single column.
    QString sAllColsWidthAuto = m_oJsonHeader.value("AllColsWidthAuto").toString();
    if (!sAllColsWidthAuto.isEmpty())
    {
        QTableWidget::resizeColumnsToContents();
    }
}
/*===============================================================
 * Set the item delegate.(override and overload)
 *===============================================================*/
void HQ_Base_Db_Grid::setItemDelegateForColumn(int column,
                                               QAbstractItemDelegate *delegate)
{
    QTableWidget::setItemDelegateForColumn(column, delegate);
}
void HQ_Base_Db_Grid::setItemDelegateForColumn(QString sColName,
                                               QAbstractItemDelegate *delegate)
{
    int iCol = f_GetColIndexByName(sColName);
    QTableWidget::setItemDelegateForColumn(iCol, delegate);
}
/*===============================================================
 * Return col index by given ColName/Field.
 * If the column name is not exists, return -1.
 *
 * If you want to get the colname/field by col index, you can use
 * the function f_GetColNameByIndex(int iCol).
 *===============================================================*/
int HQ_Base_Db_Grid::f_GetColIndexByName(QString sColName) const
{
    int iColIndex = -1;
    QJsonArray oArr;
    oArr = m_oJsonHeader.value("Rows").toArray();
    int iColCount = oArr.size();
    for (int iCol = 0; iCol < iColCount; iCol++)
    {
        //Field name
        QString sField = oArr.at(iCol).toObject().value("fColField").toString();
        if (sField == sColName)
        {
            iColIndex = iCol;
            break;
        }
    }
    return iColIndex;
}
/*===============================================================
 * Return ColName/Field by given col index.
 * Return "" on error.
 *
 * If you want to get the col index by field/colname, use the
 * function f_GetColIndexByName(QString sColName).
 *===============================================================*/
QString HQ_Base_Db_Grid::f_GetColNameByIndex(int iCol) const
{
    QString sField = "";

    if (iCol >= 0 && iCol < this->columnCount())
    {
        QTableWidgetItem *item = this->horizontalHeaderItem(iCol);

        //Using the rule 'item_ruler_grid' in file 'hq_base_db_grid.h'.
        QVariant v = item->data(Qt::UserRole);
        if (v.isValid())
        {
            sField = v.toString();
        }
    }

    return sField;
}
/*===============================================================
 * Return row index by given GUID.
 * If the GUID is not exists, return -1.
 *===============================================================*/
int HQ_Base_Db_Grid::f_GetRowIndexByID(QString sGUID) const
{
    return f_GetRowIndexByValue("fGUID", sGUID);
}
/*===============================================================
 * Return the first one of indexs of matching rows by given field
 * and its value. It is case insenstive.
 * If the field or the value is not exists, return -1.
 *===============================================================*/
int HQ_Base_Db_Grid::f_GetRowIndexByValue(QString sFeild, QString sValue) const
{
    int iRowIndex = -1;

    QJsonArray oArrRows = m_oJsonContents.value("Rows").toArray();
    for (int iRowJson = 0; iRowJson < oArrRows.size(); iRowJson++)
    {
        QJsonObject oJsonRow = oArrRows.at(iRowJson).toObject();
        QJsonValue vRow = oJsonRow.value("fRow");

        if (vRow.isUndefined() || vRow.isNull() || vRow.toString() == "")
        {
            continue;
        }
        else if (oJsonRow.value(sFeild).toString().toLower() == sValue.toLower())
        {
            iRowIndex = vRow.toString().toInt();
            break;
        }
    }

    return iRowIndex;
}
/*===============================================================
 * Select a row by given index/GUID.(override and overload)
 *
 * The function will select a specified row and let the current
 * col index to 0. So mostly it used for the selection state init.
 * Because at this time the current col info is unimportant and
 * indifferent.
 *
 * But mostly int the save function, the function 'setCurrentCell'
 * was recommended. Because at this time, the user has edited a
 * cell, the position of current cell should not be changed.
 *===============================================================*/
void HQ_Base_Db_Grid::selectRow(int row)
{
    QTableWidget::selectRow(row);
}
void HQ_Base_Db_Grid::selectRow(QString sGUID)
{
    selectRow(f_GetRowIndexByID(sGUID));
}
/*===============================================================
 * Return a string list of GUID of selected row.
 *===============================================================*/
QStringList HQ_Base_Db_Grid::f_GetSelectedRowList_ID() const
{
    QStringList sIndexList = f_GetSelectedRowList_Index();
    QStringList sGUIDList;

    for (QString sRowIndex : sIndexList)
    {
        int iRow = sRowIndex.toInt();
        sGUIDList << f_GetCellValue(iRow, "fGUID");
    }

    return sGUIDList;
}
/*===============================================================
 * Return a string list of row index of selected row.
 *===============================================================*/
QStringList HQ_Base_Db_Grid::f_GetSelectedRowList_Index() const
{
    QStringList sIndexList;

    int iColCount = this->f_ColumnCount_Visible();
    QList<QTableWidgetItem*> items = this->selectedItems();
    if (items.empty() || items.count() <= 0)
    {
        return sIndexList;
    }
    else if ((items.count() % iColCount) == 0)
    {
        for (int i = 0; i < items.count(); i++)
        {
            if ((i % iColCount) == 0)
            {
                int iRow = this->row(items.at(i));
                sIndexList << QString::number(iRow);
                continue;
            }
        }
    }

    return sIndexList;
}
/*===============================================================
 * Return item object.(override and overload)
 *===============================================================*/
QTableWidgetItem* HQ_Base_Db_Grid::item(int row, int column) const
{
    return QTableWidget::item(row, column);
}
QTableWidgetItem* HQ_Base_Db_Grid::item(int row, QString sFieldName) const
{
    return QTableWidget::item(row, f_GetColIndexByName(sFieldName));
}
/*===============================================================
 * SelectionChanged.(override)
 *
 * If the space area was clicked, the row selection state will
 * lost. Reset it.
 *===============================================================*/
void HQ_Base_Db_Grid::selectionChanged(const QItemSelection &selected,
                                       const QItemSelection &deselected)
{
    QTableWidget::selectionChanged(selected, deselected);
    if(selected.isEmpty() && !deselected.isEmpty())
    {
        this->setCurrentCell(currentRow(), currentColumn());
    }
}
/*===============================================================
 * setCurrentCell.(override and overload)
 *===============================================================*/
void HQ_Base_Db_Grid::setCurrentCell(int row, int column)
{
    QTableWidget::setCurrentCell(row, column);
}
void HQ_Base_Db_Grid::setCurrentCell(int iRow, QString sColName)
{
    int iCol = f_GetColIndexByName(sColName);
    this->setCurrentCell(iRow, iCol);
}
void HQ_Base_Db_Grid::setCurrentCell(QString sGUID, int iCol)
{
    int iRow = f_GetRowIndexByID(sGUID);
    this->setCurrentCell(iRow, iCol);
}
void HQ_Base_Db_Grid::setCurrentCell(QString sGUID, QString sColName)
{
    int iRow = f_GetRowIndexByID(sGUID);
    int iCol = f_GetColIndexByName(sColName);
    this->setCurrentCell(iRow, iCol);
}
