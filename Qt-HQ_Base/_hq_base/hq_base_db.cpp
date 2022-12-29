#include "hq_base_db.h"
#include <QtSql>
#include <QMutex>
#include "hq_base.h"

QMutex       HQ_Base_DB::m_Mutex;
QSqlDatabase HQ_Base_DB::m_db;

HQ_Base_DB::HQ_Base_DB()
{

}

/*================================================================
 * Get the connection infomation.
 * Such as hostname, port, database's name, username, password.
 *===============================================================*/
bool HQ_Base_DB::f_GetConnectInfo(QString *sDB_Type, QString *sHost, QString *sPort,
                                  QString *sDB_Name, QString *sUser, QString *sPwd,
                                  QString *sErr)
{
    QStringList sKeyList = {"db/db_type", "db/hostname", "db/port",
                           "db/db_name", "db/username", "db/password"};

    QJsonObject oJson = HQ_Base::f_INI_GetValue(sKeyList, sErr);
    if (!oJson.isEmpty())
    {
        *sDB_Type = oJson.value("db/db_type").toString().toLower();
        *sHost    = oJson.value("db/hostname").toString();
        *sPort    = oJson.value("db/port").toString();
        *sDB_Name = oJson.value("db/db_name").toString();
        *sUser    = oJson.value("db/username").toString();
        *sPwd     = oJson.value("db/password").toString();
        return true;
    }
    else
    {
        return false;
    }
}

/*================================================================
 * The common function, used to open/close/detect database.
 * Return true on successed, otherwise return false.
 *===============================================================*/
bool HQ_Base_DB::f_OpenDB(QString *sErr)
{
    if (m_db.isOpen())
    {
        return true;
    }
    else
    {
        qDebug() << "db open";
        return f_OpenDB(&m_db, sErr);
    }
}
bool HQ_Base_DB::f_OpenDB(QSqlDatabase *db, QString *sErr)
{
    QString sErrPrefix = "The database openning failed.";
    QString sDB_Type, sHost, sPort, sUser, sPwd, sDB_Name;
    if (!f_GetConnectInfo(&sDB_Type, &sHost, &sPort, &sDB_Name, &sUser, &sPwd, sErr))
    {
        return false;
    }

    if (nullptr == db)
    {
        HQ_Base::f_Pointer_SetValue(sErr, sErrPrefix + " For the db object being null.");
        return false;
    }

    bool bReturn;
    QString sErrReturn;
    if (sDB_Type == "sqllite")
    {
        bReturn = f_OpenDB_SqlLite(sDB_Name, db, sErr);
        sErrReturn = HQ_Base::f_Pointer_GetValue(sErr);
    }
    else if (sDB_Type == "mysql")
    {
        bReturn = f_OpenDB_Mysql(sHost, sPort, sDB_Name, sUser, sPwd, db, sErr);
        sErrReturn = HQ_Base::f_Pointer_GetValue(sErr);
    }
    else if (sDB_Type == "access")
    {
        bReturn = f_OpenDB_Access(sDB_Name, db, sErr);
        sErrReturn = HQ_Base::f_Pointer_GetValue(sErr);
    }
    else
    {
        bReturn = false;
        sErrReturn = "type err. Please check the ini config file.";
    }

    if (!bReturn)
    {
        f_CloseDB(db);
    }

    sErrReturn = sErrReturn.isEmpty() ? "" : sErrPrefix + " For " + sErrReturn;
    HQ_Base::f_Pointer_SetValue(sErr, sErrReturn);
    return bReturn;
}
void HQ_Base_DB::f_CloseDB()
{
    qDebug() << "db close";
    f_CloseDB(&m_db);
}
void HQ_Base_DB::f_CloseDB(QSqlDatabase *db)
{
    if (nullptr == db)
    {
        return;
    }
    if (db->isOpen())
    {
        db->close();
    }
}
void HQ_Base_DB::f_DB_FileFree(QString sFile)
{
    QFile f(sFile);
    if (f.isOpen())
    {
        f.close();
    }
}
bool HQ_Base_DB::f_IsDatabaseOK(QSqlDatabase *db, QString *sErr)
{
    if (f_OpenDB(db, sErr))
    {
        f_CloseDB(db);
        return true;
    }
    else
    {
        f_CloseDB(db);
        return false;
    }
}
QString HQ_Base_DB::f_GetConnectionName()
{
    QString sHour    = QString::number(QTime::currentTime().hour());
    QString sMinute  = QString::number(QTime::currentTime().minute());
    QString sSecond  = QString::number(QTime::currentTime().second());
    QString sMsecond = QString::number(QTime::currentTime().msec());
    return QString("nxDB_%1_%2_%3_%4").arg(sHour).arg(sMinute).arg(sSecond).arg(sMsecond);
}
/*================================================================
 * Open a mysql database with a given connection string of the
 * database. Return true on successed, and the database variant
 * will be set in the parameter db. Otherwise return false.
 *===============================================================*/
