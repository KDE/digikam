/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupalbumview.moc"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>

// Local includes

#include "albumsettings.h"
#include "dfontselect.h"

namespace Digikam
{

class SetupAlbumView::SetupAlbumViewPriv
{
public:

    SetupAlbumViewPriv() :
        iconTreeThumbLabel(0),
        iconShowNameBox(0),
        iconShowSizeBox(0),
        iconShowDateBox(0),
        iconShowModDateBox(0),
        iconShowResolutionBox(0),
        iconShowTitleBox(0),
        iconShowCommentsBox(0),
        iconShowTagsBox(0),
        iconShowOverlaysBox(0),
        iconShowRatingBox(0),
        iconShowFormatBox(0),
        previewLoadFullImageSize(0),
        previewShowIcons(0),
        showFolderTreeViewItemsCount(0),
        iconTreeThumbSize(0),
        leftClickActionComboBox(0),
        iconViewFontSelect(0),
        treeViewFontSelect(0)
    {
    }

    QLabel*      iconTreeThumbLabel;

    QCheckBox*   iconShowNameBox;
    QCheckBox*   iconShowSizeBox;
    QCheckBox*   iconShowDateBox;
    QCheckBox*   iconShowModDateBox;
    QCheckBox*   iconShowResolutionBox;
    QCheckBox*   iconShowTitleBox;
    QCheckBox*   iconShowCommentsBox;
    QCheckBox*   iconShowTagsBox;
    QCheckBox*   iconShowOverlaysBox;
    QCheckBox*   iconShowRatingBox;
    QCheckBox*   iconShowFormatBox;
    QCheckBox*   previewLoadFullImageSize;
    QCheckBox*   previewShowIcons;
    QCheckBox*   showFolderTreeViewItemsCount;

    KComboBox*   iconTreeThumbSize;
    KComboBox*   leftClickActionComboBox;

