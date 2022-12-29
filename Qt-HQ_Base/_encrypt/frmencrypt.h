#ifndef FRMENCRYPT_H
#define FRMENCRYPT_H

#include <QWidget>

namespace Ui {
class frmEncrypt;
}

class frmEncrypt : public QWidget
{
    Q_OBJECT

public:
    explicit frmEncrypt(QWidget *parent = 0);
    ~frmEncrypt();

private slots:
    void on_btnAES_Encrypt_clicked();

    void on_btnAES_Decrypt_clicked();

private:
    Ui::frmEncrypt *ui;
};

extern frmEncrypt *g_wEncrypt;

#endif // FRMENCRYPT_H
