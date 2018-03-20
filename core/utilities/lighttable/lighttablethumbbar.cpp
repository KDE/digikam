/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablethumbbar.h"

// Qt includes

#include <QAction>
#include <QList>
#include <QPixmap>
#include <QPainter>
#include <QContextMenuEvent>
#include <QApplication>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "applicationsettings.h"
#include "contextmenuhelper.h"
#include "imagefiltermodel.h"
#include "imagedragdrop.h"
#include "fileactionmngr.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

template <typename T, class Container>
void removeAnyInInterval(Container& list, const T& begin, const T& end)
{
    typename Container::iterator it;

    for (it = list.begin(); it != list.end();)
    {
        if ((*it) >= begin && (*it) <= end)
        {
            it = list.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

class LightTableImageListModel : public ImageListModel
{
public:

    explicit LightTableImageListModel(QObject* const parent = 0)
        : ImageListModel(parent), m_exclusive(false)
    {
    }

    void clearLightTableState()
    {
        m_leftIndexes.clear();
        m_rightIndexes.clear();
    }

    void setExclusiveLightTableState(bool exclusive)
    {
        m_exclusive = exclusive;
    }

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
    {
        if (role == LTLeftPanelRole)
        {
            return m_leftIndexes.contains(index.row());
        }
        else if (role == LTRightPanelRole)
        {
            return m_rightIndexes.contains(index.row());
        }

        return ImageListModel::data(index, role);
    }

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole)
    {
        if (!index.isValid())
        {
            return false;
        }

        if (role == LTLeftPanelRole)
        {
            if (m_exclusive)
            {
                m_leftIndexes.clear();
            }

            m_leftIndexes << index.row();

            return true;
        }
        else if (role == LTRightPanelRole)
        {
            if (m_exclusive)
            {
                m_rightIndexes.clear();
            }

            m_rightIndexes << index.row();

            return true;
        }

        return ImageListModel::setData(index, value, role);
    }

    virtual void imageInfosAboutToBeRemoved(int begin, int end)
    {
        removeAnyInInterval(m_leftIndexes, begin, end);
        removeAnyInInterval(m_rightIndexes, begin, end);
    }

    virtual void imageInfosCleared()
    {
        clearLightTableState();
    }

protected:

    QSet<int> m_leftIndexes;
    QSet<int> m_rightIndexes;
    bool      m_exclusive;
};

class LightTableThumbBar::Private
{

public:

    Private()
    {
        navigateByPair   = false;
        imageInfoModel   = 0;
        imageFilterModel = 0;
        dragDropHandler  = 0;
    }

    bool                      navigateByPair;

    LightTableImageListModel* imageInfoModel;
    ImageFilterModel*         imageFilterModel;
    ImageDragDropHandler*     dragDropHandler;
};

LightTableThumbBar::LightTableThumbBar(QWidget* const parent)
    : ImageThumbnailBar(parent),
      d(new Private)
{
    d->imageInfoModel   = new LightTableImageListModel(this);
    // only one is left, only one is right at a time
    d->imageInfoModel->setExclusiveLightTableState(true);

    d->imageFilterModel = new ImageFilterModel(this);
    d->imageFilterModel->setSourceImageModel(d->imageInfoModel);

    d->imageInfoModel->setWatchFlags(d->imageFilterModel->suggestedWatchFlags());
    d->imageInfoModel->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());

    d->imageFilterModel->setCategorizationMode(ImageSortSettings::NoCategories);
    d->imageFilterModel->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    d->imageFilterModel->setSortRole((ImageSortSettings::SortRole)ApplicationSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ImageSortSettings::SortOrder)ApplicationSettings::instance()->getImageSorting());
    d->imageFilterModel->setAllGroupsOpen(true); // disable filtering out by group, see bug #308948
    d->imageFilterModel->sort(0); // an initial sorting is necessary

    d->dragDropHandler  = new ImageDragDropHandler(d->imageInfoModel);
    d->dragDropHandler->setReadOnlyDrop(true);
    d->imageInfoModel->setDragDropHandler(d->dragDropHandler);

    setModels(d->imageInfoModel, d->imageFilterModel);
    setSelectionMode(QAbstractItemView::SingleSelection);

    connect(d->dragDropHandler, SIGNAL(imageInfosDropped(QList<ImageInfo>)),
            this, SIGNAL(signalDroppedItems(QList<ImageInfo>)));

    connect(d->imageInfoModel, SIGNAL(imageInfosAdded(QList<ImageInfo>)),
            this, SIGNAL(signalContentChanged()));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));
}

