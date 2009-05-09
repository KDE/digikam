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

#include "albumselectcombobox.h"
#include "albumselectcombobox.moc"

// Qt includes

#include <QSortFilterProxyModel>
#include <QTreeView>

// KDE includes

#include <klocale.h>

// Local includes

#include "albummodel.h"

namespace Digikam
{

AlbumSelectComboBox::AlbumSelectComboBox(QWidget *parent)
            : TreeViewLineEditComboBox(parent)
{
    m_noSelectionText = i18n("No Album Selected");
    m_model           = 0;
    m_filterModel     = 0;
}

AlbumSelectComboBox::~AlbumSelectComboBox()
{
}

void AlbumSelectComboBox::setDefaultAlbumModels()
{
    m_noSelectionText = i18n("No Album Selected");
    setModels(new AlbumModel(AlbumModel::IgnoreRootAlbum, this),
              new QSortFilterProxyModel(this));
    view()->expandToDepth(0);
}

void AlbumSelectComboBox::setDefaultTagModels()
{
    m_noSelectionText = i18n("No Tag Selected");
    setModels(new TagModel(AlbumModel::IgnoreRootAlbum, this),
              new QSortFilterProxyModel(this));
}

void AlbumSelectComboBox::setModels(AbstractCheckableAlbumModel *model, QSortFilterProxyModel *filterModel)
{
    m_model = model;
    m_filterModel = filterModel;

    m_model->setCheckable(true);
    connect(m_model, SIGNAL(checkStateChanged(Album*, int)),
            this, SLOT(updateText()));

    m_filterModel->setDynamicSortFilter(true);
    m_filterModel->setSourceModel(m_model);

    setModel(m_filterModel);
    installView();
    view()->setSortingEnabled(true);
    view()->sortByColumn(0, Qt::AscendingOrder);
    view()->collapseAll();

    updateText();
}

void AlbumSelectComboBox::setNoSelectionText(const QString &text)
{
    m_noSelectionText = text;
    updateText();
}

AbstractCheckableAlbumModel *AlbumSelectComboBox::model() const
{
    return m_model;
}

QSortFilterProxyModel *AlbumSelectComboBox::filterModel() const
{
    return m_filterModel;
}

void AlbumSelectComboBox::updateText()
{
    QList<Album *> checkedAlbums = m_model->checkedAlbums();
    if (checkedAlbums.isEmpty())
    {
        setLineEditText(m_noSelectionText);
    }
    else if (checkedAlbums.count() == 1)
    {
        setLineEditText(checkedAlbums.first()->title());
    }
    else
    {
        if (m_model->albumType() == Album::TAG)
            setLineEditText(i18np("1 Tag selected", "%1 Tags selected", checkedAlbums.count()));
        else
            setLineEditText(i18np("1 Album selected", "%1 Albums selected", checkedAlbums.count()));
    }
}

} // namespace Digikam