bool HQ_Base_DB::f_OpenDB_Mysql(QString sHost, QString sPort, QString sDB_Name,
                                QString sUser, QString sPwd, QSqlDatabase *db,
                                QString *sErr)
{
    if (sDB_Name.isEmpty() || nullptr == db)
    {
        HQ_Base::f_Pointer_SetValue(sErr, "The database message lost. Can not open database.");
        return false;
    }

    try
    {
        *db = QSqlDatabase::addDatabase("QMYSQL", f_GetConnectionName());
        db->setHostName(sHost);
        db->setPort(sPort.toInt());
        db->setDatabaseName(sDB_Name);
        db->setUserName(sUser);
        db->setPassword(sPwd);
        if (!db->open())
        {
            HQ_Base::f_Pointer_SetValue(sErr, db->lastError().text());
            return false;
        }
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, db->lastError().text());
        return false;
    }
}

/*================================================================
 * Open a SqlLite database with a given connection string of the
 * database. Return true on successed, and the database variant
 * will be set in the parameter db. Otherwise return false.
 *===============================================================*/
bool HQ_Base_DB::f_OpenDB_SqlLite(QString sDB_Name, QSqlDatabase *db, QString *sErr)
{
    if (sDB_Name.isEmpty() || nullptr == db)
    {
        HQ_Base::f_Pointer_SetValue(sErr, "The database message lost. Can not open database.");
        return false;
    }

    f_DB_FileFree(sDB_Name);

    try
    {
        *db = QSqlDatabase::addDatabase("QSQLITE", f_GetConnectionName());
        db->setDatabaseName(sDB_Name);
        if (!db->open())
        {
            HQ_Base::f_Pointer_SetValue(sErr, db->lastError().text());
            return false;
        }
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, db->lastError().text());
        return false;
    }
}

/*================================================================
 * Open a access database with a given connection string of the
 * database. Return true on successed, and the database variant
 * will be set in the parameter db. Otherwise return false.
 *===============================================================*/
bool HQ_Base_DB::f_OpenDB_Access(QString sDB_Name, QSqlDatabase *db, QString *sErr)
{
    if (sDB_Name.isEmpty() || nullptr == db)
    {
        HQ_Base::f_Pointer_SetValue(sErr, "The database message lost. Can not open database.");
        return false;
    }

    f_DB_FileFree(sDB_Name);

    try
    {
        *db = QSqlDatabase::addDatabase("QODBC", f_GetConnectionName());
        db->setDatabaseName(sDB_Name);
        if (!db->open())
        {
            HQ_Base::f_Pointer_SetValue(sErr, db->lastError().text());
            return false;
        }
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }
    catch (...)
    {
        HQ_Base::f_Pointer_SetValue(sErr, db->lastError().text());
        return false;
    }
}
/*================================================================
 * Execute a SQL statement without any query result.
 * Return true on successed,otherwise return false.
 *===============================================================*/
bool HQ_Base_DB::f_ExecSqlNoneQuery(QString sSQL, QString *sErr)
{
    QStringList sSQLlist = {sSQL};
    return f_ExecSqlNoneQuery(sSQLlist, sErr);
}
bool HQ_Base_DB::f_ExecSqlNoneQuery(QStringList sSQLlist, QString *sErr)
{    
    QMutexLocker locker(&m_Mutex);

//    QString sConnectionName;
//    {
//        QSqlDatabase m_db;
//        if (!f_OpenDB(&m_db, sErr))
//        {
//            return false;
//        }
//        sConnectionName = m_db.connectionName();
//        //...
//        f_CloseDB(&m_db);
//    }
//    QSqlDatabase::removeDatabase(sConnectionName);

    if (!f_OpenDB(sErr))
    {
        return false;
    }
    QSqlQuery q(m_db);
    QSqlDatabase::database().transaction();
    for (int i = 0; i < sSQLlist.size(); i++)
    {
        if (!q.exec(sSQLlist[i]))
        {
            HQ_Base::f_Pointer_SetValue(sErr, q.lastError().text());
            QSqlDatabase::database().rollback();
            return false;
        }
    }
    QSqlDatabase::database().commit();
    HQ_Base::f_Pointer_SetValue(sErr, "");
    return true;
}
/*================================================================
 * Return the result as json object or json string with given sql
 * statement.
 *
 * Use the isEmpty() function of the return value to judge executing
 * sql statement successed or failed.
 *
 * The argument sErr is a optional item, it can be pointed to a
 * variable. The value of variable will be "" on successed of the
 * function execute, otherwise it will be the err messaage.
 *
 * Format:
 * {
 *      "Cols":"Field1|Field2|...",
 *      "Rows":
 *      [
 *          {"Field1":"value","Field2":"value",...},
 *          {"Field1":"value","Field2":"value",...},
 *          ...
 *      ]
 * }
 *===============================================================*/
