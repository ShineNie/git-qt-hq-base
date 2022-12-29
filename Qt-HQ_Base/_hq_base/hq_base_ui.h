#ifndef HQ_BASE_UI_H
#define HQ_BASE_UI_H

#include "qt-hq_base_global.h"
class QWidget;
class QJsonObject;

class QTHQ_BASESHARED_EXPORT HQ_Base_Ui
{
public:
    HQ_Base_Ui();

    //Get/Set the enabled proprety of the children of a given widget.
    static QJsonObject f_GetChildrenEnable(QWidget *wParent);
    static void f_SetChildrenEnable(QWidget *wParent, QJsonObject oJson, bool b = true);
    static void f_SetChildrenEnable(QWidget *wParent, bool b);
};

#endif // HQ_BASE_UI_H
