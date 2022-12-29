#include "frmcommand.h"
#include "ui_frmcommand.h"
#include "hq_base_modbus.h"

frmCommand *g_wCmd = nullptr;

frmCommand::frmCommand(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmCommand)
{
    ui->setupUi(this);
}

frmCommand::~frmCommand()
{
    g_wCmd = nullptr;
    delete ui;
}

void frmCommand::on_pushButton_clicked()
{
    QStringList list = Hq_Base_ModBus::f_PackagesToCmdList(ui->lineEdit->text());
    if (!list.isEmpty())
    {
        ui->lineEdit_2->setText(list.at(0));
        ui->lineEdit_4->setText(Hq_Base_ModBus::f_ReplaceFuncCode(list.at(0), ui->lineEdit_3->text()));
    }
    ui->lineEdit_5->setText(Hq_Base_ModBus::f_GetCRC(ui->lineEdit->text(), false));
    ui->lineEdit_6->setText(ui->lineEdit->text() + ui->lineEdit_5->text());
}
