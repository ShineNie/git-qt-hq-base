#include "hq_base_modbus.h"
#include <QRegularExpression>
#include <QStandardItem>

Hq_Base_ModBus::Hq_Base_ModBus()
{

}
/*================================================================
 * USED FOR:
 *
 *    Return a command list by given received data package string.
 *
 * FORMAT:
 *
 *    0000 0000 **** ** | ** **** ****
 *
 * Header: 8 bits, ByteCount: 4 bits, Unit address: 2 bits,
 * Function Code: 2 bits, Register address: 4 bits, ...
 *
 * BIT:
 *
 *    If it is not specified, the 'bit' means hex bit.
 *
 * LENGTH:
 *
 *    The length of bits below, mostly means the count of hex bits.
 * if it need to be used for byte bits, there'll be the key
 * word 'Byte' in its name string.
 *
 * SpaceFormat:
 *
 *    The command is a string removed all the space and invalid char.
 * If the function returns a valid command without space, it will not
 * be human-readable easily. So the argument named 'bHumanReadable'
 * changes the command to a human-readable string with space. Such as
 * below:
 *
 *    command : 000000000006010602000001
 *    string : 00000000 0006 01 06 0200 0001
 *
 * Argument '*CmdItemList':
 *
 *    Used to return the command list as a item list. Every item is
 * a command. It stores the command parts, every part can be got by
 * given item field, such as: header, unit, funcCode, values.
 *
 *===============================================================*/
