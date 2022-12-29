/************************************************************************
 ** Class name:     HQ_Base_Db_Grid
 ** Author:         NieXin
 ** Created date:   2022-5-20
 ** First:
 **
 **         By this way, the QTableWidget is called "oGrid". The letter
 **     "o" means it is a object.
 **
 ** Used for:
 **
 **         Automaticaly load and display data. Return data as json string.
 **     If data be modified, you can call the function getModified() to
 **     get the string of it, then the string can be used to save data.
 **
 ** Usage method:
 **
 **     ui->oGrid->f_FillHeader(sJson);
 **     ui->oGrid->f_GetModified();
 **     ...
 **
 ** About data type:
 **
 **         So as to made the data type freely, the string type was used
 **     mostly. It means 'Anything is string.' So if the database type is
 **     changed, everything is ok, nothing is a problem.
 **
 ** About json format:
 **
 **     {
 **         "Cols":"Field1_Name|Field2_Name|..."
 **         "Rows":[
 **             {
 **                 "Field1_Name":"Field1_Value",
 **                 "Field2_Value",
 **                 ...,
 **                 "fRow":"row index of grid",
 **                 "fState":"row state"
 **             },
 **             {
 **                 "Field1_Name":"Field1_Value",
 **                 "Field2_Value",
 **                 ...,
 **                 "fRow":"row index of grid",
 **                 "fState":"row state"
 **             },
 **             ...
 **         ]
 **     }
 **
 ** About row state:
 **
 **     "0": new row, need to save to database
 **     "1": modified row, need to save to database
 **     "-1": deleted row, need to save to database
 **
 **     "": none state, do nothing, mostly after loading
 **
 ** About modified json string:
 **
 **      {
 **          "Cols": "Field1_Name|Field2_Name|...",
 **          "Rows": [
 **              {
 **                  "fGUID": "ec5aa996-7d1e-577d-f1ff-da79d5288716",
 **                  "Field1_Name": "value",
 **                  "Field2_Name": "value",
 **                  ...,
 **                  "fState": "-1"
 **              },
 **              {
 **                  "fGUID": "ec5aa996-7d1e-577d-f1ff-da79d5288716",
 **                  "Field1_Name": "value",
 **                  "Field2_Name": "value",
 **                  ...,
 **                  "fState": "-1"
 **              },
 **              ...
 **          ],
 **          "TableName": "table_name"
 **      }
 **
 ** About member variable m_bIsRowStateProtected:
 **
 **        This is a flag used to protect the row state.
 **
 **        When the grid is edited by user, the 'itemchanged' event will
 **    be triggered, and this will lead to set the row state to '1'. The
 **    function 'f_SetCellValue' can set the row state to '1' too. It means
 **    the row has been modified.
 **
 **        But in another case, when the data is filling in grid, it will
 **    trigger the 'itemchanged' event too. But this time, it needn't
 **    to update the row state.
 **
 **        So yet, before grid filling, the flag 'm_bIsRowStateProtected' must
 **    to be set to "true". When after filling, the flag 'm_bIsRowStateProtected'
 **    must to be set to "false".
 **
 **        At the same time in the function 'on_itemChanged', it needs to
 **    detect the flag 'm_bIsRowStateProtected'. If flag 'm_bIsRowStateProtected'
 **    is true, the slot function shouldn't be executed.
 **
 **        So if you want to update the grid ui outside, but at the same
 **    time you do not want to save the changed data to database,   you
 **    should use the function 'f_SetCellValue' to update the grid data.
 **    At this time, you should give a correct value  to  the  argument
 **    'bMarkToModified' of function 'f_SetCellValue'. About this, see
 **    the part 'About function f_SetCellValue'.
 **
 **        But But But! If the item checkBox(checkState) was used. It will
 **    trigger the 'itemChanged' slot too. So the row state protecting
 **    should be used by the variable 'm_bIsRowStateProtected'.
 **
 ** About function f_SetCellValue:
 **
 **        You can use this function to set value of grid cell. It will
 **    update the grid cell and its json data in the back.
 **
 **        The function has a bool type argument named 'bMarkToModified'.
 **    If it was set to true or by default. The function will set the
 **    cell row state to '1', it means the row has been modified. If the
 **    argument was set to false, the row state will not be changed.
 **
 **        It is very useful for update grid cell value only for display.
 **    For example, sometime you give a value to the grid cell, but you
 **    do not want to save it to database, so you do not want to change
 **    the row state to '1', you can call the function like below:
 **
 **        oGrid->f_SetCellValue(rowIndex, colName, value, false);
 **
 ** About item object:
 **
 **        The item object likes a type of data carrier. It stores many
 **    info. Mostly the type of these info may be any object, such as
 **    string, socket, thread, and so on.
 **
 **        When the item object was used, the rules must be required. The
 **    item object maybe has many fields which be distinguished by 'role'.
 **    Such as below:
 **
 **    //socket
 **    item->setData(QVariant::fromValue(m_tcpSocket), Qt::UserRole + 2);
 **    QTcpSocket *tcpSocket = item->data(Qt::UserRole + 2).value<QTcpSocket*>();
 **
 **    //thread
 **    item->setData(QVariant::fromValue(m_thd), Qt::UserRole + 3);
 **    Thread *thd = item->data(Qt::UserRole + 3).value<Thread*>();
 **
 **    //string
 **    item->setData(Qt::UserRole + 1, QVariant(str));
 **    QString str = item->data(Qt::UserRole + 1).toString();
 **
 **        If the item object was used somewhere, the rules desc here must be
 **    modified. And the desc text is required as below:
 **
 **          //Using the rule 'item_ruler_grid' in file 'hq_base_db_grid.h'.
 **
 **        If you will find the place of code where is item object used. Search
 **    the keyword 'Qt::UserRole'.
 **
 **    +-----+
 **    |rules| :
 **    +-----+
 **
 **          enum eUserRole
 **          {
 **              //This is effectively the same as:
 **              //       sColText : text
 **              //         sField : Qt::UserRole
 **              //     sWidthAuto : Qt::UserRole + 1
 **              //         sAlign : Qt::UserRole + 2
 **              //      sReadOnly : Qt::UserRole + 3
 **              eColField = 0,
 **              eColWidthAuto,
 **              eColAlign,
 **              eColReadOnly
 **          };
 **
 **           sColText : text
 **             sField : Qt::UserRole + eColField
 **         sWidthAuto : Qt::UserRole + eColWidthAuto
 **             sAlign : Qt::UserRole + eColAlign
 **          sReadOnly : Qt::UserRole + eColReadOnly
 **
 ** About memory freeing:
 **
 **         The memories of all the item objects of headers and contents
 **     must to be freed after using. Such as these functions: clear(),
 **     clearContents() and removeRow(). Pay attention the position of
 **     delete statement. If the delete statement wrote after the function
 **     clear() or remove() of the widget and so on. It'll lead to err. I
 **     thought perhaps the memory had be freed automatroly in these funcion,
 **     such as clear(), remove(), and so on. So it must wrote before these
 **     functions. Perhaps it is no use, but it is more appropriate.
 **
 ** About row deleting:
 **
 **         The function KeyPressEvent() was overload here. In the function,
 **     the code of row deleting existed as below:
 **
 **         case Qt::Key_Delete:
 **             emit sigRowDeleteQuery(event);
 **             if (event->isAccepted())
 **             {
 **                 f_RemoveSelectedRow();
 **             }
 **         ...
 **
 **         After emit a signal. The operation of row deleting depend on
 **     detecting the value of the variable 'event'. So if you will use
 **     this class in outside code, when you use the connecting code, the
 **     type of connection must to be Qt::DirectConnection. Such as below:
 **
 **         connect(ui->oGridMain, &HQ_Base_Db_Grid::sigRowDeleteQuery,
 **          this, &MainWindow::onGridMain_RowDeleteQuery, Qt::DirectConnection);
 **
 **         So as, the inside 'if' statement will be executed after waiting
 **     the outside slot funcion finished.
 **
 **         In the code of outside slot function called 'sigRowDeleteQuery()',
 **     the argument 'event' is very useful and required. It has two method
 **     called 'ignore()' and 'accept()'. If 'ignore()' method executed, the
 **     deleting operation will be abort, if the 'accept' method executed,
 **     the deleting operation will be done.
 **
 **         The type 'Qt::DirectConnection' used by default when the receiver
 **     and the sender of the signal run in the same thread. Mostly the grid
 **     widget lives in a window, so they live in the same thread,  and the
 **     'direct' type is default. If the slot function runs repeatly, the type
 **     'Qt::uniqueconnection' is required. At this time, these two types of
 **     model can combin with each other using bitwise 'or'. like this:
 **
 **         Qt::ConnectionType(Qt::DirectConnection | Qt::uniqueconnection)
 **
 **         If you won't to use these two models at the same time, it is no
 **     problem, because the window and the grid live in the same thread, and
 **     the 'direct' type runs by default.
 **
 **         So pay attention, if you want to detecting something before
 **     row deleting, the slot function must to lives in the same thread
 **     with this grid widget.
 **
 ** About grid filling:
 **
 **      ----------------------------------------------------------------
 **         By the common way.
 **      ----------------------------------------------------------------
 **
 **      QString sErr;
 **
 **      try
 **      {
 **          //Get sqs statement by model name and fk value.
 **          QString sSQL = HQ_Base_Db_Model::f_GetSqlByFk(m_sModelName_Main,
 **                                               m_sForeignKeyValue, &sErr);
 **          if (sErr != "")
 **          {
 **              throw QString("Sql statement got failed.");
 **          }
 **
 **          //Get contents as json string or object by sql statement.
 **          QString sContents = HQ_Base_DB::f_GetJsonStrBySql(sSQL, &sErr);
 **          if (sErr != "")
 **          {
 **              throw QString("Commands info gotting failed.");
 **          }
 **
 **          //Fill the contents to grid.
 **          ui->oGrid->f_FillContents(sContents, &sErr);
 **          if (sErr != "")
 **          {
 **              throw QString("Commands info filling failed.");
 **          }
 **      }
 **      catch (QString sMsg)
 **      {
 **          QMessageBox::warning(this, "Grid filling failed",
 **                               sMsg + "\n" + sErr,
 **                               QMessageBox::Ok,QMessageBox::NoButton);
 **      }
 **
 **      ----------------------------------------------------------------
 **         By an other way, you can use the class 'hq_base_db_grid_aide'
 **      to finished the above work more simply.
 **      ----------------------------------------------------------------
 **
 **      QString sErr;
 **
 **      //Main grid fills headers and contents.
 **      if (!HQ_Base_Db_Grid_Aide::f_Fill(ui->oGrid, m_sModelName_Main,
 **                                           m_sForeignKeyValue, &sErr))
 **      {
 **          QString sMsg = "The main grid filling header failed.";
 **          QMessageBox::warning(this, sMsg, sErr,
 **                               QMessageBox::Ok,QMessageBox::NoButton);
 **          ui->txtDebug->setText(sErr);
 **          return;
 **      }
 **
 ** About foreignkey (FK):
 **
 **         The foreignkey may be any string, it may be not a guid. So the
 **     it can not use the function 'HQ_Base::f_GUID_GetCommon'. Such as
 **     the below code in function 'f_MadeNewRowJson()':
 **
 **         //right
 **         QString sFK_Value = m_sForeignKeyValue;
 **
 **         //wrong
 **         QString sFK_Value = HQ_Base::f_GUID_GetCommon(m_sForeignKeyValue);
 **
 **         The wrong one, it will change all the char to lower case, this
 **     will lead to err.
 **
 **         If it is a guid sometime, it will be all right. Because commonly,
 **     the fk_value that was given outside will be got from a widget by user
 **     selection. In that widget, the value has been valid before.
 **
 **         For an example, in the ui who has double grids. The fk_value
 **     of the sub grid, it is the value of the primary key of the selected
 **     row in the main grid. And the pk is a valid guid that was generated
 **     before.
 **
 ************************************************************************/

