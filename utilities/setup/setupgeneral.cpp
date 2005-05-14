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

#include "thumbnailsize.h"
#include "albumsettings.h"
#include "setupgeneral.h"


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

   QVGroupBox *tagSettingBox = new QVGroupBox(parent);
   tagSettingBox->setTitle(i18n("Tag Settings"));

   recurseTagsBox_ = new QCheckBox(tagSettingBox);
   recurseTagsBox_->setText(i18n("Show items in su&b-tags"));
   QWhatsThis::add( recurseTagsBox_, i18n("<p>When showing items in a Tag, also "
                                          "show items in sub-Tags."));

   layout->addWidget(tagSettingBox);


   // --------------------------------------------------------
   QVGroupBox *iconTextGroup = new QVGroupBox(i18n("Thumbnails"), parent);
   iconTextGroup->setColumnLayout(0, Qt::Vertical );
   iconTextGroup->layout()->setMargin(KDialog::marginHint());
   QGridLayout* tagSettingsLayout = new QGridLayout(iconTextGroup->layout(), 3, 8,
                                                    KDialog::spacingHint());

   QLabel* thumbnailSizeLabel = new QLabel(i18n("Default &size:"), iconTextGroup);
   tagSettingsLayout->addWidget(thumbnailSizeLabel , 0, 0);
   thumbnailSize_ = new QComboBox(iconTextGroup);
   //thumbnailSize_->insertItem(i18n("Tiny (32x32)"));
   thumbnailSize_->insertItem(i18n("Small (64x64)"));
   thumbnailSize_->insertItem(i18n("Medium (100x100)"));
   thumbnailSize_->insertItem(i18n("Large (160x160)"));
   thumbnailSize_->insertItem(i18n("Huge (256x256)"));
   thumbnailSizeLabel->setBuddy(thumbnailSize_);
   tagSettingsLayout->addWidget(thumbnailSize_, 0, 1);
   QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
   tagSettingsLayout->addItem(spacer, 0, 2);

   iconShowNameBox_ = new QCheckBox(iconTextGroup);
   iconShowNameBox_->setText(i18n("Show file &name"));
   tagSettingsLayout->addMultiCellWidget(iconShowNameBox_, 1, 1, 0, 2);

   iconShowTagsBox_ = new QCheckBox(iconTextGroup);
   iconShowTagsBox_->setText(i18n("Show file &tags"));
   tagSettingsLayout->addMultiCellWidget(iconShowTagsBox_, 2, 2, 0, 2);

   iconShowSizeBox_ = new QCheckBox(iconTextGroup);
   iconShowSizeBox_->setText(i18n("Show file si&ze"));
   tagSettingsLayout->addMultiCellWidget(iconShowSizeBox_, 3, 3, 0, 2);

   iconShowDateBox_ = new QCheckBox(iconTextGroup);
   iconShowDateBox_->setText(i18n("Show file &modification date"));
   tagSettingsLayout->addMultiCellWidget(iconShowDateBox_, 4, 4, 0, 2);

   iconShowCommentsBox_ = new QCheckBox(iconTextGroup);
   iconShowCommentsBox_->setText(i18n("Show &digiKam comments"));
   tagSettingsLayout->addMultiCellWidget(iconShowCommentsBox_, 5, 5, 0, 2);

   iconShowFileCommentsBox_ = new QCheckBox(iconTextGroup);
   iconShowFileCommentsBox_->setText(i18n("Sho&w comments stored in file (warning: slow)"));
   tagSettingsLayout->addMultiCellWidget(iconShowFileCommentsBox_, 6, 6, 0, 2);

   iconShowResolutionBox_ = new QCheckBox(iconTextGroup);
   iconShowResolutionBox_->setText(i18n("Show ima&ge dimensions (warning: slow)"));
   tagSettingsLayout->addMultiCellWidget(iconShowResolutionBox_, 7, 7, 0, 2);

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

    int iconSize = ThumbnailSize::Medium;
    switch(thumbnailSize_->currentItem())
    {
//         case(TinyThumb):
//             iconSize = ThumbnailSize::Tiny;
//             break;
        case(SmallThumb):
            iconSize = ThumbnailSize::Small;
            break;
        case(LargeThumb):
            iconSize = ThumbnailSize::Large;
            break;
        case(HugeThumb):
            iconSize = ThumbnailSize::Huge;
            break;
    }

    settings->setDefaultIconSize(iconSize);
    settings->setRecurseTags(recurseTagsBox_->isChecked());
    settings->setShowToolTips(showToolTipsBox_->isChecked());

    settings->setIconShowName(iconShowNameBox_->isChecked());
    settings->setIconShowTags(iconShowTagsBox_->isChecked());
    settings->setIconShowSize(iconShowSizeBox_->isChecked());
    settings->setIconShowDate(iconShowDateBox_->isChecked());
    settings->setIconShowResolution(iconShowResolutionBox_->isChecked());
    settings->setIconShowComments(iconShowCommentsBox_->isChecked());
    settings->setIconShowFileComments(iconShowFileCommentsBox_->isChecked());

    settings->saveSettings();
}

void SetupGeneral::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    albumPathEdit->setText(settings->getAlbumLibraryPath());

    switch(settings->getDefaultIconSize())
    {
//         case(ThumbnailSize::Tiny):
//             thumbnailSize_->setCurrentItem(TinyThumb);
//             break;
        case(ThumbnailSize::Small):
            thumbnailSize_->setCurrentItem(SmallThumb);
            break;
        case(ThumbnailSize::Large):
            thumbnailSize_->setCurrentItem(LargeThumb);
            break;
        case(ThumbnailSize::Huge):
            thumbnailSize_->setCurrentItem(HugeThumb);
            break;
        default: // Medium
            thumbnailSize_->setCurrentItem(MediumThumb);
            break;
    }

    recurseTagsBox_->setChecked(settings->getRecurseTags());
    showToolTipsBox_->setChecked(settings->getShowToolTips());

    iconShowNameBox_->setChecked(settings->getIconShowName());
    iconShowTagsBox_->setChecked(settings->getIconShowTags());
    iconShowSizeBox_->setChecked(settings->getIconShowSize());
    iconShowDateBox_->setChecked(settings->getIconShowDate());
    iconShowResolutionBox_->setChecked(settings->getIconShowResolution());
    iconShowCommentsBox_->setChecked(settings->getIconShowComments());
    iconShowFileCommentsBox_->setChecked(settings->getIconShowFileComments());
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

#include "setupgeneral.moc"
