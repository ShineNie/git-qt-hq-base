#include "frmgetpackages.h"
#include "ui_frmgetpackages.h"
#include "hq_base_modbus.h"
#include <QStandardItemModel>

frmGetPackages *g_wPackages = nullptr;

frmGetPackages::frmGetPackages(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmGetPackages)
{
    ui->setupUi(this);
    m_ModelCmd = new QStandardItemModel;
    ui->listCmd->setModel(m_ModelCmd);
}

frmGetPackages::~frmGetPackages()
{
    g_wPackages = nullptr;
    delete ui;
}

void frmGetPackages::on_btnGo_clicked()
{
    QList<QStandardItem*> listItem = Hq_Base_ModBus::f_PackagesToCmdListAsItem(ui->txt->toPlainText());
    m_ModelCmd->clear();
    for (int i =0; i < listItem.length(); i++)
    {
        m_ModelCmd->appendRow(listItem.at(i));
    }
}

void frmGetPackages::on_btnClear_clicked()
{
    m_ModelCmd->clear();
    ui->listValue->clear();
}

void frmGetPackages::on_listCmd_activated(const QModelIndex &index)
{
    //Using key word: 'item_ruler_modbus'.
    //See the using rule in file 'hq_base_modbus.h'.
    QStandardItem *item = m_ModelCmd->itemFromIndex(index);
    QStringList list = item->
            data(Qt::UserRole + Hq_Base_ModBus::eCmdPart_Values).value<QStringList>();
    ui->listValue->clear();
    ui->listValue->addItems(list);
}

void frmGetPackages::on_listCmd_clicked(const QModelIndex &index)
{
    on_listCmd_activated(index);
}

