#include "hq_base_ui.h"
#include <QWidget>
#include <QJsonObject>

HQ_Base_Ui::HQ_Base_Ui()
{

}
/*===============================================================
 * Get the enabled state of the children widget of a given parent
 * widget. Return it as a json object to remember the enabled state.
 *
 * Format:
 * {
 *      "w1": bool_value,
 *      "w2": bool_value,
 *      ...
 * }
 *
 * Example:
 *
 *   //Remember the enabled state of target ui.
 *   m_oJsonTargetUiEnbled = HQ_Base_Ui::f_GetChildrenEnable(m_tabConfig);
 *
 *===============================================================*/
QJsonObject HQ_Base_Ui::f_GetChildrenEnable(QWidget *wParent)
{
    QJsonObject oJson;
    if (nullptr == wParent)
    {
        return oJson;
    }
    try
    {
        QList<QWidget *> listWidgets = wParent->findChildren<QWidget *>();
        for (QWidget *wChild : listWidgets)
        {
            oJson.insert(wChild->objectName(), wChild->isEnabled());
        }
    }
    catch(...)
    {
    }
    return oJson;
}
/*===============================================================
 * Set the enable state of the children widget of a given
 * parent widget.
 *
 * The given json object stores the status of these children
 * widget  at the last time.
 *
 * If the given json object is empty, or the value of the key
 * that equals the widget's name is empty, the enabled state of
 * the widget will be set to given bool value. The given bool
 * value is true by default.
 *
 * If you want to set the enable state to a specified value, you
 * should defined a empty json object variable without any given
 * value at first, and give the empty json object to the function
 * as a argument, such as below:
 *
 *   //Set the enabled state of target ui to disabled.
 *   QJsonObject oJson;
 *   HQ_Base_Ui::f_SetChildrenEnable(m_tabConfig, oJson, false);
 *
 * The Other case, if a json object who remembered the last time
 * enable  state was given as a argument. The function will restore
 * the  enable state to the last time state. Such as below:
 *
 *   //Restores the enabled state of target ui to remembered state
 *   //at the last time.
 *   HQ_Base_Ui::f_SetChildrenEnable(m_tabConfig, m_oJsonTargetUiEnbled);
 *
 *===============================================================*/
void HQ_Base_Ui::f_SetChildrenEnable(QWidget *wParent, QJsonObject oJson, bool b)
{
    if (nullptr == wParent)
    {
        return;
    }
    try
    {
        QList<QWidget *> listWidgets = wParent->findChildren<QWidget *>();
        for (QWidget *wChild : listWidgets)
        {
            QString sChildName = wChild->objectName();
            if (sChildName.isEmpty())
            {
                continue;
            }

            if (oJson.isEmpty())
            {
                wChild->setEnabled(b);
            }
            else
            {
                bool bBefore = oJson.value(sChildName).toBool();
                wChild->setEnabled(bBefore);
            }
        }
    }
    catch(...)
    {}
}
void HQ_Base_Ui::f_SetChildrenEnable(QWidget *wParent, bool b)
{
    f_SetChildrenEnable(wParent, QJsonObject(), b);
}