QStringList Hq_Base_ModBus::f_PackagesToCmdList(QString sPackages, bool bHumanReadable)
{
    QList<QStandardItem*> listItem = f_PackagesToCmdListAsItem(sPackages, bHumanReadable);
    QStringList listString;
    for (int i = 0; i < listItem.size(); i++)
    {
        listString << listItem.at(i)->text();
    }
    return listString;
}
QStringList Hq_Base_ModBus::f_PackagesToCmdList(QByteArray arr, bool bHumanReadable)
{
    QString s = f_ByteArrayToHexStr(arr);
    return f_PackagesToCmdList(s, bHumanReadable);
}
QList<QStandardItem*> Hq_Base_ModBus::f_PackagesToCmdListAsItem(QString sPackages,
                                                                bool bHumanReadable)
{
    QList<QStandardItem*> listCmdItem;
    QString sPackage = sPackages.replace(" ","")//remove space char
            .toUpper()//convert to upper case
            .replace(QRegularExpression("[^0-9A-F]"), "");//remove invlid char

    /* The package must to contains these bits at least: header(8 bits),
     * count of bytes(4 bits), address of unit(2 bits). 14 bits in total.
     * Otherwise the package is the last invalid tail, it'll be discarded.
     *
     *      0000 0000 0000 00 | ...
     *      ---- ---- ---- --
     */
    if (sPackage.length() < 14)
    {
        return listCmdItem;
    }

    //Check the header.
    // 0000 0000 **** ** | ...
    // ---- ----
    QString sCurHeader;
    if (!f_IsOK_Header(sPackage, &sCurHeader))
    {
        return f_PackagesToCmdListAsItem(sPackage.mid(1));
    }

    //Check the data length.
    // 0000 0000 **** ** | ...
    //           ----
    //If it returns true, the package length is surely enough.
    QString sCurData_ByteCount;
    int iCurData_Length = 0;
    int iCurPackage_Length = 0;
    if (!f_IsOK_Length(sPackage, &sCurData_ByteCount, &iCurData_Length, &iCurPackage_Length))
    {
        return f_PackagesToCmdListAsItem(sPackage.mid(1));
    }

    // 0000 0000 **** ** | ** ...
    //                        ---
    QString sAfterFuncCode;

    // 0000 0000 **** ** | ** ...
    //                     --
    QString sFuncCode = sPackage.mid(14, 2);

    QStringList sValueList;
    if (sFuncCode == "03")
    {
        if (!f_IsOK_Func03(sPackage, iCurData_Length, &sAfterFuncCode, &sValueList))
        {
            return f_PackagesToCmdListAsItem(sPackage.mid(1));
        }
    }
    else if (sFuncCode == "06")
    {
        if (!f_IsOK_Func06(sPackage, iCurData_Length, &sAfterFuncCode, &sValueList))
        {
            return f_PackagesToCmdListAsItem(sPackage.mid(1));
        }
    }
    else if (sFuncCode == "10")
    {
        if (!f_IsOK_Func10(sPackage, iCurData_Length, &sAfterFuncCode, &sValueList))
        {
            return f_PackagesToCmdListAsItem(sPackage.mid(1));
        }
    }
    else if (sFuncCode == "07")
    {
        /* Comfirming command is the command whose function code is '07'.
         * It means the machine has reached the position.
         *
         * The sending command replaced its function code by '07', and it
         * equals the confirming command.
         *
         * For sending cmd: At now, only the command whose function code is '06' or '10' has
         * its comfirming command.
         */
        QString s06 = sPackage.replace(14, 2, "06");
        QString s10 = sPackage.replace(14, 2, "10");
        if (!f_IsOK_Func06(s06, iCurData_Length, &sAfterFuncCode, &sValueList) &&
            !f_IsOK_Func10_Send(s10, iCurData_Length, &sAfterFuncCode, &sValueList))
        {
            return f_PackagesToCmdListAsItem(sPackage.mid(1));
        }
    }
    else if (sFuncCode == "83" || sFuncCode == "86" || sFuncCode == "90")
    {
        //exception: 0000 0000 0003 ** | 83 **
        //exception: 0000 0000 0003 ** | 90 **
        //exception: 0000 0000 0003 ** | 86 **
        //                          --   -- --
        if (iCurData_Length != 6)
        {
            return f_PackagesToCmdListAsItem(sPackage.mid(1));
        }

        //Other case of '83/86/90' is OK.
        //exception: 0000 0000 0003 ** | 86 **
        //                                  --
        //                                  |
        //                                  16
        sAfterFuncCode = sPackage.mid(16, 2);
    }
    else
    {
        return f_PackagesToCmdListAsItem(sPackage.mid(1));
    }

    //   00000000 **** ** | **  ...
    //                 --   --  ---
    QString sCurData = sPackage.mid(12, iCurData_Length);

    //   00000000 **** ** | **  ...
    //                 ==   --  ---
    QString sUnit = sCurData.left(2);

    //   00000000 **** ** | **  ...
    //   -------- ---- --   --  ---
    //      |      |   |    |    |
    // sCurHeader  | sUnit sFunc |
    //             |             |
    //    sCurData_ByteCount   sAfterFuncCode
    QString sCurPackage = sCurHeader + " " + sCurData_ByteCount + " "
            + sUnit + " " + sFuncCode + " " + sAfterFuncCode;

    //If it is not set to human-readable, remove the space.
    if (!bHumanReadable)
    {
        sCurPackage = sCurPackage.replace(" ", "");
    }

    QString     sNextOthers = sPackage.mid(iCurPackage_Length);

    //Using the rule 'item_ruler_modbus' in file 'hq_base_modbus.h'.
    QStandardItem *item = new QStandardItem(sCurPackage);
    item->setData(QVariant::fromValue(sCurHeader), Qt::UserRole + eCmdPart_Header);
    item->setData(QVariant::fromValue(sUnit),      Qt::UserRole + eCmdPart_Unit);
    item->setData(QVariant::fromValue(sFuncCode),  Qt::UserRole + eCmdPart_FuncCode);
    item->setData(QVariant::fromValue(sValueList), Qt::UserRole + eCmdPart_Values);

    listCmdItem << item << f_PackagesToCmdListAsItem(sNextOthers);
    return listCmdItem;
}
/*===============================================================
 * IsHeaderOK.
 *
 * Return true on successed, otherwise return false.
 * The argument '*sHeader' can output the header string.
 * Such as below:
 *
 *    0000 0000 **** ** | ...
 *    ---- ----
 *===============================================================*/
