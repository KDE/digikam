/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : A widget to select Physical or virtual albums with combo-box
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumselectors.moc"

// Qt includes

#include <QApplication>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
<<<<<<< HEAD
<<<<<<< HEAD
#include <QCheckBox>
=======
>>>>>>> master
=======
#include <QCheckBox>
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kstandardguiitem.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <kconfig.h>
=======
>>>>>>> master
=======
#include <kconfig.h>
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d

// Local includes

#include "albummodel.h"
<<<<<<< HEAD
<<<<<<< HEAD
#include "albummanager.h"
=======
>>>>>>> master
=======
#include "albummanager.h"
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
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
        setPixmap(SmallIcon(qApp->isLeftToRight() ? "edit-clear-locationbar-rtl" : "edit-clear-locationbar-ltr",
                            0, KIconLoader::DefaultState));
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
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
        wholePalbums     = 0;
        wholeTalbums     = 0;
        titleLabel       = 0;
    }

    static const QString         configUseWholePAlbumsEntry;
    static const QString         configUseWholeTAlbumsEntry;
    
    QString                      configName;
    QCheckBox*                   wholePalbums;
    QCheckBox*                   wholeTalbums;
    QLabel*                      titleLabel;
<<<<<<< HEAD
=======
    }

    QString                      configName;
>>>>>>> master
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d

    AlbumTreeViewSelectComboBox* albumSelectCB;
    TagTreeViewSelectComboBox*   tagSelectCB;
    ModelClearButton*            albumClearButton;
    ModelClearButton*            tagClearButton;
};

<<<<<<< HEAD
<<<<<<< HEAD
const QString AlbumSelectors::Private::configUseWholePAlbumsEntry("UseWholePAlbumsEntry");
const QString AlbumSelectors::Private::configUseWholeTAlbumsEntry("UseWholeTAlbumsEntry");

=======
>>>>>>> master
=======
const QString AlbumSelectors::Private::configUseWholePAlbumsEntry("UseWholePAlbumsEntry");
const QString AlbumSelectors::Private::configUseWholeTAlbumsEntry("UseWholeTAlbumsEntry");

>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
AlbumSelectors::AlbumSelectors(const QString& label, const QString& configName, QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    d->configName                         = configName;
    QGridLayout* const selectAlbumsLayout = new QGridLayout(this);
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
    d->titleLabel                         = new QLabel(label);

    d->wholePalbums     = new QCheckBox(i18n("Whole albums collection"), this);
    d->albumSelectCB    = new AlbumTreeViewSelectComboBox(this);
<<<<<<< HEAD
=======
    QLabel* const includeAlbumsLabel      = new QLabel(label);
    d->albumSelectCB                      = new AlbumTreeViewSelectComboBox(this);
>>>>>>> master
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
    //d->albumSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->albumSelectCB->setToolTip(i18nc("@info:tooltip", "Select all albums that should be processed."));
    d->albumSelectCB->setDefaultModel();
    d->albumSelectCB->setNoSelectionText(i18nc("@info:status", "Any albums"));
    d->albumSelectCB->addCheckUncheckContextMenuActions();

    d->albumClearButton = new ModelClearButton(d->albumSelectCB->view()->albumModel());
    d->albumClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected albums"));

<<<<<<< HEAD
<<<<<<< HEAD
    d->wholeTalbums     = new QCheckBox(i18n("Whole tags collection"), this);
=======
>>>>>>> master
=======
    d->wholeTalbums     = new QCheckBox(i18n("Whole tags collection"), this);
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
    d->tagSelectCB      = new TagTreeViewSelectComboBox(this);
    //d->tagSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->tagSelectCB->setToolTip(i18nc("@info:tooltip", "Select all tags that should be processed."));
    d->tagSelectCB->setDefaultModel();
    d->tagSelectCB->setNoSelectionText(i18nc("@info:status", "Any tags"));
    d->tagSelectCB->addCheckUncheckContextMenuActions();

    d->tagClearButton   = new ModelClearButton(d->tagSelectCB->view()->albumModel());
    d->tagClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected tags"));

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
    selectAlbumsLayout->addWidget(d->titleLabel,       0, 0, 1, 2);
    selectAlbumsLayout->addWidget(d->wholePalbums,     1, 0, 1, 2);
    selectAlbumsLayout->addWidget(d->albumSelectCB,    2, 0);
    selectAlbumsLayout->addWidget(d->albumClearButton, 2, 1);
    selectAlbumsLayout->addWidget(d->wholeTalbums,     3, 0, 1, 2);
    selectAlbumsLayout->addWidget(d->tagSelectCB,      4, 0);
    selectAlbumsLayout->addWidget(d->tagClearButton,   4, 1);
    selectAlbumsLayout->setRowStretch(5, 1);

    connect(d->wholePalbums, SIGNAL(toggled(bool)),
            this, SLOT(slotWholePalbums(bool)));

    connect(d->wholeTalbums, SIGNAL(toggled(bool)),
            this, SLOT(slotWholeTalbums(bool)));
    
<<<<<<< HEAD
=======
    selectAlbumsLayout->addWidget(includeAlbumsLabel,  0, 0, 1, 2);
    selectAlbumsLayout->addWidget(d->albumSelectCB,    1, 0);
    selectAlbumsLayout->addWidget(d->albumClearButton, 1, 1);
    selectAlbumsLayout->addWidget(d->tagSelectCB,      2, 0);
    selectAlbumsLayout->addWidget(d->tagClearButton,   2, 1);
    selectAlbumsLayout->setRowStretch(3, 1);

>>>>>>> master
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
    connect(d->albumSelectCB->view()->albumModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotUpdateClearButtons()));

    connect(d->tagSelectCB->view()->albumModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotUpdateClearButtons()));

    setObjectName(d->configName);
    d->albumSelectCB->view()->setObjectName(d->configName);
    d->albumSelectCB->view()->setEntryPrefix("AlbumComboBox-");
    d->albumSelectCB->view()->setRestoreCheckState(true);
    d->tagSelectCB->view()->setObjectName(d->configName);
    d->tagSelectCB->view()->setEntryPrefix("TagComboBox-");
    d->tagSelectCB->view()->setRestoreCheckState(true);
