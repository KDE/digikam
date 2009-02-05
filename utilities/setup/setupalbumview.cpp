/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupalbumview.h"
#include "setupalbumview.moc"

// Qt includes.

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

// KDE includes.

#include <kcombobox.h>
#include <kdialog.h>
#include <kapplication.h>
#include <klocale.h>

// Local includes.

#include "albumsettings.h"
#include "dfontselect.h"

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
        treeViewFontSelect           = 0;
        iconViewFontSelect           = 0;
        sidebarTypeLabel             = 0;
        sidebarType                  = 0;
    }

    QLabel      *iconTreeThumbLabel;
    QLabel      *sidebarTypeLabel;

    QCheckBox   *iconShowNameBox;
    QCheckBox   *iconShowSizeBox;
    QCheckBox   *iconShowDateBox;
    QCheckBox   *iconShowModDateBox;
    QCheckBox   *iconShowResolutionBox;
    QCheckBox   *iconShowCommentsBox;
    QCheckBox   *iconShowTagsBox;
    QCheckBox   *iconShowRatingBox;
    QCheckBox   *previewLoadFullImageSize;
    QCheckBox   *showFolderTreeViewItemsCount;

    KComboBox   *sidebarType;
    KComboBox   *iconTreeThumbSize;
    KComboBox   *rightClickActionComboBox;

    DFontSelect *iconViewFontSelect;
    DFontSelect *treeViewFontSelect;
};

SetupAlbumView::SetupAlbumView(QWidget* parent)
              : QScrollArea(parent), d(new SetupAlbumViewPriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox *iconViewGroup = new QGroupBox(i18n("Icon-View Options"), panel);
    QGridLayout *grid        = new QGridLayout(iconViewGroup);

    d->iconShowNameBox       = new QCheckBox(i18n("Show file &name"), iconViewGroup);
    d->iconShowNameBox->setWhatsThis(i18n("Set this option to show the file name below the image thumbnail."));

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

    d->iconShowCommentsBox   = new QCheckBox(i18n("Show digiKam &captions"), iconViewGroup);
    d->iconShowCommentsBox->setWhatsThis(i18n("Set this option to show the digiKam captions "
                                              "below the image thumbnail."));

    d->iconShowTagsBox       = new QCheckBox(i18n("Show digiKam &tags"), iconViewGroup);
    d->iconShowTagsBox->setWhatsThis(i18n("Set this option to show the digiKam tags "
                                          "below the image thumbnail."));

    d->iconShowRatingBox     = new QCheckBox(i18n("Show digiKam &rating"), iconViewGroup);
    d->iconShowRatingBox->setWhatsThis(i18n("Set this option to show the digiKam rating "
                                            "below the image thumbnail."));

    QLabel *rightClickLabel     = new QLabel(i18n("Thumbnail click action:"), iconViewGroup);
    d->rightClickActionComboBox = new KComboBox(iconViewGroup);
    d->rightClickActionComboBox->addItem(i18n("Show embedded preview"), AlbumSettings::ShowPreview);
    d->rightClickActionComboBox->addItem(i18n("Start image editor"), AlbumSettings::StartEditor);
    d->rightClickActionComboBox->setToolTip(i18n("Here, choose what should happen when you "
                                                 "click on a thumbnail."));

    d->iconViewFontSelect = new DFontSelect(i18n("Icon-view font:"), panel);
    d->iconViewFontSelect->setToolTip(i18n("Select here the font used to display text in all icon-view."));

    grid->addWidget(d->iconShowNameBox,          0, 0, 1, 1);
    grid->addWidget(d->iconShowSizeBox,          1, 0, 1, 1);
    grid->addWidget(d->iconShowDateBox,          2, 0, 1, 1);
    grid->addWidget(d->iconShowModDateBox,       3, 0, 1, 1);
    grid->addWidget(d->iconShowResolutionBox,    4, 0, 1, 1);
    grid->addWidget(d->iconShowCommentsBox,      0, 1, 1, 1);
    grid->addWidget(d->iconShowTagsBox,          1, 1, 1, 1);
    grid->addWidget(d->iconShowRatingBox,        2, 1, 1, 1);
    grid->addWidget(rightClickLabel,             5, 0, 1, 1);
    grid->addWidget(d->rightClickActionComboBox, 5, 1, 1, 1);
    grid->addWidget(d->iconViewFontSelect,       6, 0, 1, 2);
    grid->setSpacing(KDialog::spacingHint());
    grid->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox *folderViewGroup = new QGroupBox(i18n("Folder-View Options"), panel);
    QGridLayout* grid2         = new QGridLayout(folderViewGroup);

    d->iconTreeThumbLabel      = new QLabel(i18n("Tree-view thumbnail size:"), folderViewGroup);
    d->iconTreeThumbSize       = new KComboBox(folderViewGroup);
    d->iconTreeThumbSize->addItem(QString("16"));
    d->iconTreeThumbSize->addItem(QString("22"));
    d->iconTreeThumbSize->addItem(QString("32"));
    d->iconTreeThumbSize->addItem(QString("48"));
    d->iconTreeThumbSize->setToolTip(i18n("Set this option to configure the size in pixels of "
                                          "the tree-view thumbnails in digiKam's sidebars."));

    d->treeViewFontSelect = new DFontSelect(i18n("Tree-view font:"), folderViewGroup);
    d->treeViewFontSelect->setToolTip(i18n("Select here the font used to display text in all tree-view."));

    d->showFolderTreeViewItemsCount = new QCheckBox(i18n("Show count of items in all tree-view"), folderViewGroup);

    grid2->addWidget(d->iconTreeThumbLabel,           0, 0, 1, 1);
    grid2->addWidget(d->iconTreeThumbSize,            0, 1, 1, 1);
    grid2->addWidget(d->treeViewFontSelect,           1, 0, 1, 2);
    grid2->addWidget(d->showFolderTreeViewItemsCount, 2, 0, 1, 2);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox *interfaceOptionsGroup = new QGroupBox(i18n("Misc Options"), panel);
    QGridLayout* grid3               = new QGridLayout(interfaceOptionsGroup);

    d->previewLoadFullImageSize      = new QCheckBox(i18n("Embedded preview loads full image size"), interfaceOptionsGroup);
    d->previewLoadFullImageSize->setWhatsThis(i18n("Set this option to load the full image size "
                                                   "for a preview, instead of with a reduced size. This loads more "
                                                   "data and will be slow on older computers."));

    d->sidebarTypeLabel  = new QLabel(i18n("Sidebar tab title:"), interfaceOptionsGroup);
    d->sidebarType       = new KComboBox(interfaceOptionsGroup);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebars tab title are visible."));

    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(KDialog::spacingHint());
    grid3->addWidget(d->previewLoadFullImageSize, 0, 0, 1, 2);
    grid3->addWidget(d->sidebarTypeLabel,         1, 0, 1, 1);
    grid3->addWidget(d->sidebarType,              1, 1, 1, 1);

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
}

