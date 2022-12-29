#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "frm_dbgrid_double.h"
#include "frm_dbGrid_single.h"
#include "frm_page.h"
#include "frmcommand.h"
#include "frmgetpackages.h"
#include "frmencrypt.h"
#include "hq_base_db.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Set the color style.
    QString sStyle =
            "*{background-color:rgb(70, 70, 70);color:white;}"

            //QMenuBar,QMenu,QPushButton,QLineEdit
            "QMenuBar::item:selected,QMenu::item:selected,QPushButton:hover,QLineEdit:hover"
            "{background-color:rgb(120, 120, 120);color:white;}"

            //QTableWidget
            "QTableCornerButton::section,QHeaderView::section{background-color:rgb(70, 70, 70);color:white;}"
            "QTableWidget::item{background-color:rgb(70, 70, 70);color:white;}"
            "QTableWidget::item:selected{background-color:rgb(170, 170, 170);color:white;}"
            "QTableWidget::item:hover{background-color:rgb(120, 120, 120);color:white;}";
    setStyleSheet(sStyle);
}

MainWindow::~MainWindow()
{
    HQ_Base_DB::f_CloseDB();
    delete ui;
}

void MainWindow::on_aExit_triggered()
{
    this->close();
    this->destroy();
}

void MainWindow::on_aGridsingle_triggered()
{        
    if (g_wGridSingle == nullptr)
    {
        g_wGridSingle = new frm_DbGrid_Single;
        ui->mdiArea->addSubWindow(g_wGridSingle);
    }
    g_wGridSingle->showMaximized();
}

void MainWindow::on_aGriddouble_triggered()
{
    if (g_wGridDouble == nullptr)
    {
        g_wGridDouble = new frm_DbGrid_Double;
        ui->mdiArea->addSubWindow(g_wGridDouble);
    }
    g_wGridDouble->showMaximized();
}
void MainWindow::on_aPage_triggered()
{
    if (g_wPage == nullptr)
    {
        g_wPage = new frm_Page;
        ui->mdiArea->addSubWindow(g_wPage);
    }
    g_wPage->showMaximized();
}

void MainWindow::on_aModbusFuncCode_triggered()
{
    if (g_wCmd == nullptr)
    {
        g_wCmd = new frmCommand;
        ui->mdiArea->addSubWindow(g_wCmd);
    }
    g_wCmd->showMaximized();
}

void MainWindow::on_aModbusPackages_triggered()
{
    if (g_wPackages == nullptr)
    {
        g_wPackages = new frmGetPackages;
        ui->mdiArea->addSubWindow(g_wPackages);
    }
    g_wPackages->showMaximized();
}

void MainWindow::on_aEncryptAES_triggered()
{
    if (g_wEncrypt == nullptr)
    {
        g_wEncrypt = new frmEncrypt;
        ui->mdiArea->addSubWindow(g_wEncrypt);
    }
    g_wEncrypt->showMaximized();
}
