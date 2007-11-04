/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QComboBox>
#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kpagedialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

// // Local includes.

#include "albumsettings.h"
#include "setupalbumview.h"
#include "setupalbumview.moc"

namespace Digikam
{

class SetupAlbumViewPriv
{
public:

    SetupAlbumViewPriv()
    {
        albumPathEdit            = 0;
        iconTreeThumbSize        = 0;
        iconTreeThumbLabel       = 0;
        iconShowNameBox          = 0;
        iconShowSizeBox          = 0;
        iconShowDateBox          = 0;
        iconShowModDateBox       = 0;
        iconShowResolutionBox    = 0;
        iconShowCommentsBox      = 0;
        iconShowTagsBox          = 0;
        iconShowRatingBox        = 0;
        rightClickActionComboBox = 0;
        previewLoadFullImageSize = 0;
        databasePathEdit         = 0;
    }

    QLabel        *iconTreeThumbLabel;

    QCheckBox     *iconShowNameBox;
    QCheckBox     *iconShowSizeBox;
    QCheckBox     *iconShowDateBox;
    QCheckBox     *iconShowModDateBox;
    QCheckBox     *iconShowResolutionBox;
    QCheckBox     *iconShowCommentsBox;
    QCheckBox     *iconShowTagsBox;
    QCheckBox     *iconShowRatingBox;
    QCheckBox     *previewLoadFullImageSize;

    QComboBox     *iconTreeThumbSize;
    QComboBox     *rightClickActionComboBox;

    KUrlRequester *albumPathEdit;
    KUrlRequester *databasePathEdit;

    KPageDialog   *mainDialog;
};

SetupAlbumView::SetupAlbumView(KPageDialog* dialog, QWidget* parent)
              : QWidget(parent)
{
    d = new SetupAlbumViewPriv;
    d->mainDialog = dialog;

    QVBoxLayout *layout = new QVBoxLayout( this );

    // --------------------------------------------------------

    QGroupBox *albumPathBox = new QGroupBox(i18n("Album &Library"), this);
    QVBoxLayout *gLayout1   = new QVBoxLayout(albumPathBox);

    QLabel *rootsPathLabel  = new QLabel(i18n("Root album path:"), albumPathBox);

    d->albumPathEdit = new KUrlRequester(albumPathBox);
    d->albumPathEdit->setMode(KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly);    
    d->albumPathEdit->setToolTip(i18n("<p>Here you can set the main path to the digiKam album "
                                      "library in your computer."
                                      "<p>Write access is required for this path."));

    connect(d->albumPathEdit, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotChangeAlbumPath(const KUrl &)));

