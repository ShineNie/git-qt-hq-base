/************************************************************************
 *  Class name:     HQ_Base_Db_Grid_Aide
 *  Author:         NieXin
 *  Created date:   2022-7-11
 *  Used for:       Help the grid better to work automatically
 *  Desc:
 *
 *          This class works as a assistent of the hq_base_db_grid. It can
 *      help the grid working faster and simpler.
 *
 *  About function 'f_FillContents':
 *
 *          It is used for filling the contents of the grid. The model of
 *      the grid called 'header' is required. If the model of the grid is
 *      empty, It will executing failed with error message.
 *
 *          This function has several overloads for using easily.
 *
 *  About function 'f_RemoveSubdata':
 *
 *          It is used for deleting the sub data of the selected current
 *      data. The more desc can be find at the first of the function note.
 *
 ************************************************************************/
#ifndef HQ_BASE_DB_GRID_AIDE_H
#define HQ_BASE_DB_GRID_AIDE_H

#include "qt-hq_base_global.h"
/* In every place that class 'HQ_Base_Db_Grid_Aide' exists. Mostly the messagebox
 * is necessary. So the header file of messagebox was included here directly.
 */
#include <QMessageBox>

class HQ_Base_Db_Grid;
class QString;
class QStringList;
class QKeyEvent;

class QTHQ_BASESHARED_EXPORT HQ_Base_Db_Grid_Aide
{
public:
    HQ_Base_Db_Grid_Aide();

    static QString f_GetContents(QString sModelName,                                    QString *sErr = nullptr);
    static QString f_GetContents(QString sModelName,                 QString sFK_Value, QString *sErr = nullptr);
    static QString f_GetContents(QString sModelName, QString sWhere, QString sFK_Value, QString *sErr = nullptr);

    static bool f_FillHeader  (HQ_Base_Db_Grid *oGrid, QString sModelName,                    QString *sErr = nullptr);
    static bool f_Fill        (HQ_Base_Db_Grid *oGrid, QString sModelName, QString sFK_Value, QString *sErr = nullptr);
    static bool f_Fill        (HQ_Base_Db_Grid *oGrid, QString sModelName,                    QString *sErr = nullptr);
    static bool f_FillContents(HQ_Base_Db_Grid *oGrid, QString sWhere,     QString sFK_Value, QString *sErr = nullptr);
    static bool f_FillContents(HQ_Base_Db_Grid *oGrid,                     QString sFK_Value, QString *sErr = nullptr);
    static bool f_FillContents(HQ_Base_Db_Grid *oGrid,                                        QString *sErr = nullptr);

    static void f_RemoveSubdata_Auto(QKeyEvent *event, QString sCurDataName, QStringList sCurDataID_List,
                                     QString sSubDataName, QString sSubTableName, QString sSubDataFkField);

};

#endif // HQ_BASE_DB_GRID_AIDE_H