QString HQ_Base_DB::f_GetQueryAsJsonStr(QString sSQL, QString *sErr)
{
    QJsonObject oJson = f_GetQueryAsJsonObj(sSQL, sErr);
    if (oJson.isEmpty())
    {
        return "";
    }

    return HQ_Base::f_Json_ToString(oJson, sErr);;
}
QJsonObject HQ_Base_DB::f_GetQueryAsJsonObj(QString sSQL, QString *sErr)
{
    QMutexLocker locker(&m_Mutex);
    QJsonObject oJsonObject;

//    QString sConnectionName;
//    {
//        QSqlDatabase m_db;
//        if (f_OpenDB(m_db, sErr))
//        {
//            sConnectionName = m_db.connectionName();
//            //...
//        }
//        f_CloseDB(&m_db);
//    }
//    QSqlDatabase::removeDatabase(sConnectionName);

    if (f_OpenDB(sErr))
    {
        QSqlQuery q(sSQL, m_db);
        QString sDbErr = q.lastError().databaseText();
        QString sDriverErr = q.lastError().driverText();
        QString sQueryErr = q.lastError().text();
        if (sDbErr.isEmpty() && sDriverErr.isEmpty())
        {
            oJsonObject = f_ConvertQueryToJsonObj(&q, sErr);
        }
        else
        {
            HQ_Base::f_Pointer_SetValue(sErr, sQueryErr);
        }
    }
    return oJsonObject;
}
/**===============================================================
 ** Get the col num or row num of the result.
 ** Because the json object of the result was queried from the db.
 ** Sometime the json object will be empty on some err,
 ** for example: The given table does not exist.
 ** Sometime the json object was not empty but has no rows,
 ** for example: The given table exists, but no records.
 ** So the number of cols and rows is necessary.
 **
 ** Format:
 ** {
 **      "Cols":"Field1|Field2|...",
 **      "Rows":
 **      [
 **          {"Field1":"value","Field2":"value",...},
 **          {"Field1":"value","Field2":"value",...},
 **          ...
 **      ]
 ** }
 **===============================================================*/
int HQ_Base_DB::f_GetColCount(QJsonObject oJsonResult)
{
    return oJsonResult.value("Cols").toString().split('|', QString::SkipEmptyParts).size();
}
int HQ_Base_DB::f_GetColCount(QString sJsonResult)
{
    QJsonObject oJsonResult = HQ_Base::f_Json_StringToJsonObj(sJsonResult);
    return f_GetColCount(oJsonResult);
}
int HQ_Base_DB::f_GetRowCount(QJsonObject oJsonResult)
{
    return oJsonResult.value("Rows").toArray().size();
}
int HQ_Base_DB::f_GetRowCount(QString sJsonResult)
{
    QJsonObject oJsonResult = HQ_Base::f_Json_StringToJsonObj(sJsonResult);
    return f_GetRowCount(oJsonResult);
}
/*================================================================
 * Convert query result to json.
 * Format:
 * {
 *      "Cols":"Field1|Field2|...",
 *      "Rows":[
 *          {"Field1":"value","Field2":"value",...},
 *          {"Field1":"value","Field2":"value",...},
 *          ...
 *      ]
 * }
 *===============================================================*/