<<<<<<< HEAD
<<<<<<< HEAD
=======
    loadState();

    slotUpdateClearButtons();
>>>>>>> master
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
}

AlbumSelectors::~AlbumSelectors()
{
    delete d;
}

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
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

<<<<<<< HEAD
=======
>>>>>>> master
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
void AlbumSelectors::slotUpdateClearButtons()
{
    d->albumClearButton->animateVisible(!d->albumSelectCB->model()->checkedAlbums().isEmpty());
    d->tagClearButton->animateVisible(!d->tagSelectCB->model()->checkedAlbums().isEmpty());
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d

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
    
    if (wholeAlbumsCollection())
    {
        albums << AlbumManager::instance()->allPAlbums();
    }
    else
    {
        albums << d->albumSelectCB->model()->checkedAlbums();
    }
    
    return albums;
<<<<<<< HEAD
}

bool AlbumSelectors::wholeTagsCollection() const
{
    return d->wholeTalbums->isChecked();
}

AlbumList AlbumSelectors::selectedTAlbums() const
{
    AlbumList albums;
    
    if (wholeTagsCollection())
    {
        albums << AlbumManager::instance()->allTAlbums();
    }
    else
    {
        albums << d->tagSelectCB->model()->checkedAlbums();
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configName);
    d->wholePalbums->setChecked(group.readEntry(d->configUseWholePAlbumsEntry, true));    
    d->wholeTalbums->setChecked(group.readEntry(d->configUseWholeTAlbumsEntry, true));    
    
    d->albumSelectCB->view()->loadState();
    d->tagSelectCB->view()->loadState();
    
    slotUpdateClearButtons();

    slotWholePalbums(wholeAlbumsCollection());
    slotWholeTalbums(wholeTagsCollection());
=======
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
}

bool AlbumSelectors::wholeTagsCollection() const
{
    return d->wholeTalbums->isChecked();
}

AlbumList AlbumSelectors::selectedTAlbums() const
{
    AlbumList albums;
    
    if (wholeTagsCollection())
    {
        albums << AlbumManager::instance()->allTAlbums();
    }
    else
    {
        albums << d->tagSelectCB->model()->checkedAlbums();
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configName);
    d->wholePalbums->setChecked(group.readEntry(d->configUseWholePAlbumsEntry, true));    
    d->wholeTalbums->setChecked(group.readEntry(d->configUseWholeTAlbumsEntry, true));    
    
    d->albumSelectCB->view()->loadState();
    d->tagSelectCB->view()->loadState();
<<<<<<< HEAD
>>>>>>> master
=======
    
    slotUpdateClearButtons();

    slotWholePalbums(wholeAlbumsCollection());
    slotWholeTalbums(wholeTagsCollection());
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
}

void AlbumSelectors::saveState()
{
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configName);
    group.writeEntry(d->configUseWholePAlbumsEntry, wholeAlbumsCollection());
    group.writeEntry(d->configUseWholeTAlbumsEntry, wholeTagsCollection());
    
<<<<<<< HEAD
=======
>>>>>>> master
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
    d->albumSelectCB->view()->saveState();
    d->tagSelectCB->view()->saveState();
}

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
void AlbumSelectors::resetSelection()
{
    d->albumSelectCB->model()->resetCheckedAlbums();
    d->tagSelectCB->model()->resetCheckedAlbums();
}

<<<<<<< HEAD
=======
>>>>>>> master
=======
>>>>>>> 606db1c6a50ab5644f6870d6050f9bb49911197d
} // namespace Digikam