    connect(d->albumPathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotAlbumPathEdited(const QString&)) );

    QLabel *databasePathLabel = new QLabel(i18n("Path to store database:"), albumPathBox);
    d->databasePathEdit       = new KUrlRequester(albumPathBox);
    d->databasePathEdit->setMode(KFile::Directory | KFile::LocalOnly);    

    d->databasePathEdit->setToolTip(i18n("<p>Here you can set the path use to host to the digiKam database "
                                         "in your computer. The database file is common for all roots album path. "
                                         "<p>Write access is required for this path."
                                         "<p>Do not use a remote path here, like an NFS mounted file system."));

    connect(d->databasePathEdit, SIGNAL(urlSelected(const KUrl &)),
            this, SLOT(slotChangeDatabsePath(const KUrl &)));

    connect(d->databasePathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotDatabasePathEdited(const QString&)) );

    gLayout1->addWidget(rootsPathLabel);
    gLayout1->addWidget(d->albumPathEdit);
    gLayout1->addWidget(databasePathLabel);
    gLayout1->addWidget(d->databasePathEdit);
    gLayout1->setSpacing(0);
    gLayout1->setMargin(KDialog::spacingHint());
   
    // --------------------------------------------------------

    QGroupBox *iconTextGroup = new QGroupBox(i18n("Thumbnail Information"), this);
    QVBoxLayout *gLayout2    = new QVBoxLayout(iconTextGroup);

    d->iconShowNameBox = new QCheckBox(i18n("Show file &name"), iconTextGroup);
    d->iconShowNameBox->setWhatsThis( i18n("<p>Set this option to show file name below image thumbnail."));

    d->iconShowSizeBox = new QCheckBox(i18n("Show file si&ze"), iconTextGroup);
    d->iconShowSizeBox->setWhatsThis( i18n("<p>Set this option to show file size below image thumbnail."));

    d->iconShowDateBox = new QCheckBox(i18n("Show file creation &date"), iconTextGroup);
    d->iconShowDateBox->setWhatsThis( i18n("<p>Set this option to show file creation date "
                                           "below image thumbnail."));

    d->iconShowModDateBox = new QCheckBox(i18n("Show file &modification date"), iconTextGroup);
    d->iconShowModDateBox->setWhatsThis( i18n("<p>Set this option to show file modification date "
                                              "below image thumbnail."));

    d->iconShowCommentsBox = new QCheckBox(i18n("Show digiKam &captions"), iconTextGroup);
    d->iconShowCommentsBox->setWhatsThis( i18n("<p>Set this option to show digiKam captions "
                                               "below image thumbnail."));

    d->iconShowTagsBox = new QCheckBox(i18n("Show digiKam &tags"), iconTextGroup);
    d->iconShowTagsBox->setWhatsThis( i18n("<p>Set this option to show digiKam tags "
                                           "below image thumbnail."));

    d->iconShowRatingBox = new QCheckBox(i18n("Show digiKam &rating"), iconTextGroup);
    d->iconShowRatingBox->setWhatsThis( i18n("<p>Set this option to show digiKam rating "
                                             "below image thumbnail."));

    d->iconShowResolutionBox = new QCheckBox(i18n("Show ima&ge dimensions (warning: slow)"), iconTextGroup);
    d->iconShowResolutionBox->setWhatsThis( i18n("<p>Set this option to show image size in pixels "
                                                 "below image thumbnail."));

    gLayout2->addWidget(d->iconShowNameBox);
    gLayout2->addWidget(d->iconShowSizeBox);
    gLayout2->addWidget(d->iconShowDateBox);
    gLayout2->addWidget(d->iconShowModDateBox);
    gLayout2->addWidget(d->iconShowCommentsBox);
    gLayout2->addWidget(d->iconShowTagsBox);
    gLayout2->addWidget(d->iconShowRatingBox);
    gLayout2->addWidget(d->iconShowResolutionBox);
    gLayout2->setSpacing(0);
    gLayout2->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox *interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), this);
    QGridLayout* ifaceSettingsLayout = new QGridLayout(interfaceOptionsGroup);

    d->iconTreeThumbLabel = new QLabel(i18n("Sidebar thumbnail size:"), interfaceOptionsGroup);
    d->iconTreeThumbSize  = new QComboBox(interfaceOptionsGroup);
    d->iconTreeThumbSize->addItem(QString("16"));
    d->iconTreeThumbSize->addItem(QString("22"));
    d->iconTreeThumbSize->addItem(QString("32"));
    d->iconTreeThumbSize->addItem(QString("48"));
    d->iconTreeThumbSize->setToolTip(i18n("<p>Set this option to configure the size "
                                          "in pixels of the thumbnails in digiKam's sidebars. "
                                          "This option will take effect when you restart "
                                          "digiKam."));

    QLabel *rightClickLabel     = new QLabel(i18n("Thumbnail click action:"), interfaceOptionsGroup);
    d->rightClickActionComboBox = new QComboBox(interfaceOptionsGroup);
    d->rightClickActionComboBox->addItem(i18n("Show embedded preview"), AlbumSettings::ShowPreview);
    d->rightClickActionComboBox->addItem(i18n("Start image editor"), AlbumSettings::StartEditor);
    d->rightClickActionComboBox->setToolTip(i18n("<p>Select here the right action to do when you "
                                                 "right click with mouse button on thumbnail."));

    d->previewLoadFullImageSize = new QCheckBox(i18n("Embedded preview load full image size"), interfaceOptionsGroup);
    d->previewLoadFullImageSize->setWhatsThis( i18n("<p>Set this option to load full image size "
                     "with embedded preview instead a reduced one. Because this option will take more time "
                     "to load image, use it only if you have a fast computer."));

    ifaceSettingsLayout->setMargin(KDialog::spacingHint());
    ifaceSettingsLayout->setSpacing(KDialog::spacingHint());
    ifaceSettingsLayout->addWidget(d->iconTreeThumbLabel, 0, 0, 1, 1);
    ifaceSettingsLayout->addWidget(d->iconTreeThumbSize, 0, 1, 1, 1);
    ifaceSettingsLayout->addWidget(rightClickLabel, 1 , 0, 1, 1);
    ifaceSettingsLayout->addWidget(d->rightClickActionComboBox, 1, 1, 1, 4);
    ifaceSettingsLayout->addWidget(d->previewLoadFullImageSize, 2, 0, 1, 5 );

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(albumPathBox);
    layout->addWidget(iconTextGroup);
    layout->addWidget(interfaceOptionsGroup);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupAlbumView::~SetupAlbumView()
{
    delete d;
}

