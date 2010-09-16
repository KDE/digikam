/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-09
 * Description : A combo box for selecting albums
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "albumselectcombobox.moc"

// Qt includes

#include <QSortFilterProxyModel>
#include <QTreeView>

// KDE includes

#include <klocale.h>

// Local includes

#include "albummodel.h"
#include "albumfiltermodel.h"

namespace Digikam
{

class AlbumSelectComboBox::AlbumSelectComboBoxPriv
{
public:

    AlbumSelectComboBoxPriv(AlbumSelectComboBox *q) : q(q)
    {
        model       = 0;
        filterModel = 0;
        isCheckable = true;
        closeOnActivate = false;
    }

    AbstractCheckableAlbumModel *model;
    AlbumFilterModel            *filterModel;
    QString                      noSelectionText;
    bool                         isCheckable;
    bool                         closeOnActivate;

    void                         updateCheckable();
    void                         updateCloseOnActivate();

    AlbumSelectComboBox* const q;
};

AlbumSelectComboBox::AlbumSelectComboBox(QWidget *parent)
            : TreeViewLineEditComboBox(parent), d(new AlbumSelectComboBoxPriv(this))
{
    d->noSelectionText = i18n("No Album Selected");
}

AlbumSelectComboBox::~AlbumSelectComboBox()
{
    delete d;
}

void AlbumSelectComboBox::setDefaultAlbumModels()
{
    d->noSelectionText = i18n("No Album Selected");
    setModel(new AlbumModel(AlbumModel::IgnoreRootAlbum, this));
    view()->expandToDepth(0);
}

void AlbumSelectComboBox::setDefaultTagModels()
{
    d->noSelectionText = i18n("No Tag Selected");
    setModel(new TagModel(AlbumModel::IgnoreRootAlbum, this));
}

void AlbumSelectComboBox::setModel(AbstractCheckableAlbumModel *model, AlbumFilterModel *filterModel)
{
    d->model = model;

    if (filterModel)
        d->filterModel = filterModel;
    else
    {
        d->filterModel = new AlbumFilterModel(this);
        d->filterModel->setDynamicSortFilter(true);
        d->filterModel->setSourceAlbumModel(d->model);
    }

    d->updateCheckable();

    QComboBox::setModel(d->filterModel);
    installView();

    d->updateCloseOnActivate();
    updateText();
}

void AlbumSelectComboBox::installView()
{
    TreeViewLineEditComboBox::installView();
    view()->setSortingEnabled(true);
    view()->sortByColumn(0, Qt::AscendingOrder);
    view()->collapseAll();
}

void AlbumSelectComboBox::setCheckable(bool checkable)
{
    if (checkable == d->isCheckable)
        return;

    d->isCheckable = checkable;
    d->updateCheckable();
}

bool AlbumSelectComboBox::isCheckable() const
{
    return d->isCheckable;
}

void AlbumSelectComboBox::AlbumSelectComboBoxPriv::updateCheckable()
{
    if (!model)
        return;
    model->setCheckable(isCheckable);
    if (isCheckable)
    {
        connect(model, SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
                q, SLOT(updateText()));
    }
    else
    {
        disconnect(model, SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
                   q, SLOT(updateText()));
    }
}

void AlbumSelectComboBox::setCloseOnActivate(bool close)
{
    if (d->closeOnActivate == close)
        return;

    d->closeOnActivate = close;
    d->updateCloseOnActivate();
}

void AlbumSelectComboBox::AlbumSelectComboBoxPriv::updateCloseOnActivate()
{
    if (!q->view())
        return;
    if (closeOnActivate)
    {
        connect(q->view(), SIGNAL(activated(const QModelIndex&)),
                q, SLOT(hidePopup()));
    }
    else
    {
        disconnect(q->view(), SIGNAL(activated(const QModelIndex&)),
                   q, SLOT(hidePopup()));
    }
}

void AlbumSelectComboBox::setNoSelectionText(const QString& text)
{
    d->noSelectionText = text;
    updateText();
}

AbstractCheckableAlbumModel *AlbumSelectComboBox::model() const
{
    return d->model;
}

QSortFilterProxyModel *AlbumSelectComboBox::filterModel() const
{
    return d->filterModel;
}

void AlbumSelectComboBox::hidePopup()
{
    // just make this a slot
    return TreeViewLineEditComboBox::hidePopup();
}

void AlbumSelectComboBox::updateText()
{
    QList<Album *> checkedAlbums = d->model->checkedAlbums();
    if (checkedAlbums.isEmpty())
    {
        setLineEditText(d->noSelectionText);
    }
    else if (checkedAlbums.count() == 1)
    {
        setLineEditText(checkedAlbums.first()->title());
    }
    else
    {
        if (d->model->albumType() == Album::TAG)
            setLineEditText(i18np("1 Tag selected", "%1 Tags selected", checkedAlbums.count()));
        else
            setLineEditText(i18np("1 Album selected", "%1 Albums selected", checkedAlbums.count()));
    }
}

} // namespace Digikam

