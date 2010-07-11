#include "scriptiface.h"
#include "ui_scriptiface.h"

namespace Digikam
{

scriptiface::scriptiface()
//         : QWidget *parent),
           : QDialog(),
             ui(new Ui::scriptiface)
{
    ui->setupUi(this);
}

scriptiface::~scriptiface()
{
    delete ui;
}

void scriptiface::changeEvent(QEvent* e)
{
    QDialog::changeEvent(e);
    switch (e->type()) 
    {
        case QEvent::LanguageChange:
             ui->retranslateUi(this);
             break;
        default:
             break;
    }
}

} // namespace DigiKam
