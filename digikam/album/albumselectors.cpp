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

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kstandardguiitem.h>

// Local includes

#include "albummodel.h"
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
    }

    QString                      configName;

    AlbumTreeViewSelectComboBox* albumSelectCB;
    TagTreeViewSelectComboBox*   tagSelectCB;
    ModelClearButton*            albumClearButton;
    ModelClearButton*            tagClearButton;
};

AlbumSelectors::AlbumSelectors(const QString& label, const QString& configName, QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    d->configName                         = configName;
    QGridLayout* const selectAlbumsLayout = new QGridLayout(this);
    QLabel* const includeAlbumsLabel      = new QLabel(label);
    d->albumSelectCB                      = new AlbumTreeViewSelectComboBox(this);
    //d->albumSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->albumSelectCB->setToolTip(i18nc("@info:tooltip", "Select all albums that should be processed."));
    d->albumSelectCB->setDefaultModel();
    d->albumSelectCB->setNoSelectionText(i18nc("@info:status", "Any albums"));
    d->albumSelectCB->addCheckUncheckContextMenuActions();

    d->albumClearButton = new ModelClearButton(d->albumSelectCB->view()->albumModel());
    d->albumClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected albums"));

    d->tagSelectCB      = new TagTreeViewSelectComboBox(this);
    //d->tagSelectCB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->tagSelectCB->setToolTip(i18nc("@info:tooltip", "Select all tags that should be processed."));
    d->tagSelectCB->setDefaultModel();
    d->tagSelectCB->setNoSelectionText(i18nc("@info:status", "Any tags"));
    d->tagSelectCB->addCheckUncheckContextMenuActions();

    d->tagClearButton   = new ModelClearButton(d->tagSelectCB->view()->albumModel());
    d->tagClearButton->setToolTip(i18nc("@info:tooltip", "Reset selected tags"));

    selectAlbumsLayout->addWidget(includeAlbumsLabel,  0, 0, 1, 2);
    selectAlbumsLayout->addWidget(d->albumSelectCB,    1, 0);
    selectAlbumsLayout->addWidget(d->albumClearButton, 1, 1);
    selectAlbumsLayout->addWidget(d->tagSelectCB,      2, 0);
    selectAlbumsLayout->addWidget(d->tagClearButton,   2, 1);
    selectAlbumsLayout->setRowStretch(3, 1);

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
    loadState();

    slotUpdateClearButtons();
}

AlbumSelectors::~AlbumSelectors()
{
    delete d;
}

void AlbumSelectors::slotUpdateClearButtons()
{
    d->albumClearButton->animateVisible(!d->albumSelectCB->model()->checkedAlbums().isEmpty());
    d->tagClearButton->animateVisible(!d->tagSelectCB->model()->checkedAlbums().isEmpty());
}

QList<Album*> AlbumSelectors::selectedAlbums() const
{
    QList<Album*> albums;
    albums << d->albumSelectCB->model()->checkedAlbums();
    albums << d->tagSelectCB->model()->checkedAlbums();

    kDebug() << albums;
    return albums;
}

void AlbumSelectors::loadState()
{
    d->albumSelectCB->view()->loadState();
    d->tagSelectCB->view()->loadState();
}

void AlbumSelectors::saveState()
{
    d->albumSelectCB->view()->saveState();
    d->tagSelectCB->view()->saveState();
}

} // namespace Digikam
