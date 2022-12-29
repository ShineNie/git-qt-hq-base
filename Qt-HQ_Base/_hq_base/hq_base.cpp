#include "hq_base.h"
#include <QFile>
#include <QSettings>//for read or write ini files.
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

QString g_sINI_File = "config.ini";

HQ_Base::HQ_Base()
{

}
/*===============================================================
 * Return the value as json object by given key list of a ini file.
 * Return empty on error, and the argument 'sErr' stores err message,
 * if it isn't a null pointer.
 * The argument 'key' maybe 'section/key'.
 * Format:
 *
 * {
 *      "Key": "value",
 *      "Key": "value",
 *      ...
 * }
 *===============================================================*/
QJsonObject HQ_Base::f_INI_GetValue(QString sFile, QStringList sKeyList, QString *sErr)
{
    QSettings *ini = nullptr;
    QString sFileName;
    QJsonObject oJson;
    QString sErrPrefix = "Ini file reading failed.";

    try
    {
        if (sKeyList.isEmpty())
        {
            f_Pointer_SetValue(sErr, sErrPrefix + " For given key list is empty.");
            return oJson;
        }

        ini = new QSettings(sFile, QSettings::IniFormat);
        sFileName = ini->fileName();
        QFile f(sFileName);
        if (!f.exists())
        {
            delete ini;
            f_Pointer_SetValue(sErr, sErrPrefix
                         + QString(" For file '%1' is not exists.").arg(sFileName));
            return oJson;
        }

        for (QString sKey : sKeyList)
        {
            QString sValue = ini->value(sKey, "").toString();
            oJson.insert(sKey, sValue);
        }

        delete ini;
        f_Pointer_SetValue(sErr, "");
    }
    catch (...)
    {
        if (ini != nullptr)
        {
            delete ini;
        }
        f_Pointer_SetValue(sErr, sErrPrefix
                     + QString(" For unknown err on '%1'.").arg(sFileName));
    }
    return oJson;
}
QJsonObject HQ_Base::f_INI_GetValue(QStringList sKeyList, QString *sErr)
{
    return f_INI_GetValue(g_sINI_File, sKeyList, sErr);
}
/*===============================================================
 * Return the value as string by given key of ini.
 * It is a override function.
 *===============================================================*/
QString HQ_Base::f_INI_GetValue(QString sFile, QString sKey, QString *sErr)
{
    QStringList sKeyList = {sKey};
    QJsonObject oJson = f_INI_GetValue(sFile, sKeyList, sErr);
    if (!oJson.isEmpty())
    {
        return oJson.value(sKey).toString();
    }
    else
    {
        return "";
    }
}
QString HQ_Base::f_INI_GetValue(QString sKey, QString *sErr)
{
    return f_INI_GetValue(g_sINI_File, sKey, sErr);
}
/*===============================================================
 * Set the value of key in ini file by given json object of (key,
 * value) pairs.
 *
 * Return true on success, otherwise return false and err message
 * in the pointer argument 'sErr'.
 *===============================================================*/
bool HQ_Base::f_INI_SetValue(QString sFile, QJsonObject oJsonKeyValuePairs, QString *sErr)
{
    QSettings *ini = nullptr;
    QString sFileName;
    QString sErrPrefix = "Ini file writing failed.";
    QString sRestoreMsg = "Restore it by the bak file in the same path.";

    try
    {
        if (oJsonKeyValuePairs.isEmpty())
        {
            QString sMsg = sErrPrefix
                    + QString(" For given json (key,value) pairs is empty.");
            throw sMsg;
        }

        QFile f(sFile);

        //Check whether the file exists. If it is not exist, create it.
        if (!f.exists())
        {
            if (f.open(QIODevice::WriteOnly|QIODevice::Text))
            {
                qint64 i = f.write(sFile.toStdString().c_str());
                f.close();
                if (i < 0)
                {
                    QString sMsg = sErrPrefix
                            + QString(" For file '%1' is not exists and create failed.").arg(sFile);
                    throw sMsg;
                }
            }
        }

        //Backup the source file. remove the older one if exists.
        QString sFile_backup = sFile.left(sFile.length() - QString(".ini").length()) + ".bak";
        QFile f_backup(sFile_backup);
        if (f_backup.exists())
        {
            f_backup.remove();
        }
        if (!f.copy(sFile_backup))
        {
            QString sMsg = sErrPrefix + QString(" For the bak file create failed.");
            throw sMsg;
        }

        //Read the lines begin with ';'.
        QString sDesc;
        if (f.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            while (!f.atEnd())
            {
                QByteArray baLine = f.readLine();
                if (baLine.size() > 0 && baLine.at(0) == ';')
                {
                    sDesc.append(baLine);
                }
            }
            f.close();
        }
        else
        {
            QString sMsg = sErrPrefix + QString(" For the ini file read failed.");
            throw sMsg;
        }

        //Set value to the source ini file.
        ini = new QSettings(sFile, QSettings::IniFormat);
        for (QString sKey : oJsonKeyValuePairs.keys())
        {
            QString sValue = oJsonKeyValuePairs.value(sKey).toString();
            ini->setValue(sKey, sValue);
        }
        delete ini;

        //Read configuration to string variable.
        QString sConfig;
        if (f.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            sConfig = f.readAll();
            f.close();
        }
        else
        {
            QString sMsg = sErrPrefix
                    + QString(" For the ini file read after setting failed.\n")
                    + sRestoreMsg;
            throw sMsg;
        }

        //Append the configuration to contents.
        QString sContents = sDesc.append("\n").append(sConfig);

        //Write contents to source ini file.
        if (f.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            qint64 i = f.write(sContents.toStdString().c_str());
            f.close();
            if (i < 0)
            {
                QString sMsg = sErrPrefix
                        + QString(" For the ini file write failed.\n") + sRestoreMsg;
                throw sMsg;
            }
        }
        else
        {
            QString sMsg = sErrPrefix
                    + QString(" For the ini file write failed.\n") + sRestoreMsg;
            throw sMsg;
        }

        f_Pointer_SetValue(sErr, "");
        return true;
    }
    catch (QString sMsg)
    {
        if (ini != nullptr)
        {
            delete ini;
        }
        f_Pointer_SetValue(sErr, sMsg);
        return false;
    }
    catch (...)
    {
        if (ini != nullptr)
        {
            delete ini;
        }
        f_Pointer_SetValue(sErr, sErrPrefix
                     + QString(" For unknown err on '%1'.").arg(sFileName));
        return false;
    }
}
bool HQ_Base::f_INI_SetValue(QJsonObject oJsonKeyValuePairs, QString *sErr)
{
    return f_INI_SetValue(g_sINI_File, oJsonKeyValuePairs, sErr);
}
bool HQ_Base::f_INI_SetValue(QString sFile, QString sKey, QString sValue, QString *sErr)
{
    QJsonObject oJson;
    oJson.insert(sKey, sValue);
    return f_INI_SetValue(sFile, oJson, sErr);
}
bool HQ_Base::f_INI_SetValue(QString sKey, QString sValue, QString *sErr)
{
    return f_INI_SetValue(g_sINI_File, sKey, sValue, sErr);
}
/*================================================================
 * Convert string to json object
 *===============================================================*/
