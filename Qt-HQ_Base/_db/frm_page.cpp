#include "frm_page.h"
#include "ui_frm_page.h"
#include "hq_base.h"
#include "hq_base_db_model.h"

frm_Page *g_wPage = nullptr;

frm_Page::frm_Page(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frm_Page)
{
    ui->setupUi(this);
    f_Init();
}

frm_Page::~frm_Page()
{
    delete ui;
}
void frm_Page::f_Init()
{
    ui->txtModelName->setText("Base_Model_Sub");
    connect(ui->oPage, SIGNAL(sigRecordSet(QString)), this, SLOT(onQuery(QString)));
    f_Load();
}
void frm_Page::f_Load()
{
    QString sModelName = ui->txtModelName->text();
    QString sModel = HQ_Base_Db_Model::f_GetModel(sModelName);
    ui->oGrid->f_FillHeader(sModel);

    QJsonObject oModel = HQ_Base::f_Json_StringToJsonObj(sModel);
    QString sTableName = oModel.value("TableName").toString();
    QString sOrderBy = oModel.value("OrderBy").toString();
    ui->oPage->f_Init(sTableName, sOrderBy);
}
void frm_Page::onQuery(QString sJsonResult)
{
    ui->oGrid->f_FillContents(sJsonResult);
}
void frm_Page::on_btnRefresh_clicked()
{
    f_Load();
}
