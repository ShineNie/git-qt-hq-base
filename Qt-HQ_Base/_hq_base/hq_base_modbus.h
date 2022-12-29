/************************************************************************
 ** Class name:     Hq_Base_ModBus
 ** Author:         NieXin
 ** Created date:   2022-6-15
 ** Used for:       Modbus commands operation
 ** Such as:
 **
 **      Check out the command list from received package.
 **
 **      Convert hex string to bytearray.
 **
 **      convert bytearray to string.
 **
 ** About items:
 **
 **         If the item object was used somewhere, the rules desc here must be
 **     modified. And the desc text is required as below:
 **
 **       //Using the rule 'item_ruler_modbus' in file 'hq_base_modbus.h'.
 **       QStandardItem **item = new QStandardItem(sCurPackage);
 **       item->setData(QVariant::fromValue(sCurHeader), Qt::UserRole + eCmdPart_Header);
 **       item->setData(QVariant::fromValue(sUnit),      Qt::UserRole + eCmdPart_Unit);
 **       item->setData(QVariant::fromValue(sFuncCode),  Qt::UserRole + eCmdPart_FuncCode);
 **       item->setData(QVariant::fromValue(sValueList), Qt::UserRole + eCmdPart_Values);
 **
 ** About value list:
 **
 **         When a string has been confirmed as a valid command. If it was
 **     returned by machine, perhaps it had a value list of the register.
 **     And the value list will be set in the field named 'eCmdPart_Values'
 **     of a item. The item list will be returned as a pointer argument by
 **     the function 'f_PackagesToCmdList'.
 **
 **         But remember it, a command having value list, that doesn't mean
 **     it is surely a command received . Because for the '06' command, the
 **     sending format equals the recieving's. So the sending command of '06'
 **     has value list too.
 **
 ** About transcation code:
 **
 **     **** 0000 **** ** | ...
 **     ----
 **
 **         The transcation code is the first 4 bits of the header. It used
 **     to determine whether the receiving cmd is the right response for the
 **     sending cmd. The receiving cmd's transcation code equals the sending
 **     cmd's.
 **
 ************************************************************************/
#ifndef HQ_BASE_MODBUS_H
#define HQ_BASE_MODBUS_H

#include "qt-hq_base_global.h"
#include <QList>
class QString;
class QStringList;
class QByteArray;
class QStandardItem;

class QTHQ_BASESHARED_EXPORT Hq_Base_ModBus
{    
public:
    Hq_Base_ModBus();

    //Using the rule 'item_ruler_modbus' in file 'hq_base_modbus.h'.
    enum ECmdPart
    {
        eCmdPart_Header = 1,
        eCmdPart_Unit,
        eCmdPart_FuncCode,
        eCmdPart_Values
    };

    //Return a command list by given data package string.
    static QStringList           f_PackagesToCmdList(QString sPackages, bool bHumanReadable = true);
    static QStringList           f_PackagesToCmdList(QByteArray arr,    bool bHumanReadable = true);
    static QList<QStandardItem*> f_PackagesToCmdListAsItem(QString sPackages, bool bHumanReadable = true);

    //Convert string to hex bytearray, or convert bytearray to string.
    static QByteArray f_HexStrToByteArray(QString sHexStr);
    static QString    f_ByteArrayToHexStr(QByteArray arr);

    //Set the new function code. Return the new cmd.
    static QString f_ReplaceFuncCode(QString sCmd, QString sNewFuncCode, bool bHumanReadable = true);

    //Set the transcation code. Return the new cmd.
    static QString f_SetTransCode(QString sCmd, QString sTransCode, bool bHumanReadable = true);

    //Return the specified info of a specified cmd string.
    static QString f_GetFuncCode(QString sCommand);
    static QString f_GetCRC(QString sCmd, bool bHighBitLeft = true);

    //Determine whether the sending cmd was received or completed.
    static bool f_IsCmdReceived(QString sCmd_Send, QString sCmd_Recv);
    static bool f_IsCmdCompleted(QString sCmd_Send, QString sCmd_Recv);

private:
    static bool f_IsOK_Header(QString sPackage, QString *sHeader);
    static bool f_IsOK_Length(QString sPackage, QString *sCurData_ByteCount,
                             int *iCurData_Length, int *iCurPackage_Length);
    //03
    static bool f_IsOK_Func03        (QString sPackage, int iCurData_Length,
                                      QString *sAfterFuncCode, QStringList *sValueList);
    static bool f_IsOK_Func03_Send   (QString sPackage, int iCurData_Length,
                                      QString *sAfterFuncCode);
    static bool f_IsOK_Func03_Receive(QString sPackage, int iCurData_Length,
                                      QString *sAfterFuncCode, QStringList *sValueList);
    //06
    static bool f_IsOK_Func06        (QString sPackage, int iCurData_Length,
                                      QString *sAfterFuncCode, QStringList *sValueList);
    //10
    static bool f_IsOK_Func10        (QString sPackage, int iCurData_Length,
                                      QString *sAfterFuncCode, QStringList *sValueList);
    static bool f_IsOK_Func10_Send   (QString sPackage, int iCurData_Length,
                                      QString *sAfterFuncCode, QStringList *sValueList);
    static bool f_IsOK_Func10_Receive(QString sPackage, int iCurData_Length,
                                      QString *sAfterFuncCode);
    //CRC
    static uint16_t f_ModbusCRC16(QByteArray senddata);
};

#endif // HQ_BASE_MODBUS_H
