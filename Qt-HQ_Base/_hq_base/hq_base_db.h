/************************************************************************
 *  Class name:     HQ_Base_DB
 *  Author:         NieXin
 *  Created date:   2022-05-17
 *  Used for:       Database related
 *  Such as:        open database, get data, execute sql, and so on.
 *  Return value:
 *
 *          If the function is used for query result, the type of return
 *      value will be QSqlQuery, QJsonObject and QString.
 *
 *          If the function is used for execute sql statement without
 *      query, or used for open or close database, the type of return
 *      value is bool. True means successed, false means failed.
 *
 *          If more type of values need to be returned, such as error
 *      message, more arguments, and so on. It(or they) will be used as
 *      a pointer. When the function is called, the input argument needs
 *      using '&' operator as the variant's address, such as:
 *
 *          HQ_Base_DB *hq_base_db = new HQ_Base_DB();
 *          hq_base_db->openDB(&db).
 *          delete hq_base_db;
 *
 *          So as to realize address reference. So, when the function was
 *      executed, the argument 'db' gets the new value you want.
 *
 *          Most function posiblely uses a optional argument named 'sErr'
 *      to output err message. The 'sErr' argument's type is string pointer.
 *      If the function executed failed, the pointer argument 'sErr' will
 *      output the error message. Such as:
 *
 *          QJsonObject getJsonBySql(QString sSQL,
 *              QString *sErr = nullptr);
 *
 *          Then the err message is in 'sErr' variable. But pay attention!
 *      If the optional pointer has no given value, the pointer's value will
 *      be nullptr by default. So the code in function must to detect the
 *      pointer's value before using it. If ignore that, when a value was be
 *      set to the null pointer(nullptr), it errors.
 *
 *  Usage method:
 *
 *      Pay attention: If a database object was newed for using, it must be delete after using.
 *
 *      //detect the database connection, with the optional err message
 *      //argument.
 *
 *          QString sErr;
 *          QSqlDatabase db;
 *          HQ_Base_DB *hq_base_db = new HQ_Base_DB();
 *          bool b = hq_base_db->f_IsDatabaseOK(&db, &sErr);
 *          delete hq_base_db;
 *          if (b)
 *          {
 *              successed;
 *          }
 *          else
 *          {
 *              failed;
 *              show err message;
 *          }
 *
 *      //get a query result as json string, with returning optional err
 *      //message.
 *
 *          QString sSQL = "select * from t;";
 *          QString sErr;
 *          HQ_Base_DB *hq_base_db = new HQ_Base_DB();
 *          QString sContents = hq_base_db->f_GetQueryAsJsonStr(sSQL, &sErr);
 *          delete hq_base_db;
 *          if (sErr != "")
 *          {
 *              query failed, err message in 'sErr'.;
 *          }
 *
 *      //get a query result as json string, WITHOUT returning optional err
 *      //message.
 *
 *          QString sSQL = "select * from t;";
 *          QString sErr;
 *          HQ_Base_DB *hq_base_db = new HQ_Base_DB();
 *          QString sContents = hq_base_db->f_GetQueryAsJsonStr(sSQL, &sErr);
 *          delete hq_base_db;
 *          if (sContents == "")
 *          {
 *              query failed;
 *          }
 *
 *      //execute a sql statement without any query result, without the
 *      //optional err message.
 *
 *          HQ_Base_DB *hq_base_db = new HQ_Base_DB();
 *          bool b = hq_base_db->f_ExecSqlNoneQuery("");
 *          delete hq_base_db;
 *          if (!b)
 *          {
 *              execute failed;
 *          }
 *
 *          If it was used by the static way, do not need to free memory. using
 *      as below:
 *
 *          HQ_Base_DB::f_ExecSqlNoneQuery("");
 *          HQ_Base_DB::f_GetQueryAsJsonStr(sSQL, &sErr);
 *          ...
 *
 * Field name:
 *
 *          Two fields must be required. One is the primary key named 'fGUID',
 *      another is the optional foriegn key 'fParentID'.
 *
 *          For any data, it is must necessary to have a primary key named
 *      'fGUID' . Another case, if the data used for a sub grid that depend
 *      on main grid, the foriegn key 'fParentID' will be required.
 *
 ************************************************************************/