SetupAlbumView::~SetupAlbumView()
{
    delete d;
}

void SetupAlbumView::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setTreeViewIconSize(d->iconTreeThumbSize->currentText().toInt());
    settings->setTreeViewFont(d->treeViewFontSelect->font());
    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    settings->setIconShowModDate(d->iconShowModDateBox->isChecked());
    settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowComments(d->iconShowCommentsBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());
    settings->setIconViewFont(d->iconViewFontSelect->font());

    settings->setItemRightClickAction((AlbumSettings::ItemRightClickAction)
                                      d->rightClickActionComboBox->currentIndex());

    settings->setPreviewLoadFullImageSize(d->previewLoadFullImageSize->isChecked());
    settings->setShowFolderTreeViewItemsCount(d->showFolderTreeViewItemsCount->isChecked());
    settings->setSidebarTitleStyle(d->sidebarType->currentIndex() == 0 ? KMultiTabBar::VSNET : KMultiTabBar::KDEV3ICON);
    settings->saveSettings();
}

void SetupAlbumView::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    if (settings->getTreeViewIconSize() == 16)
        d->iconTreeThumbSize->setCurrentIndex(0);
    else if (settings->getTreeViewIconSize() == 22)
        d->iconTreeThumbSize->setCurrentIndex(1);
    else if (settings->getTreeViewIconSize() == 32)
        d->iconTreeThumbSize->setCurrentIndex(2);
    else
        d->iconTreeThumbSize->setCurrentIndex(3);

    d->treeViewFontSelect->setFont(settings->getTreeViewFont());
    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    d->iconShowModDateBox->setChecked(settings->getIconShowModDate());
    d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowCommentsBox->setChecked(settings->getIconShowComments());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());
    d->iconViewFontSelect->setFont(settings->getIconViewFont());

    d->rightClickActionComboBox->setCurrentIndex((int)settings->getItemRightClickAction());

    d->previewLoadFullImageSize->setChecked(settings->getPreviewLoadFullImageSize());
    d->showFolderTreeViewItemsCount->setChecked(settings->getShowFolderTreeViewItemsCount());
    d->sidebarType->setCurrentIndex(settings->getSidebarTitleStyle() == KMultiTabBar::VSNET ? 0 : 1);
}

}  // namespace Digikam