QJsonObject HQ_Base_DB::f_ConvertQueryToJsonObj(QSqlQuery *query, QString *sErr)
{
    QJsonObject oJsonObject;

    //made the column infomation
    QStringList sColInfoList = f_GetColsListByQuery(query, sErr);
    if (sColInfoList.isEmpty())
    {
        return oJsonObject;
    }
    QString sColInfo = sColInfoList.join("|");

    //put the column information in json root, as a key 'Cols'
    oJsonObject.insert("Cols", sColInfo);

    //get the column count
    QSqlRecord oRec = query->record();
    int iColNum = oRec.count();

    //made the row object array
    QJsonArray oArrRow;
    QJsonObject oJsonRow;
    while (query->next())
    {
        oRec = query->record();

        //made the row object with columns.
        for (int iCol = 0; iCol < iColNum; iCol++)
        {
            oJsonRow.insert(oRec.fieldName(iCol), oRec.value(iCol).toString());
        }

        oArrRow.append(oJsonRow);
    }

    if (oArrRow.size() > 0)
    {
        //put the row array in json root, as a key 'Rows'
        oJsonObject.insert("Rows", oArrRow);
    }

    HQ_Base::f_Pointer_SetValue(sErr, "");
    return oJsonObject;
}
/*===============================================================
 * Return a sql statement string list by given string named 'modified',
 * the string was generated by getmodify().
 * Used for function f_Save.
 * Return ture on successed, the sql string list is in argument
 * 'sListSQL'.
 * Return false on failed, the err info is in argument sErr.
 *===============================================================*/
bool HQ_Base_DB::f_GetSqlListByModify(QString sModified, QStringList *sListSQL, QString *sErr)
{
    //Get the modified json object if it is valid.
    if (sModified.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }
    QJsonObject oJson = HQ_Base::f_Json_StringToJsonObj(sModified, sErr);
    if (oJson.isEmpty())
    {
        QString s = "Saving failed, for modified string can not convert to json object.";
        HQ_Base::f_Pointer_SetValue(sErr, s);
        return false;
    }

    //Get the table name.
    QString sTableName = oJson.value("TableName").toString();
    if (sTableName.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }

    //Get the cols/fields list.
    //Cols or fields are all right. Only one is necessary.
    //QStringList sCollist = oJson.value("Cols").toString().split("|");
    QStringList sFieldList = oJson.value("Fields").toString().split("|");
    if (sFieldList.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }

    //Generate the sql of every modified data row.
    QJsonArray oArrRows = oJson.value("Rows").toArray();
    QStringList sSqlList;
    for (int iRow = 0; iRow < oArrRows.size(); iRow++)
    {
        QString sRowSQL;
        QJsonObject oJsonRow = oArrRows.at(iRow).toObject();
        QJsonValue vRowState = oJsonRow.value("fState");
        if (vRowState.isUndefined() || vRowState.isNull() || vRowState.toString() == "")
        {
            continue;
        }
        else if (vRowState.toString() == "0" || vRowState.toString() == "1"
                 || vRowState.toString() == "-1")
        {
            //If cur row is a modified row.
            if (vRowState.toString() == "1")
            {
                QString sColSet;
                for (int iCol = 0; iCol < sFieldList.size(); iCol++)
                {
                    if (sFieldList[iCol] == "fGUID")
                    {
                        continue;
                    }
                    QString sCurField = sFieldList[iCol];
                    QString sCurValue =
                            oJsonRow.value(sCurField).toString().replace("'", "''");
                    sColSet += sCurField + "='" + sCurValue + "',";
                }

                //remove the last ","
                sColSet = sColSet.left(sColSet.length() - 1);

                sRowSQL = QString("update %1 set %2").arg(sTableName).arg(sColSet);
            }
            //If cur row is a new row.
            else if (vRowState.toString() == "0")
            {
                QString sFields;
                QString sValues;
                for (int iCol = 0; iCol < sFieldList.size(); iCol++)
                {
                    QString sCurField = sFieldList[iCol];
                    QString sCurValue =
                            oJsonRow.value(sCurField).toString().replace("'", "''");
                    sFields += sFieldList[iCol] + ",";
                    sValues += QString("'%1',").arg(sCurValue);
                }
                //remove the last ","
                sFields = sFields.left(sFields.length() - 1);
                sValues = sValues.left(sValues.length() - 1);

                sRowSQL = QString("insert into %1 (%2) values (%3)")
                        .arg(sTableName).arg(sFields).arg(sValues);
            }
            //If cur row is a deleted row.
            else if (oJsonRow.value("fState").toString() == "-1")
            {
                sRowSQL = QString("delete from %1").arg(sTableName);
            }

            QString sID = oJsonRow.value("fGUID").toString();

            //The insert sql statement needn't 'where'.
            if (oJsonRow.value("fState").toString() != "0")
            {
                sRowSQL += QString(" where fGUID='%1'").arg(sID);
            }

            //Add the row sql to the sql list.
            //qDebug() << sRowSQL;
            sSqlList << QString("%1;").arg(sRowSQL);
        }
    }

    *sListSQL = sSqlList;
    HQ_Base::f_Pointer_SetValue(sErr, "");
    return true;
}
/*================================================================
 * Save to database by given json string, for single table.
 * Return true on success, otherwise return false.
 *  About modified json string:
 *
 *       {
 *           "Cols": "Field1_Name|Field2_Name|...",
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
 *===============================================================*/