bool Hq_Base_ModBus::f_IsOK_Header(QString sPackage, QString *sHeader)
{
    if (sPackage.isEmpty() || sPackage.length() < 8)
    {
        return false;
    }

    /* The header must be 8 zeros if there is no transcation code in it.
     * Otherwise, the header will be the transcation code(4 bits) and the
     * protocol number(4 zeros).
     *
     * The receiving cmd's transcation code equals the sending cmd's.
     *
     * without the transcation code:
     * 0000 0000 **** ** | ...
     * ---- ----
     *
     * with the transcation code:
     * **** 0000 **** ** | ...
     * ---- ====
     */
    *sHeader = sPackage.left(8);
    //if (*sHeader != "00000000")//it is valid
    if (sHeader->mid(4, 4) != "0000")//it is valid
    {
        return false;
    }

    return true;
}
/*===============================================================
 * IsLengthOK.
 *
 * Return true on successed, otherwise return false. If it returns
 * true, the package length is surely enough.
 *
 * The argument '*sCurData_ByteCount' can output the data length as
 * string. Such as below:
 *
 *      0000 0000 **** ** | ...
 *                ----
 *
 * The argument '*iCurData_Length' can output the data length as int.
 *
 *      iLength = sByteCount.toInt() * 2;
 *
 * The argument '*iCurPackage_Length' can output the package length
 * as int.
 *===============================================================*/
bool Hq_Base_ModBus::f_IsOK_Length(QString sPackage, QString *sCurData_ByteCount,
                                  int *iCurData_Length, int *iCurPackage_Length)
{
    // 0000 0000 **** ** | ...
    // ---- ---- ----
    if (sPackage.isEmpty() || sPackage.length() < 12)
    {
        return false;
    }

    bool bStrToIntOK;//used for judging converting to int successed or failed.
    int iTemp;//used for converting to int.

    // Get the data length(count of bytes of data)
    // 0000 0000 **** ** | ...
    //           ----
    *sCurData_ByteCount = sPackage.mid(8, 4);//count of data bytes
    iTemp = sCurData_ByteCount->toInt(&bStrToIntOK, 16);
    if (!bStrToIntOK)//if can not convert to int
    {
        return false;
    }
    *iCurData_Length = iTemp * 2;//bytes * 2 = bits

    // Get the length of command string.
    // 0000 0000 **** ** | 83 **
    // ==== ==== ==== --   -- --
    *iCurPackage_Length = *iCurData_Length + 12;

    //if the length is no enough to became a package
    if (sPackage.length() < *iCurPackage_Length)
    {
        return false;
    }

    return true;
}
/*===============================================================
 * Check by function code.
 *
 * The package length was checked befort it. So the package length
 * is surely enough.
 *
 * The pointer argument can output the specified value.
 *===============================================================*/
