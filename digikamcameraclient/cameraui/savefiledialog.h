#ifndef SAVEFILEDIALOG_H
#define SAVEFILEDIALOG_H

#include <qdialog.h>

class QString;
class QLineEdit;
class QPushButton;


class SavefileDialog : public QDialog {

    Q_OBJECT

public:

    enum Operation {Rename,
                    Skip,
                    SkipAll,
                    Overwrite,
                    OverwriteAll,
                    None};

    SavefileDialog(const QString& file, QWidget *parent=0,
                    const char* name=0, bool modal=true);
    ~SavefileDialog();

    Operation saveFileOperation();
    QString   renameFile();

private slots:

    void slot_rename() {op=Rename; accept();}
    void slot_skip() {op=Skip; accept();}
    void slot_skipAll() {op=SkipAll; accept();}
    void slot_overwrite() {op=Overwrite; accept();}
    void slot_overwriteAll() {op=OverwriteAll; accept();}
    void slot_renameEnabled();

private:

    QLineEdit *renameEdit;
    QPushButton *renameBtn;
    Operation op;

};


#endif
