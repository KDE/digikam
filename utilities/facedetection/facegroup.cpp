/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Managing of face tag region items on a GraphicsDImgView
 *
 * Copyright (C) 2010 Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "facegroup.moc"

// Qt includes

#include <QGraphicsSceneHoverEvent>
#include <QObject>

// KDE includes

// KFace includes

#include <libkface/kface.h>

// Local includes

#include "addtagscombobox.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "assignnamewidget.h"
#include "dimgpreviewitem.h"
#include "faceiface.h"
#include "facepipeline.h"
#include "graphicsdimgview.h"
#include "imageinfo.h"
#include "regionframeitem.h"
#include "searchutilities.h"
#include "taggingaction.h"
#include "itemvisibilitycontroller.h"

namespace Digikam
{

enum FaceGroupState
{
    NoFaces,
    LoadingFaces,
    FacesLoaded
};

class FaceItem : public RegionFrameItem
{
public:

    FaceItem(QGraphicsItem* parent = 0)
        : RegionFrameItem(parent),
          m_widget(0)
    {
    }



    void setFace(const DatabaseFace& face)
    {
        m_face = face;
    }

    DatabaseFace face() const
    {
        return m_face;
    }

    void setHudWidget(AssignNameWidget* widget)
    {
        m_widget = widget;
        RegionFrameItem::setHudWidget(widget);
    }

    AssignNameWidget* widget() const
    {
        return m_widget;
    }

protected:

    AssignNameWidget* m_widget;
    DatabaseFace      m_face;
};

class FaceGroup::FaceGroupPriv
{
public:

    FaceGroupPriv(FaceGroup* q) : q(q)
    {
        view                 = 0;
        autoSuggest          = false;
        showOnHover          = false;
        visibilityController = 0;
        state                = NoFaces;
        tagModel             = 0;
        filterModel          = 0;
        filteredModel        = 0;
    }

    void                       applyVisible();
    FaceItem*                  createItem(const DatabaseFace& face);
    AssignNameWidget*          createAssignNameWidget(const DatabaseFace& face, const QVariant& identifier);
    AssignNameWidget::Mode     assignWidgetMode(DatabaseFace::Type type);
    void                       checkModels();

public:

    GraphicsDImgView*          view;
    ImageInfo                  info;
    bool                       autoSuggest;
    bool                       showOnHover;

    QList<FaceItem*>           items;

    FaceGroupState             state;
    ItemVisibilityController*  visibilityController;

    TagModel*                  tagModel;
    CheckableAlbumFilterModel* filterModel;
    TagPropertiesFilterModel*  filteredModel;

    FaceIface                  faceIface;
    FacePipeline               confirmPipeline;