bool Hq_Base_ModBus::f_IsOK_Func03(QString sPackage, int iCurData_Length,
                                   QString *sAfterFuncCode, QStringList *sValueList)
{
    if (f_IsOK_Func03_Send(sPackage, iCurData_Length, sAfterFuncCode))
    {
        return true;
    }
    else if (f_IsOK_Func03_Receive(sPackage, iCurData_Length, sAfterFuncCode, sValueList))
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool Hq_Base_ModBus::f_IsOK_Func03_Send(QString sPackage, int iCurData_Length,
                                        QString *sAfterFuncCode)
{
    //   send: 0000 0000 0006 ** | 03 **** ****
    //         ---- ---- ---- --   -- ---- ----
    if (sPackage.isEmpty() || sPackage.length() < 24)
    {
        return false;
    }

    QString sAfterFunc;
    if (iCurData_Length == 12)
    {
        //   send: 0000 0000 0006 ** | 03 **** ****
        //                        --   -- ---- ----

        //   send: 0000 0000 0006 ** | 03 **** ****
        //                                ---------
        //                                |
        //                                16
        sAfterFunc = sPackage.mid(16, 8).insert(4, " ");

        //It's OK, do nothing here.
    }
    else
    {
        return false;
    }

    *sAfterFuncCode = sAfterFunc;
    return true;
}
bool Hq_Base_ModBus::f_IsOK_Func03_Receive(QString sPackage, int iCurData_Length,
                                   QString *sAfterFuncCode, QStringList *sValueList)
{
    //receive: 0000 0000 **** ** | 03 ** (****, ...)
    //         ---- ---- ---- --   -- --  ----
    if (sPackage.isEmpty() || sPackage.length() < 22)
    {
        return false;
    }

    bool bStrToIntOK;//used for judging converting to int successed or failed.
    int iTemp;//used for converting to int.
    QString sAfterFunc;
    QStringList sValueList_tmp;
    if (iCurData_Length == 12)
    {
        //This is send format.
        return false;
    }
    else
    {
        //receive: 0000 0000 **** ** | 03 ** (****, ...)
        //                        --   -- --  ----
        if (iCurData_Length < 10)
        {
            return false;
        }

        //receive: 0000 0000 **** ** | 03 ** (****, ...)
        //                                --
        //                                |
        //                                16
        QString sValue_ByteCount = sPackage.mid(16, 2);//byte count of value
        iTemp = sValue_ByteCount.toInt(&bStrToIntOK, 16);
        if (!bStrToIntOK)//if can not convert to int
        {
            return false;
        }
        int iValue_Length = iTemp * 2;//bits = bytes * 2

        //receive: 0000 0000 **** ** | 03 ** (****, ...)
        //                        ==   == ==  ----  ---
        if (iCurData_Length != (iValue_Length + 6))
        {
            return false;
        }

        //Other case below of '03' is OK.
        //receive: 0000 0000 **** ** | 03 ** (****, ...)
        //                                --
        sAfterFunc = sValue_ByteCount;

        //receive: 0000 0000 **** ** | 03 ** (****, ...)
        //                                --- ---- ----
        for (int i = 0; i < iValue_Length; i += 4)
        {
            //receive: 0000 0000 **** ** | 03 ** (****, ...)
            //                                    ----  ---
            //                                    |
            //                                    18
            QString sValue = sPackage.mid(18 + i, 4);
            sAfterFunc += " " + sValue;
            sValueList_tmp << sValue;
        }
    }

    *sAfterFuncCode = sAfterFunc;
    *sValueList = sValueList_tmp;
    return true;
}
bool Hq_Base_ModBus::f_IsOK_Func06(QString sPackage, int iCurData_Length,
                                   QString *sAfterFuncCode, QStringList *sValueList)
{
    //send or receive: 0000 0000 0006 ** | 06 **** ****
    //                                -- | -- ---- ----
    if (sPackage.isEmpty() || sPackage.length() < 24)
    {
        return false;
    }

    //send or receive: 0000 0000 0006 ** | 06 **** ****
    //                                -- | -- ---- ----
    if (iCurData_Length != 12)
    {
        return false;
    }

    //Other case below of '06' is OK.
    //   send: 0000 0000 0006 ** | 06 **** ****
    //                                ---- ----
    //                                |
    //                                16
    QString sAfter = sPackage.mid(16, 8).insert(4, " ");

    //   send: 0000 0000 0006 ** | 06 **** ****
    //                                     ----
    //                                     |
    //                                     20
    QString sValue = sPackage.mid(20, 4);

    *sAfterFuncCode = sAfter;
    *sValueList << sValue;
    return true;
}
bool Hq_Base_ModBus::f_IsOK_Func10(QString sPackage, int iCurData_Length,
                                   QString *sAfterFuncCode, QStringList *sValueList)
{
    if (f_IsOK_Func10_Send(sPackage, iCurData_Length, sAfterFuncCode, sValueList))
    {
        return true;
    }
    else if (f_IsOK_Func10_Receive(sPackage, iCurData_Length, sAfterFuncCode))
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool Hq_Base_ModBus::f_IsOK_Func10_Send(QString sPackage, int iCurData_Length,
                                   QString *sAfterFuncCode, QStringList *sValueList)
{
    //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
    //                        --   -- ---- ---- --
    if (sPackage.isEmpty() || sPackage.length() < 26)
    {
        return false;
    }

    bool bStrToIntOK;//used for judging converting to int successed or failed.
    int iTemp;//used for converting to int.
    QString sAfterFunc;
    QStringList sValueList_tmp;
    if (iCurData_Length == 12)
    {
        //This is receive format.
        return false;
    }
    else
    {
        //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
        //                        --   -- ---- ---- --
        if (iCurData_Length < 14)
        {
            return false;
        }

        //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
        //                                     ----
        //                                     |
        //              register(value) count: 20
        QString sValue_Count = sPackage.mid(20, 4);
        iTemp = sValue_Count.toInt(&bStrToIntOK, 16);
        if (!bStrToIntOK)//if can not convert to int
        {
            return false;
        }
        int iValue_Count = iTemp;//Perhaps it equals 0.

        //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
        //                                          --
        //                                          |
        //                                          24
        QString sValue_ByteCount = sPackage.mid(24, 2);
        iTemp = sValue_ByteCount.toInt(&bStrToIntOK, 16);
        if (!bStrToIntOK)//if can not convert to int
        {
            return false;
        }
        int iValue_Length = iTemp * 2;//bits = bytes * 2

        //length of every value is 4.
        if ((iValue_Count * 4) != iValue_Length)
        {
            return false;
        }

        //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
        //                        ==   == ==== ==== ==  ----  ---
        if (iCurData_Length != (iValue_Length + 14))
        {
            return false;
        }

        //It is OK below.
        //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
        //                                ---- ---- --
        //                                |    |    |
        //                                16   20   24
        sAfterFunc = sPackage.mid(16, 4) + " " + sPackage.mid(20, 4) + " " + sPackage.mid(24, 2);

        //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
        //                                ---- ---- --  ---- ----
        //                                              |
        //                                              26
        for (int i = 0; i < iValue_Length; i += 4)
        {
            QString sValue = sPackage.mid(26 + i, 4);
            sAfterFunc += " " + sValue;
            sValueList_tmp << sValue;
        }
    }

    *sAfterFuncCode = sAfterFunc;
    *sValueList = sValueList_tmp;
    return true;
}
bool Hq_Base_ModBus::f_IsOK_Func10_Receive(QString sPackage, int iCurData_Length,
                                           QString *sAfterFuncCode)
{
    //receive: 0000 0000 0006 ** | 10 **** ****
    //                        --   -- ---- ----
    if (sPackage.isEmpty() || sPackage.length() < 24)
    {
        return false;
    }

    QString sAfterFunc;
    if (iCurData_Length == 12)
    {
        //receive: 0000 0000 0006 ** | 10 **** ****
        //                        --   -- ---- ----
        //It's OK, do nothing here.

        //receive: 0000 0000 0006 ** | 10 **** ****
        //                                ---------
        //                                |
        //                                16
        sAfterFunc = sPackage.mid(16, 8).insert(4, " ");
    }
    else
    {
        return false;
    }

    *sAfterFuncCode = sAfterFunc;
    return true;
}
/*===============================================================
 * Convert string to hex bytearray.
 *===============================================================*/
QByteArray Hq_Base_ModBus::f_HexStrToByteArray(QString sHexStr)
{
    QString sPackage = sHexStr.replace(" ","")//remove space char
            .toUpper()//convert to upper case
            .replace(QRegularExpression("[^0-9A-F]"), "");//remove invlid char

    int len = sPackage.length();
    if (len % 2 != 0)
    {
        sPackage = QString("0") + sPackage;
    }
    QByteArray arr;
    arr.resize(len / 2);
    for (int i = 0; i < len; i += 2)
    {
        QString sHigh = QString(sPackage.at(i));
        QString sLow  = QString(sPackage.at(i+1));
        quint16 iHigh = sHigh.toInt(nullptr, 16);
        quint16 iLow  = sLow.toInt(nullptr, 16);
        arr[i/2] = ((iHigh & 0xF) << 4) | (iLow & 0xF);
    }
    return arr;
}
/*===============================================================
 * Convert bytearray to string.
 *===============================================================*/
QString Hq_Base_ModBus::f_ByteArrayToHexStr(QByteArray arr)
{
    QByteArray arrStr;
    arrStr.resize(arr.size() * 2);
    for (int i = 0; i < arr.size(); i++)
    {
        quint16 iHigh = quint16(arr[i] & 0xF0) >> 4;
        quint16 iLow  = quint16(arr[i] & 0x0F);
        QString sHigh = QString::number(iHigh, 16).toUpper();
        QString sLow  = QString::number(iLow,  16).toUpper();
        arrStr[i * 2]     = (sHigh.toUtf8())[0];
        arrStr[i * 2 + 1] = (sLow.toUtf8())[0];
    }
    return arrStr;
}
/*===============================================================
 * Set the new function code. Return the new cmd.
 *
 *  0000 0000 **** ** | ** ...
 *                      --
 *===============================================================*/
QString Hq_Base_ModBus::f_ReplaceFuncCode(QString sCmd, QString sNewFuncCode, bool bHumanReadable)
{
    //If it is invalid format.
    QStringList sList = f_PackagesToCmdList(sCmd, false);//must ot be no space
    if (sList.isEmpty())
    {
        return "";
    }

    QString sCmd_Tmp = sList.at(0);

    //Replace the function code with the new one.
    // 0000 0000 **** ** | ** ...
    //                     --
    //                     |
    //                     14
    sCmd_Tmp = sCmd_Tmp.replace(14, 2, sNewFuncCode);

    //If the new command is invalid format.
    sList = f_PackagesToCmdList(sCmd_Tmp, bHumanReadable);
    if (sList.isEmpty())
    {
        return "";
    }

    return sList.at(0);
}
/**===============================================================
 ** Set the transcation code. Return the new cmd.
 *
 *  0000 0000 **** ** | ** ...
 *  ----
 **===============================================================*/
QString Hq_Base_ModBus::f_SetTransCode(QString sCmd, QString sTransCode, bool bHumanReadable)
{
    if (sTransCode.length() != 4)
    {
        return "";
    }

    //If it is invalid format.
    QStringList sList = f_PackagesToCmdList(sCmd, false);//must ot be no space
    if (sList.isEmpty())
    {
        return "";
    }

    QString sCmd_Tmp = sList.at(0);

    //Replace the function code with the new one.
    // 0000 0000 **** ** | ** ...
    // ----
    sCmd_Tmp = sCmd_Tmp.replace(0, 4, sTransCode);

    //If the new command is invalid format.
    sList = f_PackagesToCmdList(sCmd_Tmp, bHumanReadable);
    if (sList.isEmpty())
    {
        return "";
    }

    return sList.at(0);
}
/*===============================================================
 * Return the function code by given command string.
 * Return "" on error.
 *===============================================================*/
QString Hq_Base_ModBus::f_GetFuncCode(QString sCommand)
{
    //If it is invalid format.
    QStringList sList = f_PackagesToCmdList(sCommand, false);
    if (sList.isEmpty())
    {
        return "";
    }

    QString sCmd = sList.at(0);

    // 0000 0000 **** ** | ** ...
    //                     --
    QString sFuncCode = sCmd.mid(14, 2);

    return sFuncCode;
}
/*===============================================================
 * Return the CRC.
 *
 * The function 'f_ModbusCRC16' was download from the internet.
 * About the argument 'bHighBitLeft':
 *
 *         By the default, high bit is on left, the low bit is on
 *     right. The position of high bit and low bit can be exchanged
 *     to each other if this argument was given to false.
 *===============================================================*/
QString Hq_Base_ModBus::f_GetCRC(QString sCmd, bool bHighBitLeft)
{
    QByteArray ba = f_HexStrToByteArray(sCmd);
    QString sCRC = QString::number(f_ModbusCRC16(ba), 16).toUpper();

    //High bit and low bit exchange the position.
    if (!bHighBitLeft)
    {
        QString sHigh = sCRC.mid(0,2);
        QString sLow = sCRC.mid(2,2);
        sCRC = sLow + sHigh;
    }

    return sCRC;
}
uint16_t Hq_Base_ModBus::f_ModbusCRC16(QByteArray senddata)
{
    int len=senddata.size();
    uint16_t wcrc=0XFFFF;//预置16位crc寄存器，初值全部为1
    uint8_t temp;//定义中间变量
    int i=0,j=0;//定义计数
    for(i=0;i<len;i++)//循环计算每个数据
    {
       temp=senddata.at(i);
       wcrc^=temp;
       for(j=0;j<8;j++){
          //判断右移出的是不是1，如果是1则与多项式进行异或。
          if(wcrc&0X0001){
              wcrc>>=1;//先将数据右移一位
              wcrc^=0XA001;//与上面的多项式进行异或
          }
          else//如果不是1，则直接移出
              wcrc>>=1;//直接移出
       }
    }
    temp=wcrc;//crc的值
    return wcrc;
}
/*===============================================================
 * Determine whether the machine receives the command.
 * ------------------
 * Before calling this function, the format of inputting commands
 * is surely valid. So in the code at below, it is not necessary
 * to determine all the rule-details of modbus.
 *===============================================================*/
bool Hq_Base_ModBus::f_IsCmdReceived(QString sCmd_Send, QString sCmd_Recv)
{
    QString s1 = sCmd_Send.replace(" ","").toUpper();
    QString s2 = sCmd_Recv.replace(" ","").toUpper();

    //   send: 0000 0000 0006 ** | 03 **** ****
    //receive: 0000 0000 **** ** | 03 ** (****, ...)
    //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
    //receive: 0000 0000 0006 ** | 10 **** ****
    //         ---- ---- ==== --   --
    //                   |
    //                   8
    if (s1.left(16).replace(8, 4, "") != s2.left(16).replace(8, 4, ""))
    {
        return false;
    }
    QString sFuncCode = f_GetFuncCode(sCmd_Send);
    if (sFuncCode == "06")
    {
        return s1 == s2;
    }
    else if(sFuncCode == "03")
    {
        //   send: 0000 0000 0006 ** | 03 **** ****
        //                                     ----
        //                                     |
        //                                     20
        QString sValue_Count = sCmd_Send.mid(20, 4);
        int iValue_Count = sValue_Count.toInt(nullptr, 16);

        //receive: 0000 0000 **** ** | 03 ** (****, ...)
        //                                --
        //                                |
        //                                16
        QString SValue_ByteCount = sCmd_Recv.mid(16, 2);
        int iValue_ByteCount = SValue_ByteCount.toInt(nullptr, 16);
        return (iValue_Count * 2) == iValue_ByteCount;//every value = 2 bytes
    }
    else if(sFuncCode == "10")
    {
        //   send: 0000 0000 **** ** | 10 **** **** ** (****, ...)
        //receive: 0000 0000 0006 ** | 10 **** **** |
        //         ---- ---- ---- --   -- ---- ---- |
        //                   |                      |
        //                   8                      24
        return s1.left(24).replace(8, 4, "0006") == s2;
    }
    else
    {
        return false;
    }
}
/*===============================================================
 * Determine whether the command has completed.
 *===============================================================*/
bool Hq_Base_ModBus::f_IsCmdCompleted(QString sCmd_Send, QString sCmd_Recv)
{
    QString s1 = sCmd_Send.replace(" ","").toUpper();
    QString s2 = sCmd_Recv.replace(" ","").toUpper();
    s1 = f_ReplaceFuncCode(s1, "07", false);
    return s1.left(16) == s2.left(16);
}
