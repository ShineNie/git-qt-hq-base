#ifndef FRMCOMMAND_H
#define FRMCOMMAND_H

#include <QWidget>

namespace Ui {
class frmCommand;
}

class frmCommand : public QWidget
{
    Q_OBJECT

public:
    explicit frmCommand(QWidget *parent = 0);
    ~frmCommand();

private slots:
    void on_pushButton_clicked();

private:
    Ui::frmCommand *ui;
};

extern frmCommand *g_wCmd;

#endif // FRMCOMMAND_H
