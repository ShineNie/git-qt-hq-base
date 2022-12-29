#ifndef FRM_PAGE_H
#define FRM_PAGE_H

#include <QWidget>

namespace Ui {
class frm_Page;
}

class frm_Page : public QWidget
{
    Q_OBJECT

public:
    explicit frm_Page(QWidget *parent = 0);
    ~frm_Page();

private slots:
    void onQuery(QString sJsonResult);

    void on_btnRefresh_clicked();

private:
    Ui::frm_Page *ui;

    void f_Init();
    void f_Load();
};

extern frm_Page *g_wPage;

#endif // FRM_PAGE_H
