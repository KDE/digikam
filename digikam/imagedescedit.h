#ifndef IMAGEDESCEDIT_H
#define IMAGEDESCEDIT_H

#include <kdialogbase.h>
#include <qstring.h>

class QLabel;
class QTextEdit;

class ImageDescEdit : public KDialogBase
{
    Q_OBJECT

public:

    ImageDescEdit(const QString& itemName,
                  const QString& itemComments,
                  QWidget *parent=0);
    ~ImageDescEdit();

    static bool editComments(const QString& itemName,
                             QString& itemComments,
                             QWidget *parent=0);

private:

    QLabel* mNameLabel;
    QTextEdit* mCommentsEdit;
    QString mItemName;

private slots:

    void slot_textChanged();

};

#endif