#ifndef HQ_BASE_DB_H
#define HQ_BASE_DB_H

#include "qt-hq_base_global.h"
#include <QObject>
class QSqlDatabase;
class QSqlQuery;
class QJsonObject;
class QMutex;

class QTHQ_BASESHARED_EXPORT HQ_Base_DB : public QObject
{
    Q_OBJECT
public:
    HQ_Base_DB();

    //Detect the database connection
    static bool f_IsDatabaseOK(QSqlDatabase *db, QString *sErr = nullptr);

    //Execute a SQL statement without query
    static bool f_ExecSqlNoneQuery(QString     sSQL,     QString *sErr = nullptr);
    static bool f_ExecSqlNoneQuery(QStringList sSQLlist, QString *sErr = nullptr);

    //Get the result with given sql statement
    static QJsonObject f_GetQueryAsJsonObj(QString sSQL, QString *sErr = nullptr);
    static QString     f_GetQueryAsJsonStr(QString sSQL, QString *sErr = nullptr);

    //Get the col num or row num of the result
    static int f_GetColCount(QJsonObject oJsonResult);
    static int f_GetColCount(QString     sJsonResult);
    static int f_GetRowCount(QJsonObject oJsonResult);
    static int f_GetRowCount(QString     sJsonResult);

    //Save to database by given json string
    static bool f_Save(QString sModified, QString *sErr = nullptr);
    static bool f_SaveAll(QString sModified, QString *sErr = nullptr);

    //Return the fields list by a given table name.
    static QStringList f_GetFieldsListByTableName(QString sTableName, QString *sErr = nullptr);

    //Return the cols list by a given sql statement.
    static QStringList f_GetColsListBySql(QString sSQL, QString *sErr = nullptr);

    static void f_CloseDB();

private:
    /* Cannot use pointer such as 'QMutex *m_Mutex;',
     * or the code 'QMutexLocker...' will be unused.
     * Right way:
     *     QMutex m_Mutex;
     *     QMutexLocker locker(&m_Mutex);
     * Wrong way:
     *     QMutex *m_Mutex;
     *     QMutexLocker locker(m_Mutex);
     */
    static QMutex m_Mutex;

    static QSqlDatabase m_db;

    //Get connection info from ini file
    static bool f_GetConnectInfo(QString *sDB_Type, QString *sHost, QString *sPort,
                          QString *sDB_Name, QString *sUser, QString *sPwd,
                          QString *sErr = nullptr);

    //open or close database
    static bool f_OpenDB(QSqlDatabase *db, QString *sErr = nullptr);
    static bool f_OpenDB(QString *sErr);
    static void f_CloseDB(QSqlDatabase *db);
    static void f_DB_FileFree(QString sFile);

    //Get the db connectionName string.
    static QString f_GetConnectionName();

    //Open specified type of database
    static bool f_OpenDB_Mysql  (QString sHost, QString sPort, QString sDB_Name,
                          QString sUser, QString sPwd, QSqlDatabase *db, QString *sErr = nullptr);
    static bool f_OpenDB_SqlLite(QString sDB_Name, QSqlDatabase *db, QString *sErr = nullptr);
    static bool f_OpenDB_Access (QString sDB_Name, QSqlDatabase *db, QString *sErr = nullptr);

    //Return the fields list of by a given query object.
    static QStringList f_GetColsListByQuery(QSqlQuery *query, QString *sErr = nullptr);

    //Convert query result to json
    static QJsonObject f_ConvertQueryToJsonObj(QSqlQuery *query, QString *sErr = nullptr);

    //Used for function 'f_Save'.
    static bool f_GetSqlListByModify(QString sModified, QStringList *sListSQL, QString *sErr = nullptr);
};

#endif // HQ_BASE_DB_H
