#ifndef SCRIPTIFACE_H
#define SCRIPTIFACE_H

#include <QDialog>

#include <digikam_export.h>

namespace Ui
{
    class scriptiface;
}

namespace Digikam
{

class DIGIKAM_EXPORT scriptiface : public QDialog
{
    Q_OBJECT

public:

    scriptiface(); //QWidget *parent = 0);
    ~scriptiface();

protected:

    void changeEvent(QEvent* e);

private:

    Ui::scriptiface* ui;
};

} // namespace Digikam

#endif // SCRIPTIFACE_H
