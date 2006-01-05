/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-02-01
 * Description :
 *
 * Copyright 2003-2004 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// QT includes.

#include <qlayout.h>
#include <qcombobox.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qfileinfo.h>

// KDE includes.

#include <klocale.h>
#include <kdialogbase.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>

// // Local includes.

#include "albumsettings.h"
#include "setupgeneral.h"

namespace Digikam
{

SetupGeneral::SetupGeneral(QWidget* parent, KDialogBase* dialog )
            : QWidget(parent)
{
   mainDialog_ = dialog;
   QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

   // --------------------------------------------------------

   QHGroupBox *albumPathBox = new QHGroupBox(parent);
   albumPathBox->setTitle(i18n("Album &Library Path"));

   albumPathEdit = new QLineEdit(albumPathBox);
   QWhatsThis::add( albumPathEdit, i18n("<p>Here you can set the main path to the digiKam album "
                                        "library in your computer.\n"
                                        "Write access is required for this path."));

   QPushButton *changePathButton = new QPushButton(i18n("&Change..."),
                                                   albumPathBox);
   connect(changePathButton, SIGNAL(clicked()),
           this, SLOT(slotChangeAlbumPath()));
   connect( albumPathEdit, SIGNAL(textChanged(const QString&)),
             this, SLOT(slotPathEdited(const QString&)) );

   layout->addWidget(albumPathBox);

   // --------------------------------------------------------

   QVGroupBox *tipSettingBox = new QVGroupBox(parent);
   tipSettingBox->setTitle(i18n("Tooltips Settings"));

   showToolTipsBox_ = new QCheckBox(tipSettingBox);
   showToolTipsBox_->setText(i18n("Show toolti&ps for items"));

   layout->addWidget(tipSettingBox);

   // --------------------------------------------------------
   QVGroupBox *iconTextGroup = new QVGroupBox(i18n("Thumbnails"), parent);
   iconTextGroup->setColumnLayout(0, Qt::Vertical );
   iconTextGroup->layout()->setMargin(KDialog::marginHint());
   QGridLayout* tagSettingsLayout = new QGridLayout(iconTextGroup->layout(), 3, 8,
                                                    KDialog::spacingHint());

   iconShowNameBox_ = new QCheckBox(iconTextGroup);
   iconShowNameBox_->setText(i18n("Show file &name"));
   tagSettingsLayout->addWidget(iconShowNameBox_, 0, 0);

   iconShowTagsBox_ = new QCheckBox(iconTextGroup);
   iconShowTagsBox_->setText(i18n("Show file &tags"));
   tagSettingsLayout->addWidget(iconShowTagsBox_, 1, 0);

   iconShowSizeBox_ = new QCheckBox(iconTextGroup);
   iconShowSizeBox_->setText(i18n("Show file si&ze"));
   tagSettingsLayout->addWidget(iconShowSizeBox_, 2, 0);

   iconShowDateBox_ = new QCheckBox(iconTextGroup);
   iconShowDateBox_->setText(i18n("Show file &modification date"));
   tagSettingsLayout->addWidget(iconShowDateBox_, 3, 0);

   iconShowCommentsBox_ = new QCheckBox(iconTextGroup);
   iconShowCommentsBox_->setText(i18n("Show &digiKam comments"));
   tagSettingsLayout->addWidget(iconShowCommentsBox_, 4, 0);

   iconShowRatingBox_ = new QCheckBox(iconTextGroup);
   iconShowRatingBox_->setText(i18n("Show file rating"));
   tagSettingsLayout->addWidget(iconShowRatingBox_, 5,0);

   iconShowResolutionBox_ = new QCheckBox(iconTextGroup);
   iconShowResolutionBox_->setText(i18n("Show ima&ge dimensions (warning: slow)"));
   tagSettingsLayout->addWidget(iconShowResolutionBox_, 6, 0);

   layout->addWidget(iconTextGroup);

   // --------------------------------------------------------

   layout->addStretch();

   readSettings();
   adjustSize();
}

SetupGeneral::~SetupGeneral()
{
}

void SetupGeneral::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    settings->setAlbumLibraryPath(albumPathEdit->text());

    settings->setShowToolTips(showToolTipsBox_->isChecked());

    settings->setIconShowName(iconShowNameBox_->isChecked());
    settings->setIconShowTags(iconShowTagsBox_->isChecked());
    settings->setIconShowSize(iconShowSizeBox_->isChecked());
    settings->setIconShowDate(iconShowDateBox_->isChecked());
    settings->setIconShowResolution(iconShowResolutionBox_->isChecked());
    settings->setIconShowComments(iconShowCommentsBox_->isChecked());
    settings->setIconShowRating(iconShowRatingBox_->isChecked());

    settings->saveSettings();
}

void SetupGeneral::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    albumPathEdit->setText(settings->getAlbumLibraryPath());

    showToolTipsBox_->setChecked(settings->getShowToolTips());

    iconShowNameBox_->setChecked(settings->getIconShowName());
    iconShowTagsBox_->setChecked(settings->getIconShowTags());
    iconShowSizeBox_->setChecked(settings->getIconShowSize());
    iconShowDateBox_->setChecked(settings->getIconShowDate());
    iconShowResolutionBox_->setChecked(settings->getIconShowResolution());
    iconShowCommentsBox_->setChecked(settings->getIconShowComments());
    iconShowRatingBox_->setChecked(settings->getIconShowRating());
}

void SetupGeneral::slotChangeAlbumPath()
{
    QString  result =
        KFileDialog::getExistingDirectory(
            albumPathEdit->text(),
            this);

    if (KURL(result).equals(KURL(QDir::homeDirPath()), true)) {
        KMessageBox::sorry(0, i18n("Sorry; cannot use home directory as albums library."));
        return;
    }

    QFileInfo targetPath(result);
    if (!result.isEmpty() && !targetPath.isWritable()) {
        KMessageBox::information(0, i18n("No write access for this path.\n"
                                         "Warning: the comments and tag features will not work."));
    	return;
    }

    if (!result.isEmpty()) {
        albumPathEdit->setText(result);
    }
}

void SetupGeneral::slotPathEdited(const QString& newPath)
{
    if (newPath.isEmpty()) {
       mainDialog_->enableButtonOK(false);
       return;
    }

    if (!newPath.startsWith("/")) {
        albumPathEdit->setText(QDir::homeDirPath()+"/"+newPath);
    }

    QFileInfo targetPath(newPath);
    QDir dir(newPath);
    mainDialog_->enableButtonOK(dir.exists() &&
                                dir != QDir(QDir::homeDirPath ()));
}

}  // namespace Digikam

#include "setupgeneral.moc"
