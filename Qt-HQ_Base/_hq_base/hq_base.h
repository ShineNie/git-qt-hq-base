/************************************************************************
 *  Class name:     HQ_Base
 *  Author:         NieXin
 *  Created date:   2022-5-18
 *  Used for:       base operation
 ************************************************************************/
#ifndef HQ_BASE_H
#define HQ_BASE_H

#include "qt-hq_base_global.h"
class QJsonObject;
class QJsonArray;
class QString;
class QStringList;

class QTHQ_BASESHARED_EXPORT HQ_Base
{
public:
    HQ_Base();

    //About ini file.
    //The file path is at the first of 'hq_base.cpp'.
    static QJsonObject f_INI_GetValue(QString sFile, QStringList sKeyList, QString *sErr = nullptr);
    static QJsonObject f_INI_GetValue(               QStringList sKeyList, QString *sErr = nullptr);
    static QString     f_INI_GetValue(QString sFile, QString sKey,         QString *sErr = nullptr);
    static QString     f_INI_GetValue(               QString sKey,         QString *sErr = nullptr);
    static bool        f_INI_SetValue(QString sFile, QJsonObject oJsonKeyValuePairs, QString *sErr = nullptr);
    static bool        f_INI_SetValue(               QJsonObject oJsonKeyValuePairs, QString *sErr = nullptr);
    static bool        f_INI_SetValue(QString sFile, QString sKey, QString sValue, QString *sErr = nullptr);
    static bool        f_INI_SetValue(               QString sKey, QString sValue, QString *sErr = nullptr);

    //About json
    static QJsonObject f_Json_StringToJsonObj(const QString& str, QString *sErr = nullptr);
    static QJsonArray  f_Json_StringToJsonArr(const QString& str, QString *sErr = nullptr);
    static QString     f_Json_ToString(QJsonObject oJson, QString *sErr = nullptr);
    static QString     f_Json_ToString(QJsonArray oArr, QString *sErr = nullptr);

    //Pointer operation.
    static QString f_Pointer_GetValue(QString *s);
    static void    f_Pointer_SetValue(QString     *s, QString     sValue);
    static void    f_Pointer_SetValue(QStringList *s, QStringList sList);
    static void    f_Pointer_SetValue(bool        *b, bool        bValue);

    //About GUID.
    //Return the common format of the given guid
    static QString f_GUID_GetCommon(QString sGUID);
};

#endif // HQ_BASE_H