LightTableThumbBar::~LightTableThumbBar()
{
    delete d;
}

void LightTableThumbBar::setItems(const ImageInfoList& list)
{
    foreach(const ImageInfo& info, list)
    {
        if (!d->imageInfoModel->hasImage(info))
        {
            d->imageInfoModel->addImageInfo(info);
        }
    }
}

void LightTableThumbBar::slotDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
    {
        setFlow(TopToBottom);
    }
    else
    {
        setFlow(LeftToRight);
    }

    scrollTo(currentIndex());
}

void LightTableThumbBar::clear()
{
    d->imageInfoModel->clearImageInfos();
    emit signalContentChanged();
}

void LightTableThumbBar::setNavigateByPair(bool b)
{
    d->navigateByPair = b;
}

void LightTableThumbBar::showContextMenuOnInfo(QContextMenuEvent* e, const ImageInfo& info)
{
    // temporary actions ----------------------------------

    QAction* leftPanelAction = 0, *rightPanelAction = 0, *editAction = 0, *removeAction = 0, *clearAllAction = 0;

    leftPanelAction  = new QAction(QIcon::fromTheme(QLatin1String("go-previous")),   i18n("Show on left panel"),  this);
    rightPanelAction = new QAction(QIcon::fromTheme(QLatin1String("go-next")),       i18n("Show on right panel"), this);
    editAction       = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Edit"),                this);
    removeAction     = new QAction(QIcon::fromTheme(QLatin1String("window-close")),  i18n("Remove item"),         this);
    clearAllAction   = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),   i18n("Clear all"),           this);

    leftPanelAction->setEnabled(d->navigateByPair  ? false : true);
    rightPanelAction->setEnabled(d->navigateByPair ? false : true);
    clearAllAction->setEnabled(countItems()        ? true  : false);

    // ----------------------------------------------------

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.addAction(leftPanelAction, true);
    cmhelper.addAction(rightPanelAction, true);
    cmhelper.addSeparator();
    cmhelper.addAction(editAction);
    cmhelper.addServicesMenu(QList<QUrl>() << info.fileUrl());
    cmhelper.addSeparator();
    cmhelper.addLabelsAction();
    cmhelper.addSeparator();
    cmhelper.addAction(removeAction);
    cmhelper.addAction(clearAllAction, true);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    QAction* const choice = cmhelper.exec(e->globalPos());

    if (choice)
    {
        if (choice == leftPanelAction)
        {
            emit signalSetItemOnLeftPanel(info);
        }
        else if (choice == rightPanelAction)
        {
            emit signalSetItemOnRightPanel(info);
        }
        else if (choice == editAction)
        {
            emit signalEditItem(info);
        }
        else if (choice == removeAction)
        {
            emit signalRemoveItem(info);
        }
        else if (choice == clearAllAction)
        {
            emit signalClearAll();
        }
    }
}

void LightTableThumbBar::slotColorLabelChanged(const QUrl& url, int color)
{
    assignColorLabel(ImageInfo::fromUrl(url), color);
}

void LightTableThumbBar::slotPickLabelChanged(const QUrl& url, int pick)
{
    assignPickLabel(ImageInfo::fromUrl(url), pick);
}

