/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : A widget to select Physical or virtual albums with combo-box
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumselectors.h"

// Qt includes

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummodel.h"
#include "albummanager.h"
#include "albumselectcombobox.h"
#include "albumtreeview.h"
#include "searchutilities.h"

namespace Digikam
{

class ModelClearButton : public AnimatedClearButton
{
public:

    explicit ModelClearButton(AbstractCheckableAlbumModel* const model)
    {
        setPixmap(QIcon::fromTheme(qApp->isLeftToRight() ? QLatin1String("edit-clear-locationbar-rtl")
                                                         : QLatin1String("edit-clear-locationbar-ltr")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
        stayVisibleWhenAnimatedOut(true);

        connect(this, SIGNAL(clicked()),
                model, SLOT(resetAllCheckedAlbums()));
    }
};

// ------------------------------------------------------------------------------------------

class AlbumSelectors::Private
{
public:

    Private()
    {
        albumSelectCB    = 0;
        tagSelectCB      = 0;
        albumClearButton = 0;
        tagClearButton   = 0;
        wholePalbums     = 0;
        wholeTalbums     = 0;
        titleLabel       = 0;
        pAlbumsBtn       = 0;
        tAlbumsBtn       = 0;
        btnGroup         = 0;
        pAlbumsBox       = 0;
        tAlbumsBox       = 0;
    }

    static const QString         configUseWholePAlbumsEntry;
    static const QString         configUseWholeTAlbumsEntry;
    static const QString         configAlbumTypeEntry;

    QString                      configName;
    QCheckBox*                   wholePalbums;
    QCheckBox*                   wholeTalbums;
    QLabel*                      titleLabel;

    QGroupBox*                   pAlbumsBox;
    QGroupBox*                   tAlbumsBox;
    QRadioButton*                pAlbumsBtn;
    QRadioButton*                tAlbumsBtn;
    QButtonGroup*                btnGroup;

    AlbumTreeViewSelectComboBox* albumSelectCB;
    TagTreeViewSelectComboBox*   tagSelectCB;
    ModelClearButton*            albumClearButton;
    ModelClearButton*            tagClearButton;
};

const QString AlbumSelectors::Private::configUseWholePAlbumsEntry(QLatin1String("UseWholePAlbumsEntry"));
const QString AlbumSelectors::Private::configUseWholeTAlbumsEntry(QLatin1String("UseWholeTAlbumsEntry"));
const QString AlbumSelectors::Private::configAlbumTypeEntry(QLatin1String("AlbumTypeEntry"));

AlbumSelectors::AlbumSelectors(const QString& label, const QString& configName, QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    d->configName           = configName;
    QGridLayout* const grid = new QGridLayout(this);
    d->titleLabel           = new QLabel(label, this);

    // ---------------

    d->pAlbumsBtn       = new QRadioButton(this);
    d->pAlbumsBox       = new QGroupBox(i18n("Albums"), this);

    d->wholePalbums     = new QCheckBox(i18n("Whole albums collection"), d->pAlbumsBox);
    d->albumSelectCB    = new AlbumTreeViewSelectComboBox(d->pAlbumsBox);
    d->albumSelectCB->setToolTip(i18nc("@info:tooltip", "Select all albums that should be processed."));
    d->albumSelectCB->setDefaultModel();
    d->albumSelectCB->setNoSelectionText(i18nc("@info:status", "Any albums"));
    d->albumSelectCB->addCheckUncheckContextMenuActions();

    d->albumClearButton = new ModelClearButton(d->albumSelectCB->view()->albumModel());
    d->albumClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected albums"));

    QGridLayout* const pAlbumsGrid = new QGridLayout(d->pAlbumsBox);
    pAlbumsGrid->addWidget(d->wholePalbums,     0, 0, 1, 2);
    pAlbumsGrid->addWidget(d->albumSelectCB,    1, 0, 1, 1);
    pAlbumsGrid->addWidget(d->albumClearButton, 1, 1, 1, 1);
    pAlbumsGrid->setSpacing(0);
    d->pAlbumsBox->setLayout(pAlbumsGrid);

    // ---------------

    d->tAlbumsBtn       = new QRadioButton(this);
    d->tAlbumsBox       = new QGroupBox(i18n("Tags"), this);

    d->wholeTalbums     = new QCheckBox(i18n("Whole tags collection"), d->tAlbumsBox);
    d->tagSelectCB      = new TagTreeViewSelectComboBox(d->tAlbumsBox);
    d->tagSelectCB->setToolTip(i18nc("@info:tooltip", "Select all tags that should be processed."));
    d->tagSelectCB->setDefaultModel();
    d->tagSelectCB->setNoSelectionText(i18nc("@info:status", "Any tags"));
    d->tagSelectCB->addCheckUncheckContextMenuActions();

    d->tagClearButton   = new ModelClearButton(d->tagSelectCB->view()->albumModel());
    d->tagClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected tags"));

    QGridLayout* const tAlbumsGrid = new QGridLayout(d->tAlbumsBox);
    tAlbumsGrid->addWidget(d->wholeTalbums,   0, 0, 1, 2);
    tAlbumsGrid->addWidget(d->tagSelectCB,    1, 0, 1, 1);
    tAlbumsGrid->addWidget(d->tagClearButton, 1, 1, 1, 1);
    tAlbumsGrid->setSpacing(0);
    d->tAlbumsBox->setLayout(tAlbumsGrid);

    // ---------------

    d->btnGroup         = new QButtonGroup(this);
    d->btnGroup->addButton(d->pAlbumsBtn);
    d->btnGroup->addButton(d->tAlbumsBtn);
    d->btnGroup->setId(d->pAlbumsBtn, PhysAlbum);
    d->btnGroup->setId(d->tAlbumsBtn, TagsAlbum);
    d->btnGroup->setExclusive(true);

    // ---------------

    grid->addWidget(d->titleLabel, 0, 0, 1, 2);
    grid->addWidget(d->pAlbumsBtn, 1, 0, 1, 1);
    grid->addWidget(d->pAlbumsBox, 1, 1, 1, 1);
    grid->addWidget(d->tAlbumsBtn, 2, 0, 1, 1);
    grid->addWidget(d->tAlbumsBox, 2, 1, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setAlignment(d->pAlbumsBtn, Qt::AlignTop);
    grid->setAlignment(d->tAlbumsBtn, Qt::AlignTop);

    // ---------------

    connect(d->wholePalbums, SIGNAL(toggled(bool)),
            this, SLOT(slotWholePalbums(bool)));

    connect(d->wholeTalbums, SIGNAL(toggled(bool)),
            this, SLOT(slotWholeTalbums(bool)));

    connect(d->albumSelectCB->view()->albumModel(), SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SLOT(slotUpdateClearButtons()));

    connect(d->tagSelectCB->view()->albumModel(), SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SLOT(slotUpdateClearButtons()));

    connect(d->btnGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotToggleTypeSelection(int)));

    setObjectName(d->configName);
    d->albumSelectCB->view()->setObjectName(d->configName);
    d->albumSelectCB->view()->setEntryPrefix(QLatin1String("AlbumComboBox-"));
    d->albumSelectCB->view()->setRestoreCheckState(true);
    d->tagSelectCB->view()->setObjectName(d->configName);
    d->tagSelectCB->view()->setEntryPrefix(QLatin1String("TagComboBox-"));
    d->tagSelectCB->view()->setRestoreCheckState(true);
}

AlbumSelectors::~AlbumSelectors()
{
    delete d;
}

void AlbumSelectors::slotWholePalbums(bool b)
{
    d->albumSelectCB->setEnabled(!b);
    d->albumClearButton->setEnabled(!b);
}

void AlbumSelectors::slotWholeTalbums(bool b)
{
    d->tagSelectCB->setEnabled(!b);
    d->tagClearButton->setEnabled(!b);
}

void AlbumSelectors::slotUpdateClearButtons()
{
    d->albumClearButton->animateVisible(!d->albumSelectCB->model()->checkedAlbums().isEmpty());
    d->tagClearButton->animateVisible(!d->tagSelectCB->model()->checkedAlbums().isEmpty());

    emit signalSelectionChanged();
}

AlbumList AlbumSelectors::selectedAlbums() const
{
    AlbumList albums;
    albums << selectedPAlbums();
    albums << selectedTAlbums();
    return albums;
}

bool AlbumSelectors::wholeAlbumsCollection() const
{
    return d->wholePalbums->isChecked();
}

AlbumList AlbumSelectors::selectedPAlbums() const
{
    AlbumList albums;

    if (d->btnGroup->checkedId() == PhysAlbum)
    {
        if (wholeAlbumsCollection())
        {
            albums << AlbumManager::instance()->allPAlbums();
        }
        else
        {
            albums << d->albumSelectCB->model()->checkedAlbums();
        }
    }

    return albums;
}

bool AlbumSelectors::wholeTagsCollection() const
{
    return d->wholeTalbums->isChecked();
}

AlbumList AlbumSelectors::selectedTAlbums() const
{
    AlbumList albums;

    if (d->btnGroup->checkedId() == TagsAlbum)
    {
        if (wholeTagsCollection())
        {
            albums << AlbumManager::instance()->allTAlbums();
        }
        else
        {
            albums << d->tagSelectCB->model()->checkedAlbums();
        }
    }

    return albums;
}

void AlbumSelectors::setPAlbumSelected(Album* const album, bool singleSelection)
{
    if (!album)
        return;

    if (singleSelection)
        d->albumSelectCB->model()->resetCheckedAlbums();

    d->albumSelectCB->model()->setChecked(album, true);
    d->wholePalbums->setChecked(false);
}

void AlbumSelectors::setTAlbumSelected(Album* const album, bool singleSelection)
{
    if (!album)
        return;

    if (singleSelection)
        d->tagSelectCB->model()->resetCheckedAlbums();

    d->tagSelectCB->model()->setChecked(album, true);
    d->wholeTalbums->setChecked(false);
}

void AlbumSelectors::loadState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configName);
    d->wholePalbums->setChecked(group.readEntry(d->configUseWholePAlbumsEntry, true));
    d->wholeTalbums->setChecked(group.readEntry(d->configUseWholeTAlbumsEntry, true));
    d->btnGroup->button(group.readEntry(d->configAlbumTypeEntry, (int)PhysAlbum))->setChecked(true);

