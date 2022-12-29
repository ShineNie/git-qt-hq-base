#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class frm_DbGrid_Double;
class frm_DbGrid_Single;
class frmCommand;
class frmGetPackages;
class frmEncrypt;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_aExit_triggered();

    void on_aGridsingle_triggered();

    void on_aGriddouble_triggered();

    void on_aModbusFuncCode_triggered();

    void on_aModbusPackages_triggered();

    void on_aEncryptAES_triggered();

    void on_aPage_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