#ifndef HQ_BASE_DB_GRID_H
#define HQ_BASE_DB_GRID_H

#include "qt-hq_base_global.h"
#include <QTableWidget>
#include <QJsonObject>
#include <QJsonArray>
class QKeyEvent;

class QTHQ_BASESHARED_EXPORT HQ_Base_Db_Grid : public QTableWidget
{
    Q_OBJECT
public:
    explicit HQ_Base_Db_Grid(QWidget *parent = Q_NULLPTR);    

    bool m_bIsReadOnly = false;

    /* This is a flag used to protect the row state.
     * More desc can be find in the part 'About member variable m_bIsRowStateProtected:'
     * at the first of this header file.
     */
    bool m_bIsRowStateProtected;

    //Fill the grid header with given json.
    bool f_FillHeader(QJsonObject oJsonModel, QString *sErr = nullptr);
    bool f_FillHeader(QString     sModel,     QString *sErr = nullptr);

    //Fill the grid contents data with given json.
    bool f_FillContents(QJsonObject oJsonContents, QString *sErr = nullptr);
    bool f_FillContents(QString     sJsonContents, QString *sErr = nullptr);

    //Set or get the row state.
    //state: ""=nothing, "1"=modified, "0"=new, "-1"=deleted
    void    f_SetRowState(int iRow, QString sState, QString *sErr = nullptr);
    QString f_GetRowState(int iRow, QString *sErr = nullptr) const;

