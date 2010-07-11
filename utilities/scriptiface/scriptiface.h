#ifndef SCRIPTIFACE_H
#define SCRIPTIFACE_H

#include <QDialog>

namespace Ui {
    class scriptiface;
}

namespace Digikam{
	
class scriptiface : public QDialog {
    Q_OBJECT
public:
    scriptiface();//QWidget *parent = 0);
    ~scriptiface();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::scriptiface *ui;
};

#endif // SCRIPTIFACE_H
}//namespace
