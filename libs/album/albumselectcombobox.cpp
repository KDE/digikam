/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-09
 * Description : A combo box for selecting albums
 *
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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

// Qt includes

#include <QSortFilterProxyModel>
#include <QTreeView>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albumtreeview.h"
#include "contextmenuhelper.h"

namespace Digikam
{

class AlbumSelectComboBox::Private
{
public:

    explicit Private(AlbumSelectComboBox* q)
        : q(q)
    {
        model                 = 0;
        filterModel           = 0;
        isCheckable           = true;
        closeOnActivate       = false;
        showCheckStateSummary = true;
    }

    AbstractCheckableAlbumModel* model;
    AlbumFilterModel*            filterModel;
    QString                      noSelectionText;
    bool                         isCheckable;
    bool                         closeOnActivate;
    bool                         showCheckStateSummary;

    void                         updateCheckable();
    void                         updateCloseOnActivate();

    AlbumSelectComboBox* const   q;
};

AlbumSelectComboBox::AlbumSelectComboBox(QWidget* const parent)
    : TreeViewLineEditComboBox(parent),
      d(new Private(this))
{
    d->noSelectionText = i18n("No Album Selected");
}

AlbumSelectComboBox::~AlbumSelectComboBox()
{
    delete d;
}

void AlbumSelectComboBox::setDefaultAlbumModel()
{
    d->noSelectionText = i18n("No Album Selected");
    setModel(new AlbumModel(AlbumModel::IgnoreRootAlbum, this));
    view()->expandToDepth(0);
}

void AlbumSelectComboBox::setDefaultTagModel()
{
    d->noSelectionText = i18n("No Tag Selected");
    setModel(new TagModel(AlbumModel::IgnoreRootAlbum, this));
}

void AlbumSelectComboBox::setModel(AbstractCheckableAlbumModel* model, AlbumFilterModel* filterModel)
{
    d->model = model;

    if (filterModel)
    {
        d->filterModel = filterModel;
    }
    else
    {
        d->filterModel = new AlbumFilterModel(this);
        //d->filterModel->setDynamicSortFilter(true);
        d->filterModel->setSourceAlbumModel(d->model);
    }

    d->updateCheckable();

    QComboBox::setModel(d->filterModel);
    installView();

    d->updateCloseOnActivate();
    updateText();
}

void AlbumSelectComboBox::installView(QAbstractItemView* v)
{
    if (view())
    {
        return;
    }

    TreeViewLineEditComboBox::installView(v);
    view()->setSortingEnabled(true);
    view()->sortByColumn(0, Qt::AscendingOrder);
    view()->collapseAll();
}

void AlbumSelectComboBox::setCheckable(bool checkable)
{
    if (checkable == d->isCheckable)
    {
        return;
    }

    d->isCheckable = checkable;
    d->updateCheckable();
}

bool AlbumSelectComboBox::isCheckable() const
{
    return d->isCheckable;
}

void AlbumSelectComboBox::Private::updateCheckable()
{
    if (!model)
    {
        return;
    }

    model->setCheckable(isCheckable);

    if (isCheckable)
    {
        connect(model, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
                q, SLOT(updateText()));
    }
    else
    {
        disconnect(model, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
                   q, SLOT(updateText()));
    }
}

void AlbumSelectComboBox::setCloseOnActivate(bool close)
{
    if (d->closeOnActivate == close)
    {
        return;
    }

    d->closeOnActivate = close;
    d->updateCloseOnActivate();
}

void AlbumSelectComboBox::Private::updateCloseOnActivate()
{
    if (!q->view())
    {
        return;
    }

    if (closeOnActivate)
    {
        connect(q->view(), SIGNAL(activated(QModelIndex)),
                q, SLOT(hidePopup()));
    }
    else
    {
        disconnect(q->view(), SIGNAL(activated(QModelIndex)),
                   q, SLOT(hidePopup()));
    }
}

void AlbumSelectComboBox::setNoSelectionText(const QString& text)
{
    d->noSelectionText = text;
    updateText();
}

void AlbumSelectComboBox::setShowCheckStateSummary(bool show)
{
    d->showCheckStateSummary = show;
    updateText();
}

AbstractCheckableAlbumModel* AlbumSelectComboBox::model() const
{
    return d->model;
}

QSortFilterProxyModel* AlbumSelectComboBox::filterModel() const
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
    if (!d->isCheckable || !d->showCheckStateSummary)
    {
        return;
    }

    QList<Album*> checkedAlbums          = d->model->checkedAlbums();
    QList<Album*> partiallyCheckedAlbums = d->model->partiallyCheckedAlbums();
    QString newIncludeText;
    QString newExcludeText;

    if (!checkedAlbums.isEmpty())
    {
        if (checkedAlbums.count() == 1)
        {
            newIncludeText = checkedAlbums.first()->title();
        }
        else
        {
            if (d->model->albumType() == Album::TAG)
            {
                newIncludeText = i18np("1 Tag selected", "%1 Tags selected", checkedAlbums.count());
            }
            else
            {
                newIncludeText = i18np("1 Album selected", "%1 Albums selected", checkedAlbums.count());
            }
        }
    }

