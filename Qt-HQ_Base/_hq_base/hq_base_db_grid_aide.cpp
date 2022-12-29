#include "hq_base_db_grid_aide.h"
#include <QKeyEvent>
#include "hq_base.h"
#include "hq_base_db.h"
#include "hq_base_db_model.h"
#include "hq_base_db_grid.h"

HQ_Base_Db_Grid_Aide::HQ_Base_Db_Grid_Aide()
{

}
/*===============================================================
 * Fill the grid header.
 *===============================================================*/
bool HQ_Base_Db_Grid_Aide::f_FillHeader(HQ_Base_Db_Grid *oGrid,
                                        QString         sModelName,
                                        QString         *sErr)
{
    QString sErrPrefix = "Grid header filling failed.";
    QString sModel = HQ_Base_Db_Model::f_GetModel(sModelName, sErr);
    if (sModel.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix
                              + QString(" \nModelName: %1\n").arg(sModelName)
                              + *sErr);
        return false;
    }

    if (!oGrid->f_FillHeader(sModel, sErr))
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix
                              + QString(" \nModelName: %1").arg(sModelName)
                              + QString(" \nModel:\n %1\n").arg(sModel)
                              + *sErr);
        return false;
    }

    HQ_Base::f_Pointer_SetValue(sErr, "");
    return true;
}
/*===============================================================
 * Return contents of the grid by given model name and fk value.
 *===============================================================*/
QString HQ_Base_Db_Grid_Aide::f_GetContents(QString sModelName,
                                            QString *sErr)
{
    return f_GetContents(sModelName, "", "", sErr);
}
QString HQ_Base_Db_Grid_Aide::f_GetContents(QString sModelName,
                                            QString sFK_Value,
                                            QString *sErr)
{
    return f_GetContents(sModelName, "", sFK_Value, sErr);
}
QString HQ_Base_Db_Grid_Aide::f_GetContents(QString sModelName,
                                            QString sWhere,
                                            QString sFK_Value,
                                            QString *sErr)
{
    QString sErrPrefix = "Grid contents gotting failed.";

    try
    {
        QString sSQL = HQ_Base_Db_Model::f_GetSqlByFk(sModelName, sWhere, sFK_Value, sErr);
        if (sSQL.isEmpty())
        {
            throw QString(" Sql gotting failed.\n");
        }

        QString sContents = HQ_Base_DB::f_GetQueryAsJsonStr(sSQL, sErr);
        if (sContents.isEmpty())
        {
            throw QString("\n");
        }

        return sContents;
    }
    catch (QString sMsg)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + sMsg + *sErr);
        return "";
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " unknown err.\n" + *sErr);
        return "";
    }
}
/*===============================================================
 * Fill the grid contents.
 * It depends on the grid header and given fk_value.
 * It must to be called after the header filling.
 *===============================================================*/
bool HQ_Base_Db_Grid_Aide::f_FillContents(HQ_Base_Db_Grid   *oGrid,
                                          QString           *sErr)
{
    return f_FillContents(oGrid, "", "", sErr);
}
bool HQ_Base_Db_Grid_Aide::f_FillContents(HQ_Base_Db_Grid   *oGrid,
                                          QString           sFK_Value,
                                          QString           *sErr)
{
    return f_FillContents(oGrid, "", sFK_Value, sErr);
}
bool HQ_Base_Db_Grid_Aide::f_FillContents(HQ_Base_Db_Grid   *oGrid,
                                          QString           sWhere,
                                          QString           sFK_Value,
                                          QString           *sErr)
{
    QString sErrPrefix = "Grid filling failed.";

    try
    {
        QString sModel = oGrid->f_GetHeader(sErr);
        if (sModel.isEmpty())
        {
            throw QString(" FillHeader first.\n");
        }

        QJsonObject oModel = HQ_Base::f_Json_StringToJsonObj(sModel, sErr);
        if (oModel.isEmpty())
        {
            throw QString(" \nModel: %1\n").arg(sModel);
        }

        QString sSQL = HQ_Base_Db_Model::f_GetSqlByFk(oModel, sWhere, sFK_Value, sErr);
        if (sSQL.isEmpty())
        {
            throw QString(" \nModel: %1.\n").arg(sModel);
        }

        QString sContents = HQ_Base_DB::f_GetQueryAsJsonStr(sSQL, sErr);
        if (sContents.isEmpty())
        {
            throw QString(" Contents getting failed.\nSQL:\n%1").arg(sSQL);
        }        

        if (!oGrid->f_FillContents(sContents, sErr))
        {
            throw QString("\n Sql:\n %1\n Contents:\n %2\n").arg(sSQL).arg(sContents);
        }

        oGrid->f_SetForeignKeyValue(sFK_Value);
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }
    catch (QString sMsg)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + sMsg + *sErr);
        return false;
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " Unknown err.\n" + *sErr);
        return false;
    }
}
/*===============================================================
 * Fill header and contents.
 *===============================================================*/
