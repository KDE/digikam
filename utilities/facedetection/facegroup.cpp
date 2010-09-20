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

#include <QObject>

// KDE includes

// KFace includes

#include <libkface/kface.h>

// Local includes

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

class FaceGroup::FaceGroupPriv
{
public:

    FaceGroupPriv(FaceGroup *q) : q(q)
    {
        view                 = 0;
        autoSuggest          = false;
        visibilityController = 0;
        state                = NoFaces;
        tagModel             = 0;
        filterModel          = 0;
    }

    GraphicsDImgView        *view;
    ImageInfo                info;
    bool                     autoSuggest;

    QList<RegionFrameItem*>  items;
    QList<DatabaseFace>      faces;

    FaceGroupState           state;
    ItemVisibilityController*visibilityController;

    TagModel                *tagModel;
    CheckableAlbumFilterModel*filterModel;

    FaceIface                faceIface;
    FacePipeline             confirmPipeline;

    FaceGroup* const         q;

    void                     transitionToVisible(bool visible);
    RegionFrameItem         *createItem(const DatabaseFace &face);
    AssignNameWidget        *createAssignNameWidget(const DatabaseFace& face, const QVariant& identifier);
    void                     checkModels();
};

FaceGroup::FaceGroup(GraphicsDImgView *view)
    : QObject(view), d(new FaceGroupPriv(this))
{
    d->view = view;
    d->visibilityController = new ItemVisibilityController(ItemVisibilityController::ItemGroup, this);
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
    kDebug() << "itemState" << itemState << "state" << d->state;
    switch (itemState)
    {
        case DImgPreviewItem::NoImage:
        case DImgPreviewItem::Loading:
        case DImgPreviewItem::ImageLoadingFailed:
            d->visibilityController->hide();
        case DImgPreviewItem::ImageLoaded:
            if (d->state == FacesLoaded)
                d->visibilityController->show();
    }
}

bool FaceGroup::isVisible() const
{
    return d->visibilityController->shallBeShown();
}

ImageInfo FaceGroup::info() const
{
    return d->info;
}

QList<RegionFrameItem*> FaceGroup::items() const
{
    return d->items;
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

void FaceGroup::show()
{
    setVisible(true);
}

void FaceGroup::hide()
{
    setVisible(false);
}

void FaceGroup::setVisible(bool visible)
{
    d->visibilityController->setShallBeShown(visible);

    if (d->state == NoFaces)
    {
        // If not yet loaded, load. load() will transitionToVisible after loading.
        load();
    }
    else if (d->state == FacesLoaded)
    {
        // show existing faces, if we have an image
        if (d->view->previewItem()->isLoaded())
            d->visibilityController->show();
    }
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

RegionFrameItem *FaceGroup::closestItem(const QPointF& p, qreal *manhattanLength) const
{
    RegionFrameItem *closestItem = 0;
    qreal minDistance = 0;
    qreal minCenterDistance = 0;
    foreach (RegionFrameItem *item, d->items)
    {
        QRectF r = item->boundingRect();
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

RegionFrameItem *FaceGroup::FaceGroupPriv::createItem(const DatabaseFace &face)
{
    RegionFrameItem* item = new RegionFrameItem(view->previewItem());
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
}

AssignNameWidget *FaceGroup::FaceGroupPriv::createAssignNameWidget(const DatabaseFace &face, const QVariant& identifier)
{
    AssignNameWidget *assignWidget = new AssignNameWidget;
    switch (face.type())
    {
        case DatabaseFace::UnknownName:
            assignWidget->setMode(AssignNameWidget::UnknownName);
            break;
        case DatabaseFace::UnconfirmedName:
            assignWidget->setMode(AssignNameWidget::UnconfirmedName);
            break;
        case DatabaseFace::ConfirmedName:
            assignWidget->setMode(AssignNameWidget::ConfirmedName);
            break;
        default:
            return 0;
    }
    assignWidget->setBackgroundStyle(AssignNameWidget::TransparentRound);
    assignWidget->setLayoutMode(AssignNameWidget::FullLine);
    assignWidget->setFace(info, identifier);
    checkModels();
    assignWidget->setTagModel(tagModel, filterModel);

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

    d->faces = d->faceIface.databaseFaces(d->info.id());

    foreach (const DatabaseFace &face, d->faces)
    {
        RegionFrameItem *item = d->createItem(face);

        // for identification, use index in our list
        AssignNameWidget *assignWidget = d->createAssignNameWidget(face, d->items.size());
        item->setHudWidget(assignWidget);
        new StyleSheetDebugger(assignWidget);

        d->visibilityController->addItem(item);

        d->items << item;
    }

    d->state = FacesLoaded;

    if (d->view->previewItem()->isLoaded())
        d->visibilityController->show();
}

void FaceGroup::clear()
{
    d->visibilityController->clear();
    foreach (RegionFrameItem *item, d->items)
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
    kDebug() << action.tagId() << info.id() << faceIdentifier;

    /*    for(int i = 0; i < d->currentFaces.size(); ++i)
    {
        if(d->currentFaces[i].toRect() == rect)
        {
            d->currentFaces[i].setName(name);
            break;
        }
    }

    d->faceIface->confirmName(getImageInfo().id(), rect, name);
    */
}

void FaceGroup::slotRejected(const ImageInfo& info, const QVariant& faceIdentifier)
{
    kDebug() << info.id() << faceIdentifier;
    /*
    for(int i = 0; i < d->currentFaces.size(); ++i)
    {
        if(d->currentFaces[i].toRect() == rect)
        {
            d->currentFaces.removeAt(i);
            d->faceitems.removeAt(i);
            break;
        }
    }

    d->faceIface->removeFace(getImageInfo().id(), rect);
    */
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

    kDebug()<<"Number of training faces"<<trainList.size();

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

    kDebug()<<"Number of suggestions = "<<recogList.size();
    kDebug()<<"Number of faceitems = "<<d->faceitems.size();
    // Now find the relevant face items and suggest faces
    for(int i = 0; i < recogList.size(); ++i)
    {
        for(int j = 0; j < d->faceitems.size(); ++j)
        {
            if(recogList[i].toRect() == d->faceitems[j]->originalRect())
            {
                kDebug()<<"Suggesting a name "<<recogList[i].name();
                d->faceitems[j]->suggest(recogList[i].name());
                break;
            }
        }
    }
    * /

}
*/

} // namespace Digikam