    d->albumSelectCB->view()->loadState();
    d->tagSelectCB->view()->loadState();

    slotToggleTypeSelection(d->btnGroup->checkedId());
    slotUpdateClearButtons();

    slotWholePalbums(wholeAlbumsCollection());
    slotWholeTalbums(wholeTagsCollection());
}

void AlbumSelectors::saveState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configName);
    group.writeEntry(d->configUseWholePAlbumsEntry, wholeAlbumsCollection());
    group.writeEntry(d->configUseWholeTAlbumsEntry, wholeTagsCollection());
    group.writeEntry(d->configAlbumTypeEntry,       d->btnGroup->checkedId());

    d->albumSelectCB->view()->saveState();
    d->tagSelectCB->view()->saveState();
}

void AlbumSelectors::resetSelection()
{
    d->albumSelectCB->model()->resetCheckedAlbums();
    d->tagSelectCB->model()->resetCheckedAlbums();
}

void AlbumSelectors::slotToggleTypeSelection(int type)
{
    switch (type)
    {
        case PhysAlbum:
            d->pAlbumsBox->setEnabled(true);
            d->tAlbumsBox->setEnabled(false);
            slotWholePalbums(d->wholePalbums->isChecked());
            break;
        default: // TagsAlbum
            d->pAlbumsBox->setEnabled(false);
            d->tAlbumsBox->setEnabled(true);
            slotWholeTalbums(d->wholeTalbums->isChecked());
            break;
    }
}

} // namespace Digikam