bool HQ_Base_Db_Grid_Aide::f_Fill(HQ_Base_Db_Grid   *oGrid,
                                  QString           sModelName,
                                  QString           sFK_Value,
                                  QString           *sErr)
{
    if (!f_FillHeader(oGrid, sModelName, sErr))
    {
        return false;
    }

    if (!f_FillContents(oGrid, sFK_Value, sErr))
    {
        return false;
    }

    HQ_Base::f_Pointer_SetValue(sErr, "");
    return true;
}
bool HQ_Base_Db_Grid_Aide::f_Fill(HQ_Base_Db_Grid   *oGrid,
                                  QString           sModelName,
                                  QString           *sErr)
{
    return f_Fill(oGrid, sModelName, "", sErr);
}
/*===============================================================
 * Remove the data that depends on the current data.
 *
 * The current data was selected by user, it will be deleted if
 * user confirmed.
 *
 * If it was used by some other data. Then those 'other data' was
 * called 'sub data'. If the current data was deleted, those sub
 * data will lost the data depended and became to useless.
 *
 * So this function provides a method. When the current data will
 * be deleted, the sub data will be deleted at the same time if
 * user confirmed.
 *
 * The return value and err output arguments is not necessary.
 * Because the function can work and finish automatically.
 *
 * About arguments:
 *
 *     *event:
 *
 *         Used for whether continue the deleting operation. Use
 *     function 'event->ignore' to give up,  use 'event->accept'
 *     to continue.
 *
 *     sCurDataName/sSubDataName:
 *
 *         They are the desc info used for output the message.
 *
 *     sCurDataID_List:
 *
 *         It stores the GUID list of selected row in the grid.
 *
 *     sSubDataFkField:
 *
 *         It is the foreignkey's field name of sub data.
 *
 *     sSubTableName:
 *
 *         It is the table name of the sub data.
 *
 *===============================================================*/
void HQ_Base_Db_Grid_Aide::f_RemoveSubdata_Auto(QKeyEvent   *event,
                                                QString     sCurDataName,
                                                QStringList sCurDataID_List,
                                                QString     sSubDataName,
                                                QString     sSubTableName,
                                                QString     sSubDataFkField)
{
    try
    {
        //Confirm whether more than one curren data was selected.
        if (sCurDataID_List.isEmpty() || sCurDataID_List.size() <= 0)
        {
            throw QString("");
        }

        //Make the where condition.
        QString sWhere = " where 1=2";
        for (QString sID : sCurDataID_List)
        {
            sWhere += QString(" or %1='%2'").arg(sSubDataFkField).arg(sID);
        }

        //Comfirm whether the rights info of the selected user exist in the database.
        QString sSQL = QString("select 1 from %1").arg(sSubTableName);
        sSQL += sWhere;

        QString sErr;
        QJsonObject oJson = HQ_Base_DB::f_GetQueryAsJsonObj(sSQL, &sErr);
        if (!sErr.isEmpty())
        {
            throw sErr;
        }
        if (oJson.isEmpty())
        {
            throw QString("");
        }

        //Show the msgbox.
        QString sTitle = "Are you sure to continue deleting?";
        QString sMessage = QString("    The selected '%1' data has been already referenced by '%2' data."
                                   " The '%2' data depends on '%1' data, they will be deleted at the same"
                                   " time if continue.\n"
                                   "    Pay attention! If you selected 'Yes', the '%2' data who depends on"
                                   " '%1' will be deleted surely, even the deleted '%1' data was not saved"
                                   " yet. This operation can not be undo. Do you want to continue?\n"
                                   "    Click 'Yes' to delete current selected '%1' data and its '%2' data.\n"
                                   "    Click 'Cancel' to cancel operation.").arg(sCurDataName).arg(sSubDataName);

        QMessageBox::StandardButton msgBtn =
                QMessageBox::question(0, sTitle, sMessage,
                                      QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Yes);
        if (msgBtn != QMessageBox::Yes)
        {
            throw QString("");
        }

        /* It is unnecessary to delete the current selected info at first. Or say,
         * the current selected info can not be deleted here. Because at this time
         * the data in grid has not been deleted and saved yet.  After asking here,
         * the grid can decide whether continue the deleting by user selection.
         */

        sSQL = QString("delete from %1").arg(sSubTableName);
        sSQL += sWhere;

        bool b = HQ_Base_DB::f_ExecSqlNoneQuery(sSQL, &sErr);
        if (!b)
        {
            throw sErr;
        }

        event->accept();
        return;
    }
    catch(QString sMsg)
    {
        if (!sMsg.isEmpty())
        {
            QMessageBox::warning(0, "Deleting failed.",
                                 QString("All the operation canceled.\n") + sMsg,
                                 QMessageBox::Ok,QMessageBox::NoButton);
        }
        event->ignore();
        return;
    }
    catch (...)
    {
        QMessageBox::warning(0, "Deleting failed.",
                             QString("All the operation canceled. Unkown err.\n"),
                             QMessageBox::Ok,QMessageBox::NoButton);
        event->ignore();
        return;
    }
}
