#include "frmencrypt.h"
#include "ui_frmencrypt.h"
#include "hq_base_encrypt_aes.h"

frmEncrypt *g_wEncrypt = nullptr;

frmEncrypt::frmEncrypt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmEncrypt)
{
    ui->setupUi(this);
}

frmEncrypt::~frmEncrypt()
{
    g_wEncrypt = nullptr;
    delete ui;
}

void frmEncrypt::on_btnAES_Encrypt_clicked()
{
    QString sStr = ui->txtAES_String->text();
    QString sKey = ui->txtAES_Key->text();
    QString sEnCrypt = HQ_Base_Encrypt_AES::Crypt(HQ_Base_Encrypt_AES::AES_128,
                                                  HQ_Base_Encrypt_AES::ECB,
                                          sStr, sKey);
    ui->txtAES_Encrypt->setText(sEnCrypt);
}

void frmEncrypt::on_btnAES_Decrypt_clicked()
{
    QString sEncrypt = ui->txtAES_Encrypt->text();
    QString sKey = ui->txtAES_Key->text();
    QString sDeCrypt = HQ_Base_Encrypt_AES::Decrypt(HQ_Base_Encrypt_AES::AES_128,
                                                    HQ_Base_Encrypt_AES::ECB,
                                            sEncrypt, sKey);
    ui->txtAES_String->setText(sDeCrypt);
}
