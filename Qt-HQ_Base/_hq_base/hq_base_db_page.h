/************************************************************************
 *  Class name:     HQ_Base_DB_page
 *  Author:         NieXin
 *  Created date:   2022-8-17
 *  Used for:       Data paging
 *  Desc:
 *
 *          This class was created as a widget for data paging. It contains
 *      first, prev, next, last, row-count, page-count, page-size, page-num.
 *          As a rule, I want it to works by a common way. It means this
 *      widget uses standard sql statement to finish its work. So any type
 *      of database can be supported.
 *          But, this way is not fast enough. It was designed for the upper
 *      computer.
 *          It can generate a sql statement with beautiful format. This is
 *      my favorite.
 *
 ************************************************************************/
#ifndef HQ_BASE_DB_PAGE_H
#define HQ_BASE_DB_PAGE_H

#include "qt-hq_base_global.h"
#include <QWidget>

namespace Ui {
class HQ_Base_DB_page;
}

class QTHQ_BASESHARED_EXPORT HQ_Base_DB_page : public QWidget
{
    Q_OBJECT

public:
    explicit HQ_Base_DB_page(QWidget *parent = 0);
    ~HQ_Base_DB_page();

    void f_Init(QString sTable, QString sSort_Field = QString());

signals:
    sigRecordSet(QString sJsonResult);

private slots:
    void on_btnFirst_clicked();
    void on_btnPrev_clicked();
    void on_cbPageNum_currentIndexChanged(const QString &arg1);
    void on_btnNext_clicked();
    void on_btnLast_clicked();
    void on_spPageSize_valueChanged(const QString &arg1);

    void on_btnDebug_clicked();

private:
    Ui::HQ_Base_DB_page *ui;
    QString m_sTableName;
    QString m_sSortField;
    int m_iRowCount = 0;
    int m_iPageSize = 10;
    int m_iPageNum = 1;
    int m_iPageCount = 1;
    bool m_bIsFilling = false;
    QString m_sIndent = "     ";

    void f_Query();

    //Return a sql string with row number by given table name and sort-field.
    QString f_GetSQL_WithRowNum(QString sTable, QString sOrderBy = QString());

    //Return a sql string with page info by given table name, page size and sort-field.
    QString f_GetSQL_WithPageInfo(QString sTable, int iPageSize, QString sOrderBy = QString());

    //Return a sql string by given table info and page info.
    QString f_GetSQL_ByPageinfo(QString sTable, int iPageSize, int iPageNum, QString sOrderBy = QString());
};

#endif // HQ_BASE_DB_PAGE_H