void LightTableThumbBar::slotAssignPickLabel(int pickId)
{
    assignPickLabel(currentInfo(), pickId);
}

void LightTableThumbBar::slotAssignColorLabel(int colorId)
{
    assignColorLabel(currentInfo(), colorId);
}

void LightTableThumbBar::slotRatingChanged(const QUrl& url, int rating)
{
    assignRating(ImageInfo::fromUrl(url), rating);
}

void LightTableThumbBar::slotAssignRating(int rating)
{
    assignRating(currentInfo(), rating);
}

void LightTableThumbBar::assignPickLabel(const ImageInfo& info, int pickId)
{
    FileActionMngr::instance()->assignPickLabel(info, pickId);
}

void LightTableThumbBar::assignRating(const ImageInfo& info, int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));
    FileActionMngr::instance()->assignRating(info, rating);
}

void LightTableThumbBar::assignColorLabel(const ImageInfo& info, int colorId)
{
    FileActionMngr::instance()->assignColorLabel(info, colorId);
}

void LightTableThumbBar::slotToggleTag(const QUrl& url, int tagID)
{
    toggleTag(ImageInfo::fromUrl(url), tagID);
}

void LightTableThumbBar::toggleTag(int tagID)
{
    toggleTag(currentInfo(), tagID);
}

void LightTableThumbBar::toggleTag(const ImageInfo& info, int tagID)
{
    if (!info.isNull())
    {
        if (!info.tagIds().contains(tagID))
        {
            FileActionMngr::instance()->assignTag(info, tagID);
        }
        else
        {
            FileActionMngr::instance()->removeTag(info, tagID);
        }
    }
}

void LightTableThumbBar::setOnLeftPanel(const ImageInfo& info)
{
    QModelIndex index = d->imageInfoModel->indexForImageInfo(info);
    // model has exclusiveLightTableState, so any previous index will be reset
    d->imageInfoModel->setData(index, true, ImageModel::LTLeftPanelRole);
    viewport()->update();
}

void LightTableThumbBar::setOnRightPanel(const ImageInfo& info)
{
    QModelIndex index = d->imageInfoModel->indexForImageInfo(info);
    // model has exclusiveLightTableState, so any previous index will be reset
    d->imageInfoModel->setData(index, true, ImageModel::LTRightPanelRole);
    viewport()->update();
}

bool LightTableThumbBar::isOnLeftPanel(const ImageInfo& info) const
{
    return d->imageInfoModel->indexForImageInfo(info).data(ImageModel::LTLeftPanelRole).toBool();
}

bool LightTableThumbBar::isOnRightPanel(const ImageInfo& info) const
{
    return d->imageInfoModel->indexForImageInfo(info).data(ImageModel::LTRightPanelRole).toBool();
}

QModelIndex LightTableThumbBar::findItemByInfo(const ImageInfo& info) const
{
    if (!info.isNull())
    {
        return d->imageInfoModel->indexForImageInfo(info);
    }

    return QModelIndex();
}

ImageInfo LightTableThumbBar::findItemByIndex(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return d->imageInfoModel->imageInfo(index);
    }

    return ImageInfo();
}

void LightTableThumbBar::removeItemByInfo(const ImageInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    d->imageInfoModel->removeImageInfo(info);
    emit signalContentChanged();
}

int LightTableThumbBar::countItems() const
{
    return d->imageInfoModel->rowCount();
}

void LightTableThumbBar::paintEvent(QPaintEvent* e)
{
    if (!countItems())
    {
        QPainter p(viewport());
        p.setPen(QPen(qApp->palette().color(QPalette::Text)));
        p.drawText(0, 0, width(), height(),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   i18n("Drag and drop images here"));
        p.end();
        return;
    }

    ImageThumbnailBar::paintEvent(e);
}

void LightTableThumbBar::slotSetupChanged()
{
    d->imageFilterModel->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
}

}  // namespace Digikam