    DFontSelect* iconViewFontSelect;
    DFontSelect* treeViewFontSelect;
};

SetupAlbumView::SetupAlbumView(QWidget* const parent)
    : QScrollArea(parent), d(new SetupAlbumViewPriv)
{
    QWidget* panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* iconViewGroup = new QGroupBox(i18n("Icon-View Options"), panel);
    QGridLayout* grid        = new QGridLayout(iconViewGroup);

    d->iconShowNameBox       = new QCheckBox(i18n("Show file&name"), iconViewGroup);
    d->iconShowNameBox->setWhatsThis(i18n("Set this option to show the filename below the image thumbnail."));

    d->iconShowSizeBox       = new QCheckBox(i18n("Show file si&ze"), iconViewGroup);
    d->iconShowSizeBox->setWhatsThis(i18n("Set this option to show the file size below the image thumbnail."));

    d->iconShowDateBox       = new QCheckBox(i18n("Show camera creation &date"), iconViewGroup);
    d->iconShowDateBox->setWhatsThis(i18n("Set this option to show the camera creation date "
                                          "below the image thumbnail."));

    d->iconShowModDateBox    = new QCheckBox(i18n("Show file &modification date"), iconViewGroup);
    d->iconShowModDateBox->setWhatsThis(i18n("Set this option to show the file modification date "
                                             "below the image thumbnail."));

    d->iconShowResolutionBox = new QCheckBox(i18n("Show ima&ge dimensions"), iconViewGroup);
    d->iconShowResolutionBox->setWhatsThis(i18n("Set this option to show the image size in pixels "
                                                "below the image thumbnail."));

    d->iconShowFormatBox     = new QCheckBox(i18n("Show image Format"), iconViewGroup);
    d->iconShowFormatBox->setWhatsThis(i18n("Set this option to show image format over image thumbnail."));

    d->iconShowTitleBox      = new QCheckBox(i18n("Show digiKam tit&le"), iconViewGroup);
    d->iconShowTitleBox->setWhatsThis(i18n("Set this option to show the digiKam title "
                                           "below the image thumbnail."));

    d->iconShowCommentsBox   = new QCheckBox(i18n("Show digiKam &captions"), iconViewGroup);
    d->iconShowCommentsBox->setWhatsThis(i18n("Set this option to show the digiKam captions "
                                              "below the image thumbnail."));

    d->iconShowTagsBox       = new QCheckBox(i18n("Show digiKam &tags"), iconViewGroup);
    d->iconShowTagsBox->setWhatsThis(i18n("Set this option to show the digiKam tags "
                                          "below the image thumbnail."));

    d->iconShowRatingBox     = new QCheckBox(i18n("Show digiKam &rating"), iconViewGroup);
    d->iconShowRatingBox->setWhatsThis(i18n("Set this option to show the digiKam rating "
                                            "below the image thumbnail."));

    d->iconShowOverlaysBox   = new QCheckBox(i18n("Show rotation overlay buttons"), iconViewGroup);
    d->iconShowOverlaysBox->setWhatsThis(i18n("Set this option to show overlay buttons on "
                                              "the image thumbnail for image rotation."));

    QLabel* leftClickLabel     = new QLabel(i18n("Thumbnail click action:"), iconViewGroup);
    d->leftClickActionComboBox = new KComboBox(iconViewGroup);
    d->leftClickActionComboBox->addItem(i18n("Show embedded preview"), AlbumSettings::ShowPreview);
    d->leftClickActionComboBox->addItem(i18n("Start image editor"), AlbumSettings::StartEditor);
    d->leftClickActionComboBox->setToolTip(i18n("Choose what should happen when you click on a thumbnail."));

    d->iconViewFontSelect = new DFontSelect(i18n("Icon View font:"), panel);
    d->iconViewFontSelect->setToolTip(i18n("Select here the font used to display text in Icon Views."));

    grid->addWidget(d->iconShowNameBox,          0, 0, 1, 1);
    grid->addWidget(d->iconShowSizeBox,          1, 0, 1, 1);
    grid->addWidget(d->iconShowDateBox,          2, 0, 1, 1);
    grid->addWidget(d->iconShowModDateBox,       3, 0, 1, 1);
    grid->addWidget(d->iconShowResolutionBox,    4, 0, 1, 1);
    grid->addWidget(d->iconShowFormatBox,        5, 0, 1, 1);

    grid->addWidget(d->iconShowTitleBox,         0, 1, 1, 1);
    grid->addWidget(d->iconShowCommentsBox,      1, 1, 1, 1);
    grid->addWidget(d->iconShowTagsBox,          2, 1, 1, 1);
    grid->addWidget(d->iconShowRatingBox,        3, 1, 1, 1);
    grid->addWidget(d->iconShowOverlaysBox,      4, 1, 1, 1);

    grid->addWidget(leftClickLabel,              6, 0, 1, 1);
    grid->addWidget(d->leftClickActionComboBox,  6, 1, 1, 1);
    grid->addWidget(d->iconViewFontSelect,       7, 0, 1, 2);
    grid->setSpacing(KDialog::spacingHint());
    grid->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox* folderViewGroup = new QGroupBox(i18n("Folder View Options"), panel);
    QGridLayout* grid2         = new QGridLayout(folderViewGroup);

    d->iconTreeThumbLabel      = new QLabel(i18n("Tree View thumbnail size:"), folderViewGroup);
    d->iconTreeThumbSize       = new KComboBox(folderViewGroup);
    d->iconTreeThumbSize->addItem(QString("16"));
    d->iconTreeThumbSize->addItem(QString("22"));
    d->iconTreeThumbSize->addItem(QString("32"));
    d->iconTreeThumbSize->addItem(QString("48"));
    d->iconTreeThumbSize->setToolTip(i18n("Set this option to configure the size in pixels of "
                                          "the Tree View thumbnails in digiKam's sidebars."));

    d->treeViewFontSelect = new DFontSelect(i18n("Tree View font:"), folderViewGroup);
    d->treeViewFontSelect->setToolTip(i18n("Select here the font used to display text in Tree Views."));

    d->showFolderTreeViewItemsCount = new QCheckBox(i18n("Show a count of items in Tree Views"), folderViewGroup);

    grid2->addWidget(d->iconTreeThumbLabel,           0, 0, 1, 1);
    grid2->addWidget(d->iconTreeThumbSize,            0, 1, 1, 1);
    grid2->addWidget(d->treeViewFontSelect,           1, 0, 1, 2);
    grid2->addWidget(d->showFolderTreeViewItemsCount, 2, 0, 1, 2);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox* interfaceOptionsGroup = new QGroupBox(i18n("Preview Options"), panel);
    QGridLayout* grid3               = new QGridLayout(interfaceOptionsGroup);

    d->previewLoadFullImageSize      = new QCheckBox(i18n("Embedded preview loads full-sized images"), interfaceOptionsGroup);
    d->previewLoadFullImageSize->setWhatsThis(i18n("<p>Set this option to load images at their full size "
                                                   "for preview, rather than at a reduced size. As this option "
                                                   "will make it take longer to load images, only use it if you have "
                                                   "a fast computer.</p>"
                                                   "<p><b>Note:</b> for Raw images, a half size version of the Raw data "
                                                   "is used instead of the embedded JPEG preview.</p>"));

    d->previewShowIcons              = new QCheckBox(i18n("Show icons and text over preview"), interfaceOptionsGroup);
    d->previewShowIcons->setWhatsThis(i18n("Uncheck this if you don't want to see icons and text in the image preview."));

    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(KDialog::spacingHint());
    grid3->addWidget(d->previewLoadFullImageSize, 0, 0, 1, 2);
    grid3->addWidget(d->previewShowIcons,         1, 0, 1, 2);

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(iconViewGroup);
    layout->addWidget(folderViewGroup);
    layout->addWidget(interfaceOptionsGroup);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
    adjustSize();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupAlbumView::~SetupAlbumView()
{
    delete d;
}

void SetupAlbumView::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setTreeViewIconSize(d->iconTreeThumbSize->currentText().toInt());
    settings->setTreeViewFont(d->treeViewFontSelect->font());
    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    settings->setIconShowModDate(d->iconShowModDateBox->isChecked());
    settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowTitle(d->iconShowTitleBox->isChecked());
    settings->setIconShowComments(d->iconShowCommentsBox->isChecked());
    settings->setIconShowOverlays(d->iconShowOverlaysBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());
    settings->setIconShowImageFormat(d->iconShowFormatBox->isChecked());
    settings->setIconViewFont(d->iconViewFontSelect->font());

    settings->setItemLeftClickAction((AlbumSettings::ItemLeftClickAction)
                                     d->leftClickActionComboBox->currentIndex());

    settings->setPreviewLoadFullImageSize(d->previewLoadFullImageSize->isChecked());
    settings->setPreviewShowIcons(d->previewShowIcons->isChecked());
    settings->setShowFolderTreeViewItemsCount(d->showFolderTreeViewItemsCount->isChecked());
    settings->saveSettings();
}

void SetupAlbumView::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    if (settings->getTreeViewIconSize() == 16)
    {
        d->iconTreeThumbSize->setCurrentIndex(0);
    }
    else if (settings->getTreeViewIconSize() == 22)
    {
        d->iconTreeThumbSize->setCurrentIndex(1);
    }
    else if (settings->getTreeViewIconSize() == 32)
    {
        d->iconTreeThumbSize->setCurrentIndex(2);
    }
    else
    {
        d->iconTreeThumbSize->setCurrentIndex(3);
    }

    d->treeViewFontSelect->setFont(settings->getTreeViewFont());
    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    d->iconShowModDateBox->setChecked(settings->getIconShowModDate());
    d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowTitleBox->setChecked(settings->getIconShowTitle());
    d->iconShowCommentsBox->setChecked(settings->getIconShowComments());
    d->iconShowOverlaysBox->setChecked(settings->getIconShowOverlays());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());
    d->iconShowFormatBox->setChecked(settings->getIconShowImageFormat());
    d->iconViewFontSelect->setFont(settings->getIconViewFont());

    d->leftClickActionComboBox->setCurrentIndex((int)settings->getItemLeftClickAction());

    d->previewLoadFullImageSize->setChecked(settings->getPreviewLoadFullImageSize());
    d->previewShowIcons->setChecked(settings->getPreviewShowIcons());
    d->showFolderTreeViewItemsCount->setChecked(settings->getShowFolderTreeViewItemsCount());
}

}  // namespace Digikam
