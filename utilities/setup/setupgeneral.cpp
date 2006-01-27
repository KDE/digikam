/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-02-01
 * Description : general configuration setup tab
 *
 * Copyright 2003-2004 by Renchi Raju
 * Copyright 2005-2006 by Gilles Caulier
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


class SetupGeneralPriv
{
public:

    SetupGeneralPriv()
    {
        albumPathEdit         = 0;
        showToolTipsBox       = 0;
        iconShowNameBox       = 0;
        iconShowSizeBox       = 0;
        iconShowDateBox       = 0;
        iconShowResolutionBox = 0;
        iconShowCommentsBox   = 0;
        iconShowTagsBox       = 0;
        iconShowRatingBox     = 0;
    }

    QLineEdit   *albumPathEdit;

    QCheckBox   *showToolTipsBox;
    QCheckBox   *iconShowNameBox;
    QCheckBox   *iconShowSizeBox;
    QCheckBox   *iconShowDateBox;
    QCheckBox   *iconShowResolutionBox;
    QCheckBox   *iconShowCommentsBox;
    QCheckBox   *iconShowTagsBox;
    QCheckBox   *iconShowRatingBox;

    KDialogBase *mainDialog;
};

SetupGeneral::SetupGeneral(QWidget* parent, KDialogBase* dialog )
            : QWidget(parent)
{
    d = new SetupGeneralPriv;
    d->mainDialog = dialog;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );
    
    // --------------------------------------------------------
    
    QHGroupBox *albumPathBox = new QHGroupBox(parent);
    albumPathBox->setTitle(i18n("Album &Library Path"));
    
    d->albumPathEdit = new QLineEdit(albumPathBox);
    QWhatsThis::add( d->albumPathEdit, i18n("<p>Here you can set the main path to the digiKam album "
                                            "library in your computer.\n"
                                            "Write access is required for this path."));
    
    QPushButton *changePathButton = new QPushButton(i18n("&Change..."),
                                                    albumPathBox);
    connect(changePathButton, SIGNAL(clicked()),
            this, SLOT(slotChangeAlbumPath()));
    connect( d->albumPathEdit, SIGNAL(textChanged(const QString&)),
                this, SLOT(slotPathEdited(const QString&)) );
    
    layout->addWidget(albumPathBox);
    
    // --------------------------------------------------------
    
    QVGroupBox *tipSettingBox = new QVGroupBox(parent);
    tipSettingBox->setTitle(i18n("Tooltips Settings"));
    
    d->showToolTipsBox = new QCheckBox(tipSettingBox);
    d->showToolTipsBox->setText(i18n("Show toolti&ps for items"));
    
    layout->addWidget(tipSettingBox);
    
    // --------------------------------------------------------
    QVGroupBox *iconTextGroup = new QVGroupBox(i18n("Thumbnails"), parent);
    iconTextGroup->setColumnLayout(0, Qt::Vertical );
    iconTextGroup->layout()->setMargin(KDialog::marginHint());
    QGridLayout* tagSettingsLayout = new QGridLayout(iconTextGroup->layout(), 3, 8,
                                                        KDialog::spacingHint());
    
    d->iconShowNameBox = new QCheckBox(iconTextGroup);
    d->iconShowNameBox->setText(i18n("Show file &name"));
    tagSettingsLayout->addWidget(d->iconShowNameBox, 0, 0);
    
    d->iconShowTagsBox = new QCheckBox(iconTextGroup);
    d->iconShowTagsBox->setText(i18n("Show file &tags"));
    tagSettingsLayout->addWidget(d->iconShowTagsBox, 1, 0);
    
    d->iconShowSizeBox = new QCheckBox(iconTextGroup);
    d->iconShowSizeBox->setText(i18n("Show file si&ze"));
    tagSettingsLayout->addWidget(d->iconShowSizeBox, 2, 0);
    
    d->iconShowDateBox = new QCheckBox(iconTextGroup);
    d->iconShowDateBox->setText(i18n("Show file &modification date"));
    tagSettingsLayout->addWidget(d->iconShowDateBox, 3, 0);
    
    d->iconShowCommentsBox = new QCheckBox(iconTextGroup);
    d->iconShowCommentsBox->setText(i18n("Show &digiKam comments"));
    tagSettingsLayout->addWidget(d->iconShowCommentsBox, 4, 0);
    
    d->iconShowRatingBox = new QCheckBox(iconTextGroup);
    d->iconShowRatingBox->setText(i18n("Show file rating"));
    tagSettingsLayout->addWidget(d->iconShowRatingBox, 5,0);
    
    d->iconShowResolutionBox = new QCheckBox(iconTextGroup);
    d->iconShowResolutionBox->setText(i18n("Show ima&ge dimensions (warning: slow)"));
    tagSettingsLayout->addWidget(d->iconShowResolutionBox, 6, 0);
    
    layout->addWidget(iconTextGroup);
    
    // --------------------------------------------------------
    
    layout->addStretch();
    
    readSettings();
    adjustSize();
}

SetupGeneral::~SetupGeneral()
{
    delete d;
}

void SetupGeneral::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    settings->setAlbumLibraryPath(d->albumPathEdit->text());

    settings->setShowToolTips(d->showToolTipsBox->isChecked());

    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowComments(d->iconShowCommentsBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());

    settings->saveSettings();
}

void SetupGeneral::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->albumPathEdit->setText(settings->getAlbumLibraryPath());

    d->showToolTipsBox->setChecked(settings->getShowToolTips());

    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowCommentsBox->setChecked(settings->getIconShowComments());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());
}

void SetupGeneral::slotChangeAlbumPath()
{
    QString result = KFileDialog::getExistingDirectory(d->albumPathEdit->text(), this);

    if (KURL(result).equals(KURL(QDir::homeDirPath()), true)) 
    {
        KMessageBox::sorry(0, i18n("Sorry; cannot use home directory as albums library."));
        return;
    }

    QFileInfo targetPath(result);

    if (!result.isEmpty() && !targetPath.isWritable()) 
    {
        KMessageBox::information(0, i18n("No write access for this path.\n"
                                         "Warning: the comments and tag features will not work."));
    	return;
    }

    if (!result.isEmpty()) 
    {
        d->albumPathEdit->setText(result);
    }
}

void SetupGeneral::slotPathEdited(const QString& newPath)
{
    if (newPath.isEmpty()) 
    {
       d->mainDialog->enableButtonOK(false);
       return;
    }

    if (!newPath.startsWith("/")) 
    {
        d->albumPathEdit->setText(QDir::homeDirPath()+"/"+newPath);
    }

    QFileInfo targetPath(newPath);
    QDir dir(newPath);
    d->mainDialog->enableButtonOK(dir.exists() && dir != QDir(QDir::homeDirPath ()));
}

}  // namespace Digikam

#include "setupgeneral.moc"