void SetupAlbumView::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setAlbumLibraryPath(d->albumPathEdit->url().path());
    settings->setDatabaseFilePath(d->databasePathEdit->url().path());

    settings->setDefaultTreeIconSize(d->iconTreeThumbSize->currentText().toInt());
    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    settings->setIconShowModDate(d->iconShowModDateBox->isChecked());
    settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowComments(d->iconShowCommentsBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());

    settings->setItemRightClickAction((AlbumSettings::ItemRightClickAction)
                                      d->rightClickActionComboBox->currentIndex());

    settings->setPreviewLoadFullImageSize(d->previewLoadFullImageSize->isChecked());
    settings->saveSettings();
}

void SetupAlbumView::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->albumPathEdit->setUrl(settings->getAlbumLibraryPath());
    d->databasePathEdit->setUrl(settings->getDatabaseFilePath());

    if (settings->getDefaultTreeIconSize() == 16)
        d->iconTreeThumbSize->setCurrentIndex(0);
    else if (settings->getDefaultTreeIconSize() == 22)
        d->iconTreeThumbSize->setCurrentIndex(1);
    else if (settings->getDefaultTreeIconSize() == 32)
        d->iconTreeThumbSize->setCurrentIndex(2);
    else 
        d->iconTreeThumbSize->setCurrentIndex(3);

    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    d->iconShowModDateBox->setChecked(settings->getIconShowModDate());
    d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowCommentsBox->setChecked(settings->getIconShowComments());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());

    d->rightClickActionComboBox->setCurrentIndex((int)settings->getItemRightClickAction());

    d->previewLoadFullImageSize->setChecked(settings->getPreviewLoadFullImageSize());
}

void SetupAlbumView::slotChangeAlbumPath(const KUrl &result)
{
    if (KUrl(result).equals(KUrl(QDir::homePath()), KUrl::CompareWithoutTrailingSlash)) 
    {
        KMessageBox::sorry(0, i18n("Sorry; cannot use home directory as album library."));
        return;
    }

    QFileInfo targetPath(result.path());

    if (!result.isEmpty() && !targetPath.isWritable()) 
    {
        KMessageBox::information(0, i18n("No write access for this root album path.\n"
                                         "Warning: image and metadata editing will not work."));
    }
}

void SetupAlbumView::slotChangeDatabasePath(const KUrl &result)
{
    QFileInfo targetPath(result.path());

    if (!result.isEmpty() && !targetPath.isWritable()) 
    {
        KMessageBox::information(0, i18n("No write access for this path to store database.\n"
                                         "Warning: the caption and tag features will not work."));
    }
}

void SetupAlbumView::slotAlbumPathEdited(const QString& newPath)
{
    if (newPath.isEmpty()) 
    {
        d->mainDialog->enableButtonOk(false);
        return;
    }

    if (!newPath.startsWith("/")) 
    {
        d->albumPathEdit->setUrl(QDir::homePath() + '/' + newPath);
    }

    checkforOkButton();
}

void SetupAlbumView::slotDatabasePathEdited(const QString& newPath)
{
    if (newPath.isEmpty()) 
    {
        d->mainDialog->enableButtonOk(false);
        return;
    }

    if (!newPath.startsWith("/")) 
    {
        d->databasePathEdit->setUrl(QDir::homePath() + '/' + newPath);
    }

    checkforOkButton();
}

void SetupAlbumView::checkforOkButton()
{
    QFileInfo albumPath(d->albumPathEdit->url().path());
    QDir albumDir(d->albumPathEdit->url().path());
    bool albumOk = albumDir.exists() && (albumDir.path() != QDir::homePath());
    
    QDir dbDir(d->databasePathEdit->url().path());
    bool dbOk = dbDir.exists();

    d->mainDialog->enableButtonOk(dbOk && albumOk);
}

}  // namespace Digikam