bool HQ_Base_DB::f_Save(QString sModified, QString *sErr)
{
    //Generate the sql list of single table.
    QStringList sSqlList;
    if (!f_GetSqlListByModify(sModified, &sSqlList, sErr))
    {
        return false;
    }

    //Execute the sql list to save data.
    if (!f_ExecSqlNoneQuery(sSqlList, sErr))
    {
        return false;
    }
    HQ_Base::f_Pointer_SetValue(sErr, "");
    return true;
}
/*================================================================
 * Save to database by given json string, for multi tables.
 * Return true on success, otherwise return false.
 *  About modified json string:
 *  [
 *       {
 *           "Cols": "Field1_Name|Field2_Name|...",
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
 *       },
 *       {
 *           "Cols": "Field1_Name|Field2_Name|...",
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
 *  ]
 *===============================================================*/
bool HQ_Base_DB::f_SaveAll(QString sModified, QString *sErr)
{
    if (sModified.isEmpty())
    {
        HQ_Base::f_Pointer_SetValue(sErr, "");
        return true;
    }

    //Convert the json string of multi tables to the array of single table.
    QJsonArray oArr = HQ_Base::f_Json_StringToJsonArr(sModified, sErr);
    if (oArr.isEmpty())
    {
        QString s = "Saving failed, for modified string can not convert to json array.";
        HQ_Base::f_Pointer_SetValue(sErr, s);
        return false;
    }

    //Generate the row sql list of every table.
    QStringList sSqlList;
    for (int i = 0; i < oArr.size(); i++)
    {
        QJsonObject oJson = oArr.at(i).toObject();
        if (oJson.isEmpty())
        {
            continue;
        }
        QString sModify_SingleTable = HQ_Base::f_Json_ToString(oJson, sErr);
        if (sModify_SingleTable.isEmpty())
        {
            return false;
        }
        QStringList list;
        if (!f_GetSqlListByModify(sModify_SingleTable, &list, sErr))
        {
            return false;
        }
        sSqlList << list;
    }

    //Execute the sql list to save data.
    if (!f_ExecSqlNoneQuery(sSqlList, sErr))
    {
        return false;
    }
    HQ_Base::f_Pointer_SetValue(sErr, "");
    return true;
}
/*===============================================================
 * Return a string list of cols by given query object.
 * Return a empty list on error.
 *===============================================================*/
QStringList HQ_Base_DB::f_GetColsListByQuery(QSqlQuery *query, QString *sErr)
{
    QStringList sColInfoList;

    if (query == nullptr)
    {
        HQ_Base::f_Pointer_SetValue(sErr, "Query object error, convert to json object failed.");
        return sColInfoList;
    }

    //get the column count
    QSqlRecord oRec = query->record();
    int iColNum = oRec.count();

    //made the column infomation
    for (int iCol = 0; iCol < iColNum; iCol++)
    {
        sColInfoList << oRec.fieldName(iCol);
    }

    HQ_Base::f_Pointer_SetValue(sErr, "");
    return sColInfoList;
}
/*===============================================================
 * Return a string list of fields by given table name.
 * Return a empty list on error.
 *===============================================================*/
QStringList HQ_Base_DB::f_GetFieldsListByTableName(QString sTableName, QString *sErr)
{
    QString sSQL = QString("select * from %1 where 1=2").arg(sTableName);
    return f_GetColsListBySql(sSQL, sErr);
}
/*===============================================================
 * Return a string list of fields by given sql.
 * Return a empty list on error.
 *===============================================================*/
QStringList HQ_Base_DB::f_GetColsListBySql(QString sSQL, QString *sErr)
{
    QMutexLocker locker(&m_Mutex);
    QStringList sColInfoList;
//    QString sConnectionName;
//    {
//        QSqlDatabase m_db;
//        if (f_OpenDB(&m_db, sErr))
//        {
//            sConnectionName = m_db.connectionName();
//            //...
//        }
//        f_CloseDB(&m_db);
//    }
//    QSqlDatabase::removeDatabase(sConnectionName);

    if (f_OpenDB(sErr))
    {
        QSqlQuery q(sSQL, m_db);
        sColInfoList = f_GetColsListByQuery(&q, sErr);
    }

    HQ_Base::f_Pointer_SetValue(sErr, "");
    return sColInfoList;
}
