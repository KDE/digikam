#ifndef DIGIKAMFIRSTRUN_H
#define DIGIKAMFIRSTRUN_H

#include <qdialog.h>

class QCheckBox;
class QLineEdit;
class QPushButton;
class KConfig;

class DigikamFirstRun : public QDialog
{
    Q_OBJECT

public:

    DigikamFirstRun( KConfig* config,
                     QWidget* parent = 0,
                     const char* name = 0,
                     bool modal = true,
                     WFlags fl = WDestructiveClose );
    ~DigikamFirstRun();

private:

    QLineEdit*   pathEdit_;
    KConfig*     config_;
    QPushButton *okButton_;
    QPushButton *cancelButton_;

private slots:

    void accept();
    void slotChangePath();
    void slotPathEdited(const QString& newPath);
};

#endif // DIGIKAMFIRSTRUN_H
