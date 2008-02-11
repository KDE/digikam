/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

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
        iconTreeThumbSize            = 0;
        iconTreeThumbLabel           = 0;
        iconShowNameBox              = 0;
        iconShowSizeBox              = 0;
        iconShowDateBox              = 0;
        iconShowModDateBox           = 0;
        iconShowResolutionBox        = 0;
        iconShowCommentsBox          = 0;
        iconShowTagsBox              = 0;
        iconShowRatingBox            = 0;
        rightClickActionComboBox     = 0;
        previewLoadFullImageSize     = 0;
        showFolderTreeViewItemsCount = 0;
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
    QCheckBox     *showFolderTreeViewItemsCount;

    QComboBox     *iconTreeThumbSize;
    QComboBox     *rightClickActionComboBox;
};

SetupAlbumView::SetupAlbumView(QWidget* parent)
              : QWidget(parent)
{
    d = new SetupAlbumViewPriv;

    QVBoxLayout *layout = new QVBoxLayout( this );

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

    d->showFolderTreeViewItemsCount = new QCheckBox(i18n("Show count of items in all tree-view"), interfaceOptionsGroup);

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
    ifaceSettingsLayout->addWidget(d->showFolderTreeViewItemsCount, 1, 0, 1, 4);
    ifaceSettingsLayout->addWidget(rightClickLabel, 2 , 0, 1, 1);
    ifaceSettingsLayout->addWidget(d->rightClickActionComboBox, 2, 1, 1, 4);
    ifaceSettingsLayout->addWidget(d->previewLoadFullImageSize, 3, 0, 1, 5 );

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
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
    settings->setShowFolderTreeViewItemsCount(d->showFolderTreeViewItemsCount->isChecked());
    settings->saveSettings();
}

void SetupAlbumView::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

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
    d->showFolderTreeViewItemsCount->setChecked(settings->getShowFolderTreeViewItemsCount());
}

}  // namespace Digikam