    FaceGroup* const           q;
};

FaceGroup::FaceGroup(GraphicsDImgView* view)
         : QObject(view), d(new FaceGroupPriv(this))
{
    d->view                 = view;
    d->visibilityController = new ItemVisibilityController(this);
    d->visibilityController->setShallBeShown(false);

    connect(view->previewItem(), SIGNAL(stateChanged(int)),
            this, SLOT(itemStateChanged(int)));
}

FaceGroup::~FaceGroup()
{
    delete d;
}

void FaceGroup::itemStateChanged(int itemState)
{
    switch (itemState)
    {
        case DImgPreviewItem::NoImage:
        case DImgPreviewItem::Loading:
        case DImgPreviewItem::ImageLoadingFailed:
            d->visibilityController->hide();
            break;
        case DImgPreviewItem::ImageLoaded:
            if (d->state == FacesLoaded)
                d->visibilityController->show();
            break;
    }
}

bool FaceGroup::isVisible() const
{
    return d->visibilityController->shallBeShown();
}

bool FaceGroup::hasVisibleItems() const
{
    return d->visibilityController->hasVisibleItems();
}

ImageInfo FaceGroup::info() const
{
    return d->info;
}

QList<RegionFrameItem*> FaceGroup::items() const
{
    QList<RegionFrameItem*> items;
    foreach (FaceItem* item, d->items)
        items << item;
    return items;
}

void FaceGroup::setAutoSuggest(bool doAutoSuggest)
{
    if (d->autoSuggest == doAutoSuggest)
        return;
    d->autoSuggest = doAutoSuggest;
}

bool FaceGroup::autoSuggest() const
{
    return d->autoSuggest;
}

void FaceGroup::setShowOnHover(bool show)
{
    d->showOnHover = show;
}

bool FaceGroup::showOnHover() const
{
    return d->showOnHover;
}

void FaceGroup::FaceGroupPriv::applyVisible()
{
    if (state == NoFaces)
    {
        // If not yet loaded, load. load() will transitionToVisible after loading.
        q->load();
    }
    else if (state == FacesLoaded)
    {
        // show existing faces, if we have an image
        if (view->previewItem()->isLoaded())
            visibilityController->show();
    }
}

void FaceGroup::setVisible(bool visible)
{
    d->visibilityController->setShallBeShown(visible);
    d->applyVisible();
}

void FaceGroup::setVisibleItem(RegionFrameItem *item)
{
    d->visibilityController->setItemThatShallBeShown(item);
    d->applyVisible();
}

void FaceGroup::setInfo(const ImageInfo& info)
{
    if (d->info == info)
        return;

    clear();

    d->info = info;

    if (d->visibilityController->shallBeShown())
        load();
}

static QPointF closestPointOfRect(const QPointF& p, const QRectF& r)
{
    QPointF cp = p;
    if (p.x() < r.left())
        cp.setX(r.left());
    else if (p.x() > r.right())
        cp.setX(r.right());

    if (p.y() < r.top())
        cp.setY(r.top());
    else if (p.y() > r.bottom())
        cp.setY(r.bottom());

    return cp;
}

RegionFrameItem* FaceGroup::closestItem(const QPointF& p, qreal* manhattanLength) const
{
    RegionFrameItem* closestItem = 0;
    qreal minDistance            = 0;
    qreal minCenterDistance      = 0;

    foreach (RegionFrameItem* item, d->items)
    {
        QRectF r        = item->boundingRect().translated(item->pos());
        qreal distance = (p - closestPointOfRect(p, r)).manhattanLength();

        if (!closestItem
            || distance < minDistance
            || (distance == 0 && (p - r.center()).manhattanLength() < minCenterDistance)
           )
        {
            closestItem = item;
            minDistance = distance;
            if (distance == 0)
                minCenterDistance = (p - r.center()).manhattanLength();
        }
    }

    if (manhattanLength)
        *manhattanLength = minDistance;

    return closestItem;
}

void FaceGroup::itemHoverMoveEvent(QGraphicsSceneHoverEvent *e)
{
    if (d->showOnHover && !isVisible())
    {
        qreal distance;
        RegionFrameItem *item = closestItem(e->scenePos(), &distance);
        // There's a possible nuisance when the direct mouse way from hovering pos to HUD widget
        // is not part of the condition. Maybe, we should add a exemption for this case.
        if (distance < 25)
            setVisibleItem(item);
        else
            setVisibleItem(0);
    }
}

void FaceGroup::itemHoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (d->showOnHover && !isVisible())
        setVisibleItem(0);
}

void FaceGroup::itemHoverEnterEvent(QGraphicsSceneHoverEvent *)
{
}

FaceItem* FaceGroup::FaceGroupPriv::createItem(const DatabaseFace& face)
{
    FaceItem* item = new FaceItem(view->previewItem());
    item->setFace(face);
    item->setOriginalRect(face.region().toRect());
    item->setVisible(false);

    q->connect(view, SIGNAL(viewportRectChanged(const QRectF&)),
               item, SLOT(setViewportRect(const QRectF&)));

    return item;
}

void FaceGroup::FaceGroupPriv::checkModels()
{
    if (!tagModel)
    {
        tagModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, q);
    }
    if (!filterModel)
    {
        filterModel = new CheckableAlbumFilterModel(q);
    }
    if (!filteredModel)
    {
        filteredModel = new TagPropertiesFilterModel(q);
    }
}

AssignNameWidget::Mode FaceGroup::FaceGroupPriv::assignWidgetMode(DatabaseFace::Type type)
{
    switch (type)
    {
        case DatabaseFace::UnknownName:
            return AssignNameWidget::UnknownName;
        case DatabaseFace::UnconfirmedName:
            return AssignNameWidget::UnconfirmedName;
        case DatabaseFace::ConfirmedName:
            return AssignNameWidget::ConfirmedName;
        default:
            return AssignNameWidget::InvalidMode;
    }
}

AssignNameWidget* FaceGroup::FaceGroupPriv::createAssignNameWidget(const DatabaseFace& face, const QVariant& identifier)
{
    AssignNameWidget* assignWidget = new AssignNameWidget;
    assignWidget->setMode(assignWidgetMode(face.type()));
    assignWidget->setBackgroundStyle(AssignNameWidget::TransparentRound);
    assignWidget->setLayoutMode(AssignNameWidget::TwoLines);
    assignWidget->setFace(info, identifier);
    checkModels();
    assignWidget->setTagModel(tagModel, filteredModel, filterModel);

    q->connect(assignWidget, SIGNAL(assigned(const TaggingAction&, const ImageInfo&, const QVariant&)),
               q, SLOT(slotAssigned(const TaggingAction&, const ImageInfo&, const QVariant&)));

    q->connect(assignWidget, SIGNAL(rejected(const ImageInfo&, const QVariant&)),
               q, SLOT(slotRejected(const ImageInfo&, const QVariant&)));

    return assignWidget;
}