QJsonObject HQ_Base::f_Json_StringToJsonObj(const QString& str, QString *sErr)
{
    QJsonObject ret;

    if (!str.isEmpty())
    {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError)
        {
            if (doc.isObject())
            {
                ret = doc.object();
                f_Pointer_SetValue(sErr, "");
            }
            else
            {
                qDebug() << __FUNCTION__ << "doc is not Object";
            }
        }
        else
        {
            f_Pointer_SetValue(sErr, "Convert string to json object failed.");
            qDebug() << __FUNCTION__ << "Error!!!";
        }
    }
    return ret;
}
/*================================================================
 * Convert string to json array
 *===============================================================*/
QJsonArray HQ_Base::f_Json_StringToJsonArr(const QString& str, QString *sErr)
{
    QJsonArray ret;

    if (!str.isEmpty())
    {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError)
        {
            if (doc.isArray())
            {
                ret = doc.array();
                f_Pointer_SetValue(sErr, "");
            }
            else
            {
                qDebug() << __FUNCTION__ << __LINE__ << "doc is not array";
            }
        }
        else
        {
            f_Pointer_SetValue(sErr, "Convert string to json array failed.");
            qDebug() << __FUNCTION__ << __LINE__ << "Error!!!";
        }
    }
    return ret;
}
/*===============================================================
 * Convert json object/array to string.
 * Return "" on error.
 *===============================================================*/
QString HQ_Base::f_Json_ToString(QJsonObject oJson, QString *sErr)
{
    try
    {
        QString s = oJson.isEmpty() ? "" : QString(QJsonDocument(oJson).toJson());
        f_Pointer_SetValue(sErr, "");
        return s;
    }
    catch (...)
    {
        f_Pointer_SetValue(sErr, "Convert json object to string failed.");
        return "";
    }
}
QString HQ_Base::f_Json_ToString(QJsonArray oArr, QString *sErr)
{
    try
    {
        QString s = oArr.isEmpty() ? "" : QString(QJsonDocument(oArr).toJson());
        f_Pointer_SetValue(sErr, "");
        return s;
    }
    catch (...)
    {
        f_Pointer_SetValue(sErr, "Convert json array to string failed.");
        return "";
    }
}
/*================================================================
 * Pointer operation.
 * Get/set value of the pointer variable.
 *===============================================================*/
QString HQ_Base::f_Pointer_GetValue(QString *s)
{
    return (nullptr == s) ? "" : *s;
}
void HQ_Base::f_Pointer_SetValue(QString *s, QString sValue)
{
    if (nullptr != s)
    {
        *s = sValue;
    }
}
void HQ_Base::f_Pointer_SetValue(QStringList *s, QStringList sList)
{
    if (nullptr != s)
    {
        *s = sList;
    }
}
void HQ_Base::f_Pointer_SetValue(bool *b, bool bValue)
{
    if (nullptr != b)
    {
        *b = bValue;
    }
}
/*===============================================================
 * Return the common format of the given GUID.
 *===============================================================*/
QString HQ_Base::f_GUID_GetCommon(QString sGUID)
{
    return sGUID.toLower().replace("{", "").replace("}", "");
}