    //Retrun the json info of header/data/modified as string.
    QString f_GetHeader(QString *sErr = nullptr) const;
    QString f_GetData(QString *sErr = nullptr) const;
    QString f_GetModified(QString *sErr = nullptr) const;

    //Return/Set the cell data as string.
    QString f_GetCellValue(int iRow, QString sColName, QString *sErr = nullptr) const;
    bool    f_SetCellValue(int iRow, QString sColName, QString sValue,
                           bool bMarkToModified, QString *sErr = nullptr);

    //Set/Get the value of foreign key.
    void    f_SetForeignKeyValue(QString sForeignKey);
    QString f_GetForeignKeyValue() const;

    //Return selected rows as json array.
    QJsonArray f_GetRows(int iRowStart, int iCount) const;

    //Return col info by given value.
    int     f_GetColIndexByName(QString sColName) const;
    QString f_GetColNameByIndex(int iCol) const;

    //Return row index by given value.
    int     f_GetRowIndexByValue(QString sFeild, QString sValue) const;
    int     f_GetRowIndexByID(QString sGUID) const;

    //Return a list of selected row.
    QStringList f_GetSelectedRowList_ID() const;
    QStringList f_GetSelectedRowList_Index() const;

    //Set the item delegate.(override and overload)
    void setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate);
    void setItemDelegateForColumn(QString sColName, QAbstractItemDelegate *delegate);

    //Return the item object.(override and overload)
    QTableWidgetItem *item(int row, int column) const;
    QTableWidgetItem *item(int row, QString sFieldName) const;

    //Set the a specified cell to the current cell.(override and overload)
    void setCurrentCell(int row, int column);
    void setCurrentCell(int iRow, QString sColName);
    void setCurrentCell(QString sGUID, int iCol);
    void setCurrentCell(QString sGUID, QString sColName);