void FaceGroup::load()
{
    if (d->state != NoFaces)
        return;

    d->state = LoadingFaces;

    if (d->info.isNull())
    {
        d->state = FacesLoaded;
        return;
    }

    QList<DatabaseFace> faces = d->faceIface.databaseFaces(d->info.id());

    foreach (const DatabaseFace &face, faces)
    {
        FaceItem* item = d->createItem(face);

        // for identification, use index in our list
        AssignNameWidget* assignWidget = d->createAssignNameWidget(face, d->items.size());
        item->setHudWidget(assignWidget);
        //new StyleSheetDebugger(assignWidget);

        d->visibilityController->addItem(item);

        d->items << item;
        /*kDebug() << assignWidget->size() << assignWidget->sizeHint() << assignWidget->minimumSizeHint()
                 << ((QComboBox*)assignWidget->comboBox())->view()->sizeHint() << ((QComboBox*)assignWidget->comboBox())->view()->minimumSizeHint()
                 << assignWidget->sizePolicy() << assignWidget->comboBox()->sizeHint();*/
    }

    d->state = FacesLoaded;

    if (d->view->previewItem()->isLoaded())
        d->visibilityController->show();
}

void FaceGroup::clear()
{
    d->visibilityController->clear();
    foreach (RegionFrameItem* item, d->items)
        delete item;
    d->items.clear();
    d->state = NoFaces;
}

void FaceGroup::addFace()
{
/*
    int w = this->scene()->width()/2;
    int h = w;
    int x = this->scene()->width()/2 - w/2;
    int y = this->scene()->height()/2 - w/2;

    d->faceitems.append(new FaceItem(0, this->scene(), QRect(x, y, w, h), d->scale , "", d->scale));
*/
}

void FaceGroup::rejectAll()
{
}

void FaceGroup::slotAssigned(const TaggingAction& action, const ImageInfo& info, const QVariant& faceIdentifier)
{
    if (info != d->info)
        return;
    FaceItem* item    = d->items[faceIdentifier.toInt()];

    DatabaseFace face = item->face();
    QRect currentRect = item->originalRect();

    face              = d->faceIface.confirmName(d->info.id(), action.tagId(), currentRect, face.region().toRect());

    item->widget()->setMode(d->assignWidgetMode(face.type()));
    //item->widget()->setCurrentTag(face.tagId())
    item->setFace(face);
}

void FaceGroup::slotRejected(const ImageInfo& info, const QVariant& faceIdentifier)
{
    if (info != d->info)
        return;
    FaceItem* item = d->items.takeAt(faceIdentifier.toInt());
    d->faceIface.removeFace(item->face());

    item->setFace(DatabaseFace());
    d->visibilityController->hideAndRemoveItem(item);
}

void FaceGroup::startAutoSuggest()
{
    if (!d->autoSuggest)
        return;
}

/*
void ImagePreviewView::trainFaces()
{
    QList<Face> trainList;
    foreach(Face f, d->currentFaces)
    {
        if(f.name() != "" && !d->faceIface->isFaceTrained(getImageInfo().id(), f.toRect(), f.name()))
            trainList += f;
    }

    kDebug() << "Number of training faces" << trainList.size();

    if(trainList.size()!=0)
    {
        d->faceIface->trainWithFaces(trainList);
        d->faceIface->markFacesAsTrained(getImageInfo().id(), trainList);
    }
}
* /

void ImagePreviewView::suggestFaces()
{
    / *
    // Assign tentative names to the face list
    QList<Face> recogList;
    foreach(Face f, d->currentFaces)
    {
        if(!d->faceIface->isFaceRecognized(getImageInfo().id(), f.toRect(), f.name()) && f.name().isEmpty())
        {
            f.setName(d->faceIface->recognizedName(f));
            d->faceIface->markFaceAsRecognized(getImageInfo().id(), f.toRect(), f.name());

            // If the face wasn't recognized (too distant) don't suggest anything
            if(f.name().isEmpty())
                continue;
            else
                recogList += f;
        }
    }

    kDebug() << "Number of suggestions = " << recogList.size();
    kDebug() << "Number of faceitems = " << d->faceitems.size();
    // Now find the relevant face items and suggest faces
    for(int i = 0; i < recogList.size(); ++i)
    {
        for(int j = 0; j < d->faceitems.size(); ++j)
        {
            if(recogList[i].toRect() == d->faceitems[j]->originalRect())
            {
                kDebug() << "Suggesting a name " << recogList[i].name();
                d->faceitems[j]->suggest(recogList[i].name());
                break;
            }
        }
    }
    * /
}
*/

} // namespace Digikam
