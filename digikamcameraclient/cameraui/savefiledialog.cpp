#include <qstring.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kbuttonbox.h>
#include <kiconloader.h>

#include "savefiledialog.h"





SavefileDialog::SavefileDialog(const QString& file, QWidget *parent,
                                 const char* name, bool modal):
    QDialog(parent, name, modal)
{

    QFileInfo fileInfo(file);

    setCaption(i18n("File already exists"));
    QLabel *descLbl = new QLabel(i18n("The file '%1' already exists!").arg(fileInfo.absFilePath()), this);

    renameEdit = new QLineEdit(this);
    renameEdit->setText(fileInfo.fileName());
    connect(renameEdit, SIGNAL(textChanged(const QString &)), this,
	  SLOT(slot_renameEnabled()));

    KButtonBox *bbox = new KButtonBox(this);
    renameBtn = bbox->addButton(i18n("Rename"), this, SLOT(slot_rename()), true);
    renameBtn->setEnabled(false);
    bbox->addButton(i18n("Skip"), this, SLOT(slot_skip()), false);
    bbox->addButton(i18n("Skip All"), this, SLOT(slot_skipAll()), true);
    bbox->addButton(i18n("Overwrite"), this, SLOT(slot_overwrite()), true);
    bbox->addButton(i18n("Overwrite All"), this, SLOT(slot_overwriteAll()), true);
    QPushButton *cancelBtn = bbox->addButton(i18n("Cancel"), this, SLOT(reject()), true);
    cancelBtn->setDefault(true);
    bbox->layout();

    QGridLayout *layout = new QGridLayout(this, 0, 0, 15);
    layout->addMultiCellWidget(descLbl, 0, 0, 0, 3);
    layout->addMultiCellWidget(renameEdit, 3, 3, 0, 3);
    layout->addMultiCellWidget(bbox, 4, 4, 0, 3);


}

SavefileDialog::~SavefileDialog()
{
}

SavefileDialog::Operation SavefileDialog::saveFileOperation()
{

    return op;

}

QString SavefileDialog::renameFile()
{
    return renameEdit->text();
}

void SavefileDialog::slot_renameEnabled()
{
    renameBtn->setEnabled(true);
    renameBtn->setDefault(true);
}