public Q_SLOTS:
    void clear();
    void clearContents();
    void removeRow(int row);
    void removeRow(QString sGUID);
    void insertRow(int row);
    void insertRow();
    void addRow(int row);
    void addRow();
    void addRow(QJsonArray oArrNewRows);
    void resizeColumnsToContents();
    void selectRow(int row);
    void selectRow(QString sGUID);

protected:
    void keyPressEvent(QKeyEvent *event);
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) Q_DECL_OVERRIDE;

signals:
    void sigRowChanged(int iRow);
    void sigRowDeleted();
    void sigRowDeleteQuery(QKeyEvent *event);
    void sigSave();

private slots:
    void on_itemChanged(QTableWidgetItem *item);
    void on_currentCellChanged(int currentRow, int currentColumn,
                               int previousRow, int previousColumn);

private:
    enum eUserRole
    {
        //       sColText : text
        //         sField : Qt::UserRole
        //     sWidthAuto : Qt::UserRole + 1
        //         sAlign : Qt::UserRole + 2
        //      sReadOnly : Qt::UserRole + 3
        eColField = 0,
        eColWidthAuto,
        eColAlign,
        eColReadOnly
    };

    QJsonObject m_oJsonHeader;
    QJsonObject m_oJsonContents;

    //foreign key
    QString m_sForeignKeyValue = "";

    //Return the value from Model, with given field and feature.
    //For excample: f_getFieldFeature("FieldName", "Align");
    QString f_GetFieldFeature(QString sField, QString sFeature);

    int  f_ColumnCount_Visible() const;//Return the number of columns visibled
    void f_InsertRowByPressKey();
    void f_RemoveSelectedRow(QKeyEvent *event);
    void f_SetRowSelectionUpDown(bool bUpOrDown, bool bLoop = true);
    void f_SetCellSelectionLeftRight(bool bLeftOrRight, bool bLoop = true);
    void f_FillRowByJsonObject(int iRow, QJsonObject oJsonRow);
    void f_MadeNewRowJson(QJsonObject* oJsonNewRow, int iRow);
    QJsonObject f_InsertJsonRow(int iRow);
};

#endif // HQ_BASE_DB_GRID_H
