/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
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
#include <kurlrequester.h>

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
        iconShowNameBox       = 0;
        iconShowSizeBox       = 0;
        iconShowDateBox       = 0;
        iconShowModDateBox    = 0;
        iconShowResolutionBox = 0;
        iconShowCommentsBox   = 0;
        iconShowTagsBox       = 0;
        iconShowRatingBox     = 0;
    }

    QCheckBox     *iconShowNameBox;
    QCheckBox     *iconShowSizeBox;
    QCheckBox     *iconShowDateBox;
    QCheckBox     *iconShowModDateBox;
    QCheckBox     *iconShowResolutionBox;
    QCheckBox     *iconShowCommentsBox;
    QCheckBox     *iconShowTagsBox;
    QCheckBox     *iconShowRatingBox;

    KURLRequester *albumPathEdit;

    KDialogBase   *mainDialog;
};

SetupGeneral::SetupGeneral(QWidget* parent, KDialogBase* dialog )
            : QWidget(parent)
{
    d = new SetupGeneralPriv;
    d->mainDialog       = dialog;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    QHGroupBox *albumPathBox = new QHGroupBox(parent);
    albumPathBox->setTitle(i18n("Album &Library Path"));

    d->albumPathEdit = new KURLRequester(albumPathBox);
    d->albumPathEdit->setMode(KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly);    
    QWhatsThis::add( d->albumPathEdit, i18n("<p>Here you can set the main path to the digiKam album "
                                            "library in your computer."
                                            "<p>Write access is required for this path and do not use a "
                                            "remote path here, like an NFS mounted file system."));


    connect(d->albumPathEdit, SIGNAL(urlSelected(const QString &)),
            this, SLOT(slotChangeAlbumPath(const QString &)));

    connect(d->albumPathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotPathEdited(const QString&)) );

    layout->addWidget(albumPathBox);

    // --------------------------------------------------------

    QVGroupBox *iconTextGroup = new QVGroupBox(i18n("Thumbnails"), parent);
    iconTextGroup->setColumnLayout(0, Qt::Vertical );
    iconTextGroup->layout()->setMargin(KDialog::marginHint());
    QGridLayout* tagSettingsLayout = new QGridLayout(iconTextGroup->layout(), 3, 9,
                                                     KDialog::spacingHint());

    d->iconShowNameBox = new QCheckBox(iconTextGroup);
    d->iconShowNameBox->setText(i18n("Show file &name"));
    QWhatsThis::add( d->iconShowNameBox, i18n("<p>Set this option to show file name below image thumbnail."));
    tagSettingsLayout->addWidget(d->iconShowNameBox, 0, 0);

    d->iconShowSizeBox = new QCheckBox(iconTextGroup);
    d->iconShowSizeBox->setText(i18n("Show file si&ze"));
    QWhatsThis::add( d->iconShowSizeBox, i18n("<p>Set this option to show file size below image thumbnail."));
    tagSettingsLayout->addWidget(d->iconShowSizeBox, 1, 0);

    d->iconShowDateBox = new QCheckBox(iconTextGroup);
    d->iconShowDateBox->setText(i18n("Show file creation &date"));
    QWhatsThis::add( d->iconShowDateBox, i18n("<p>Set this option to show file creation date "
                                              "below image thumbnail."));
    tagSettingsLayout->addWidget(d->iconShowDateBox, 2, 0);

    d->iconShowModDateBox = new QCheckBox(iconTextGroup);
    d->iconShowModDateBox->setText(i18n("Show file &modification date"));
    QWhatsThis::add( d->iconShowModDateBox, i18n("<p>Set this option to show file modification date "
                                                 "below image thumbnail."));
    tagSettingsLayout->addWidget(d->iconShowModDateBox, 3, 0);

    d->iconShowCommentsBox = new QCheckBox(iconTextGroup);
    d->iconShowCommentsBox->setText(i18n("Show digiKam &comments"));
    QWhatsThis::add( d->iconShowCommentsBox, i18n("<p>Set this option to show digiKam comments "
                                                  "below image thumbnail."));
    tagSettingsLayout->addWidget(d->iconShowCommentsBox, 4, 0);

    d->iconShowTagsBox = new QCheckBox(iconTextGroup);
    d->iconShowTagsBox->setText(i18n("Show digiKam &tags"));
    QWhatsThis::add( d->iconShowTagsBox, i18n("<p>Set this option to show digiKam tags "
                                                  "below image thumbnail."));
    tagSettingsLayout->addWidget(d->iconShowTagsBox, 5, 0);

    d->iconShowRatingBox = new QCheckBox(iconTextGroup);
    d->iconShowRatingBox->setText(i18n("Show digiKam &rating"));
    QWhatsThis::add( d->iconShowRatingBox, i18n("<p>Set this option to show digiKam rating "
                                                "below image thumbnail."));
    tagSettingsLayout->addWidget(d->iconShowRatingBox, 6,0);

    d->iconShowResolutionBox = new QCheckBox(iconTextGroup);
    d->iconShowResolutionBox->setText(i18n("Show ima&ge dimensions (warning: slow)"));
    QWhatsThis::add( d->iconShowResolutionBox, i18n("<p>Set this option to show picture size in pixels "
                                                    "below image thumbnail."));
    tagSettingsLayout->addWidget(d->iconShowResolutionBox, 7, 0);

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

    settings->setAlbumLibraryPath(d->albumPathEdit->url());

    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    settings->setIconShowModDate(d->iconShowModDateBox->isChecked());
    settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowComments(d->iconShowCommentsBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());

    settings->saveSettings();
}

void SetupGeneral::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->albumPathEdit->setURL(settings->getAlbumLibraryPath());

    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    d->iconShowModDateBox->setChecked(settings->getIconShowModDate());
    d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowCommentsBox->setChecked(settings->getIconShowComments());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());
}

void SetupGeneral::slotChangeAlbumPath(const QString &result)
{
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
        d->albumPathEdit->setURL(QDir::homeDirPath() + '/' + newPath);
    }

    QFileInfo targetPath(newPath);
    QDir dir(newPath);
    d->mainDialog->enableButtonOK(dir.exists() && dir != QDir(QDir::homeDirPath ()));
}

}  // namespace Digikam

#include "setupgeneral.moc"