    if (!partiallyCheckedAlbums.isEmpty())
    {
        if (d->model->albumType() == Album::TAG)
        {
            newExcludeText = i18np("1 Tag excluded", "%1 Tags excluded", partiallyCheckedAlbums.count());
        }
        else
        {
            newExcludeText = i18np("1 Album excluded", "%1 Albums excluded", partiallyCheckedAlbums.count());
        }
    }

    if (newIncludeText.isEmpty() && newExcludeText.isEmpty())
    {
        setLineEditText(d->noSelectionText);
    }
    else if (newIncludeText.isEmpty() || newExcludeText.isEmpty())
    {
        setLineEditText(newIncludeText + newExcludeText);
    }
    else
    {
        setLineEditText(newIncludeText + QLatin1String(", ") + newExcludeText);
    }
}

// ---------------------------------------------------------------------------------------------------

AbstractAlbumTreeViewSelectComboBox::AbstractAlbumTreeViewSelectComboBox(QWidget* const parent)
    : AlbumSelectComboBox(parent),
      m_treeView(0)
{
}

void AbstractAlbumTreeViewSelectComboBox::installView(QAbstractItemView* view)
{
    if (!view)
    {
        view = m_treeView;
    }

    AlbumSelectComboBox::installView(view);
}

void AbstractAlbumTreeViewSelectComboBox::sendViewportEventToView(QEvent* e)
{
    // needed for StayPoppedUpComboBox
    m_treeView->viewportEvent(e);
}

void AbstractAlbumTreeViewSelectComboBox::setTreeView(AbstractAlbumTreeView* const treeView)
{
    // this is independent from the installView mechanism, just to override
    // the tree view created below without the need to subclass
    if (!m_treeView)
    {
        m_treeView = treeView;
    }
}

// -------------------------------------------------------------------------------------------------------------------

class CheckUncheckContextMenuElement : public QObject, public AbstractAlbumTreeView::ContextMenuElement
{
public:

    explicit CheckUncheckContextMenuElement(QObject* const parent)
        : QObject(parent)
    {
    }

    void addActions(AbstractAlbumTreeView* view, ContextMenuHelper& cmh, Album* album)
    {
        AbstractCheckableAlbumModel* const checkable = qobject_cast<AbstractCheckableAlbumModel*>(view->albumModel());

        if (checkable)
        {
            cmh.setAlbumModel(checkable);
            cmh.addAlbumCheckUncheckActions(album);
        }
    }
};

void AbstractAlbumTreeViewSelectComboBox::addCheckUncheckContextMenuActions()
{
    if (m_treeView)
    {
        m_treeView->setEnableContextMenu(true);
        m_treeView->addContextMenuElement(new CheckUncheckContextMenuElement(this));
    }
}

// ---------------------------------------------------------------------------------

AlbumTreeViewSelectComboBox::AlbumTreeViewSelectComboBox(QWidget* const parent)
    : AbstractAlbumTreeViewSelectComboBox(parent)
{
}

AlbumTreeView* AlbumTreeViewSelectComboBox::view() const
{
    return static_cast<AlbumTreeView*>(m_treeView);
}

void AlbumTreeViewSelectComboBox::setDefaultModel()
{
    setModel(0,0);
}

void AlbumTreeViewSelectComboBox::setModel(AlbumModel* model, CheckableAlbumFilterModel* filterModel)
{
    if (!m_treeView)
    {
        AlbumTreeView::Flags flags;
        m_treeView = new AlbumTreeView(this, flags);
    }

    if (!model)
    {
        model = new AlbumModel(AlbumModel::IgnoreRootAlbum, this);
    }

    if (!filterModel)
    {
        filterModel = new CheckableAlbumFilterModel(this);
    }

    view()->setAlbumModel(model);
    view()->setAlbumFilterModel(filterModel);

    AlbumSelectComboBox::setModel(view()->albumModel(), view()->albumFilterModel());

    view()->expandToDepth(0);
}

// ---------------------------------------------------------------------------------------------------

TagTreeViewSelectComboBox::TagTreeViewSelectComboBox(QWidget* const parent)
    : AbstractAlbumTreeViewSelectComboBox(parent)
{
}

TagTreeView* TagTreeViewSelectComboBox::view() const
{
    return static_cast<TagTreeView*>(m_treeView);
}

void TagTreeViewSelectComboBox::setDefaultModel()
{
    setModel(0,0);
}

void TagTreeViewSelectComboBox::setModel(TagModel* model,
                                         TagPropertiesFilterModel* filteredModel,
                                         CheckableAlbumFilterModel* filterModel)
{
    if (!m_treeView)
    {
        TagTreeView::Flags flags;
        m_treeView = new TagTreeView(this, flags);
    }

    if (!model)
    {
        model = new TagModel(AlbumModel::IgnoreRootAlbum, this);
    }

    if (!filteredModel)
    {
        filteredModel = new TagPropertiesFilterModel(this);
    }

    if (!filterModel)
    {
        filterModel = new CheckableAlbumFilterModel(this);
    }

    view()->setAlbumModel(model);
    view()->setAlbumFilterModel(filteredModel, filterModel);

    AlbumSelectComboBox::setModel(view()->albumModel(), view()->albumFilterModel());
}

} // namespace Digikam
