/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Managing of face tag region items on a GraphicsDImgView
 *
 * Copyright (C) 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "facegroup.h"

// Qt includes

#include <QGraphicsSceneHoverEvent>
#include <QGraphicsWidget>
#include <QObject>

// Local includes

#include "addtagscombobox.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albummanager.h"
#include "assignnamewidget.h"
#include "clickdragreleaseitem.h"
#include "dimgpreviewitem.h"
#include "facepipeline.h"
#include "facetags.h"
#include "facetagseditor.h"
#include "graphicsdimgview.h"
#include "imageinfo.h"
#include "regionframeitem.h"
#include "taggingaction.h"
#include "itemvisibilitycontroller.h"
#include "digikam_debug.h"

namespace Digikam
{

enum FaceGroupState
{
    NoFaces,
    LoadingFaces,
    FacesLoaded
};

//-------------------------------------------------------------------------------

class FaceItem : public RegionFrameItem
{
public:

    explicit FaceItem(QGraphicsItem* const parent = 0);

    void setFace(const FaceTagsIface& face);
    FaceTagsIface face() const;
    void setHudWidget(AssignNameWidget* const widget);
    AssignNameWidget* widget() const;
    void switchMode(AssignNameWidget::Mode mode);
    void setEditable(bool allowEdit);
    void updateCurrentTag();

protected:

    AssignNameWidget*   m_widget;
    FaceTagsIface        m_face;
    HidingStateChanger* m_changer;
};

//-------------------------------------------------------------------------------

FaceItem::FaceItem(QGraphicsItem* const parent)
    : RegionFrameItem(parent),
      m_widget(0),
      m_changer(0)
{
}

void FaceItem::setFace(const FaceTagsIface& face)
{
    m_face = face;
    updateCurrentTag();
    setEditable(!m_face.isConfirmedName());
}

FaceTagsIface FaceItem::face() const
{
    return m_face;
}

void FaceItem::setHudWidget(AssignNameWidget* const widget)
{
    m_widget = widget;
    updateCurrentTag();
    RegionFrameItem::setHudWidget(widget);
    // Ensure that all HUD widgets are stacked before the frame items
    hudWidget()->setZValue(1);
}

AssignNameWidget* FaceItem::widget() const
{
    return m_widget;
}

void FaceItem::switchMode(AssignNameWidget::Mode mode)
{
    if (!m_widget || m_widget->mode() == mode)
    {
        return;
    }

    if (!m_changer)
    {
        m_changer = new AssignNameWidgetHidingStateChanger(this);
    }

    m_changer->changeValue(mode);
}

void FaceItem::setEditable(bool allowEdit)
{
    changeFlags(ShowResizeHandles | MoveByDrag, allowEdit);
}

void FaceItem::updateCurrentTag()
{
    if (m_widget)
    {
        m_widget->setCurrentFace(m_face);
    }
}

//-------------------------------------------------------------------------------

AssignNameWidgetHidingStateChanger::AssignNameWidgetHidingStateChanger(FaceItem* const item)
    : HidingStateChanger(item->widget(), "mode", item)
{
    // The WidgetProxyItem
    addItem(item->hudWidget());

    connect(this, SIGNAL(stateChanged()),
            this, SLOT(slotStateChanged()));
}

void AssignNameWidgetHidingStateChanger::slotStateChanged()
{
    FaceItem* const item = static_cast<FaceItem*>(parent());
    // Show resize handles etc. only in edit modes
    item->setEditable(item->widget()->mode() != AssignNameWidget::ConfirmedMode);
}

//-------------------------------------------------------------------------------

class FaceGroup::Private
{
public:

    explicit Private(FaceGroup* const q)
        : q(q)
    {
        view                 = 0;
        autoSuggest          = false;
        showOnHover          = false;
        manuallyAddWrapItem  = 0;
        manuallyAddedItem    = 0;
        visibilityController = 0;
        state                = NoFaces;
        tagModel             = 0;
        filterModel          = 0;
        filteredModel        = 0;
    }

    void                       applyVisible();
    FaceItem*                  createItem(const FaceTagsIface& face);
    FaceItem*                  addItem(const FaceTagsIface& face);
    AssignNameWidget*          createAssignNameWidget(const FaceTagsIface& face, const QVariant& identifier);
    AssignNameWidget::Mode     assignWidgetMode(FaceTagsIface::Type type);
    void                       checkModels();
    QList<QGraphicsItem*>      hotItems(const QPointF& scenePos);

public:

    GraphicsDImgView*          view;
    ImageInfo                  info;
    bool                       autoSuggest;
    bool                       showOnHover;

    QList<FaceItem*>           items;

    ClickDragReleaseItem*      manuallyAddWrapItem;
    FaceItem*                  manuallyAddedItem;

    FaceGroupState             state;
    ItemVisibilityController*  visibilityController;

    TagModel*                  tagModel;
    CheckableAlbumFilterModel* filterModel;
    TagPropertiesFilterModel*  filteredModel;

    FacePipeline               editPipeline;

    FaceGroup* const           q;
};

FaceGroup::FaceGroup(GraphicsDImgView* const view)
    : QObject(view),
      d(new Private(this))
{
    d->view                 = view;
    d->visibilityController = new ItemVisibilityController(this);
    d->visibilityController->setShallBeShown(false);

    connect(view->previewItem(), SIGNAL(stateChanged(int)),
            this, SLOT(itemStateChanged(int)));

    d->editPipeline.plugDatabaseEditor();
    d->editPipeline.plugTrainer();
    d->editPipeline.construct();
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
            {
                d->visibilityController->show();
            }

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

    foreach(FaceItem* const item, d->items)
    {
        items << item;
    }

    return items;
}

void FaceGroup::setAutoSuggest(bool doAutoSuggest)
{
    if (d->autoSuggest == doAutoSuggest)
    {
        return;
    }

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

void FaceGroup::Private::applyVisible()
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
        {
            visibilityController->show();
        }
    }
}

void FaceGroup::setVisible(bool visible)
{
    d->visibilityController->setShallBeShown(visible);
    d->applyVisible();
}

void FaceGroup::setVisibleItem(RegionFrameItem* item)
{
    d->visibilityController->setItemThatShallBeShown(item);
    d->applyVisible();
}

void FaceGroup::setInfo(const ImageInfo& info)
{
    if (d->info == info && d->state != NoFaces)
    {
        return;
    }

    clear();
    d->info = info;

    if (d->visibilityController->shallBeShown())
    {
        load();
    }
}

void FaceGroup::aboutToSetInfo(const ImageInfo& info)
{
    if (d->info == info)
    {
        return;
    }

    applyItemGeometryChanges();
    clear();
}

void FaceGroup::aboutToSetInfoAfterRotate(const ImageInfo& info)
{
    if (d->info == info)
    {
        return;
    }

    //applyItemGeometryChanges();
    clear();
}

static QPointF closestPointOfRect(const QPointF& p, const QRectF& r)
{
    QPointF cp = p;

    if (p.x() < r.left())
    {
        cp.setX(r.left());
    }
    else if (p.x() > r.right())
    {
        cp.setX(r.right());
    }

    if (p.y() < r.top())
    {
        cp.setY(r.top());
    }
    else if (p.y() > r.bottom())
    {
        cp.setY(r.bottom());
    }

    return cp;
}

RegionFrameItem* FaceGroup::closestItem(const QPointF& p, qreal* const manhattanLength) const
{
    RegionFrameItem* closestItem = 0;
    qreal minDistance            = 0;
    qreal minCenterDistance      = 0;

    foreach(RegionFrameItem* const item, d->items)
    {
        QRectF r       = item->boundingRect().translated(item->pos());
        qreal distance = (p - closestPointOfRect(p, r)).manhattanLength();

        if (!closestItem || distance < minDistance || (distance == 0 && (p - r.center()).manhattanLength() < minCenterDistance))
        {
            closestItem = item;
            minDistance = distance;

            if (distance == 0)
            {
                minCenterDistance = (p - r.center()).manhattanLength();
            }
        }
    }

    if (manhattanLength)
    {
        *manhattanLength = minDistance;
    }
    return closestItem;
}

QList<QGraphicsItem*> FaceGroup::Private::hotItems(const QPointF& scenePos)
{
    if (!q->hasVisibleItems())
    {
        return QList<QGraphicsItem*>();
    }

    const int distance               = 15;
    QRectF hotSceneRect              = QRectF(scenePos, QSize(0, 0)).adjusted(-distance, -distance, distance, distance);
    QList<QGraphicsItem*> closeItems = view->scene()->items(hotSceneRect, Qt::IntersectsItemBoundingRect);

    closeItems.removeOne(view->previewItem());

    return closeItems;

/*
    qreal distance;
    d->faceGroup->closestItem(mapToScene(e->pos()), &distance);
    if (distance < 15)
        return false;
*/
}

bool FaceGroup::acceptsMouseClick(const QPointF& scenePos)
{
    return d->hotItems(scenePos).isEmpty();
}

void FaceGroup::itemHoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    if (d->showOnHover && !isVisible())
    {
        qreal distance;
        RegionFrameItem* const item = closestItem(e->scenePos(), &distance);

        // There's a possible nuisance when the direct mouse way from hovering pos to HUD widget
        // is not part of the condition. Maybe, we should add a exemption for this case.
        if (distance < 25)
        {
            setVisibleItem(item);
        }
        else
        {
            // get all items close to pos
            QList<QGraphicsItem*> hotItems = d->hotItems(e->scenePos());
            // this will be the one item shown by mouse over
            QList<QObject*> visible        = d->visibilityController->visibleItems(ItemVisibilityController::ExcludeFadingOut);

            foreach(QGraphicsItem* const item, hotItems)
            {
                foreach(QObject* const parent, visible)
                {
                    if (static_cast<QGraphicsObject*>(parent)->isAncestorOf(item))
                    {
                        return;
                    }
                }
            }

            setVisibleItem(0);
        }
    }
}

void FaceGroup::itemHoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
}

void FaceGroup::itemHoverEnterEvent(QGraphicsSceneHoverEvent*)
{
}

void FaceGroup::leaveEvent(QEvent*)
{
    if (d->showOnHover && !isVisible())
    {
        setVisibleItem(0);
    }
}

void FaceGroup::enterEvent(QEvent*)
{
}

FaceItem* FaceGroup::Private::createItem(const FaceTagsIface& face)
{
    FaceItem* const item = new FaceItem(view->previewItem());
    item->setFace(face);
    item->setOriginalRect(face.region().toRect());
    item->setVisible(false);

    q->connect(view, SIGNAL(viewportRectChanged(QRectF)),
               item, SLOT(setViewportRect(QRectF)));

    return item;
}

FaceItem* FaceGroup::Private::addItem(const FaceTagsIface& face)
{
    FaceItem* const item                 = createItem(face);
    // for identification, use index in our list

    AssignNameWidget* const assignWidget = createAssignNameWidget(face, items.size());
    item->setHudWidget(assignWidget);
    //new StyleSheetDebugger(assignWidget);

    visibilityController->addItem(item);

    items << item;
    return item;
}

void FaceGroup::Private::checkModels()
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

AssignNameWidget::Mode FaceGroup::Private::assignWidgetMode(FaceTagsIface::Type type)
{
    switch (type)
    {
        case FaceTagsIface::UnknownName:
        case FaceTagsIface::UnconfirmedName:
            return AssignNameWidget::UnconfirmedEditMode;

        case FaceTagsIface::ConfirmedName:
            return AssignNameWidget::ConfirmedMode;

        default:
            return AssignNameWidget::InvalidMode;
    }
}

AssignNameWidget* FaceGroup::Private::createAssignNameWidget(const FaceTagsIface& face, const QVariant& identifier)
{
    AssignNameWidget* const assignWidget = new AssignNameWidget(view);
    assignWidget->setMode(assignWidgetMode(face.type()));
    assignWidget->setTagEntryWidgetMode(AssignNameWidget::AddTagsComboBoxMode);
    assignWidget->setVisualStyle(AssignNameWidget::TranslucentDarkRound);
    assignWidget->setLayoutMode(AssignNameWidget::TwoLines);
    assignWidget->setUserData(info, identifier);
    checkModels();
    assignWidget->setModel(tagModel, filteredModel, filterModel);
    assignWidget->setParentTag(AlbumManager::instance()->findTAlbum(FaceTags::personParentTag()));

    q->connect(assignWidget, SIGNAL(assigned(TaggingAction,ImageInfo,QVariant)),
               q, SLOT(slotAssigned(TaggingAction,ImageInfo,QVariant)));

    q->connect(assignWidget, SIGNAL(rejected(ImageInfo,QVariant)),
               q, SLOT(slotRejected(ImageInfo,QVariant)));

    q->connect(assignWidget, SIGNAL(labelClicked(ImageInfo,QVariant)),
               q, SLOT(slotLabelClicked(ImageInfo,QVariant)));

    return assignWidget;
}

void FaceGroup::load()
{
    if (d->state != NoFaces)
    {
        return;
    }

    d->state = LoadingFaces;

    if (d->info.isNull())
    {
        d->state = FacesLoaded;
        return;
    }

    QList<FaceTagsIface> faces = FaceTagsEditor().databaseFaces(d->info.id());
    d->visibilityController->clear();

    foreach(const FaceTagsIface& face, faces)
    {
        d->addItem(face);
    }

    d->state = FacesLoaded;

    if (d->view->previewItem()->isLoaded())
    {
        d->visibilityController->show();
    }
}

void FaceGroup::clear()
{
    cancelAddItem();
    d->visibilityController->clear();

    foreach(RegionFrameItem* const item, d->items)
    {
        delete item;
    }

    d->items.clear();
    d->state = NoFaces;
}

void FaceGroup::rejectAll()
{
    foreach(FaceItem* const item, d->items)
    {
        d->editPipeline.remove(d->info, item->face());
        item->setFace(FaceTagsIface());
        d->visibilityController->hideAndRemoveItem(item);
    }

    clear();
}

void FaceGroup::slotAssigned(const TaggingAction& action, const ImageInfo&, const QVariant& faceIdentifier)
{
    FaceItem* const item    = d->items[faceIdentifier.toInt()];
    FaceTagsIface face      = item->face();
    TagRegion currentRegion = TagRegion(item->originalRect());

    if (!face.isConfirmedName() || face.region() != currentRegion || action.shallCreateNewTag() || (action.shallAssignTag() && action.tagId() != face.tagId()))
    {
        int tagId = 0;

        if (action.shallAssignTag())
        {
            tagId = action.tagId();
        }
        else if (action.shallCreateNewTag())
        {
            tagId = FaceTags::getOrCreateTagForPerson(action.newTagName(), action.parentTagId());
        }

        if (FaceTags::isTheUnknownPerson(tagId))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Refusing to assign the unknown person to an image";
            return;
        }

        if (!tagId)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to get person tag";
            return;
        }

        if (tagId)
        {
            face = d->editPipeline.confirm(d->info, face, d->view->previewItem()->image(), tagId, currentRegion);
        }
    }

    item->setFace(face);
    item->switchMode(AssignNameWidget::ConfirmedMode);
}

void FaceGroup::slotRejected(const ImageInfo&, const QVariant& faceIdentifier)
{
    FaceItem* const item = d->items[faceIdentifier.toInt()];
    d->editPipeline.remove(d->info, item->face());

    item->setFace(FaceTagsIface());
    d->visibilityController->hideAndRemoveItem(item);
}

void FaceGroup::slotLabelClicked(const ImageInfo&, const QVariant& faceIdentifier)
{
    FaceItem* const item = d->items[faceIdentifier.toInt()];
    item->switchMode(AssignNameWidget::ConfirmedEditMode);
}

void FaceGroup::startAutoSuggest()
{
    if (!d->autoSuggest)
    {
        return;
    }
}

void FaceGroup::addFace()
{
    if (d->manuallyAddWrapItem)
    {
        return;
    }

    d->manuallyAddWrapItem = new ClickDragReleaseItem(d->view->previewItem());
    d->manuallyAddWrapItem->setFocus();
    d->view->setFocus();

    connect(d->manuallyAddWrapItem, SIGNAL(started(QPointF)),
            this, SLOT(slotAddItemStarted(QPointF)));

    connect(d->manuallyAddWrapItem, SIGNAL(moving(QRectF)),
            this, SLOT(slotAddItemMoving(QRectF)));

    connect(d->manuallyAddWrapItem, SIGNAL(finished(QRectF)),
            this, SLOT(slotAddItemFinished(QRectF)));

    connect(d->manuallyAddWrapItem, SIGNAL(cancelled()),
            this, SLOT(cancelAddItem()));
}

void FaceGroup::slotAddItemStarted(const QPointF& pos)
{
    Q_UNUSED(pos);
}

void FaceGroup::slotAddItemMoving(const QRectF& rect)
{
    if (!d->manuallyAddedItem)
    {
        d->manuallyAddedItem = d->createItem(FaceTagsIface());
        d->visibilityController->addItem(d->manuallyAddedItem);
        d->visibilityController->showItem(d->manuallyAddedItem);
    }

    d->manuallyAddedItem->setRectInSceneCoordinatesAdjusted(rect);
}

void FaceGroup::slotAddItemFinished(const QRectF& rect)
{
    if (d->manuallyAddedItem)
    {
        d->manuallyAddedItem->setRectInSceneCoordinatesAdjusted(rect);
        FaceTagsIface face   = d->editPipeline.addManually(d->info, d->view->previewItem()->image(),
                                                           TagRegion(d->manuallyAddedItem->originalRect()));
        FaceItem* const item = d->addItem(face);
        d->visibilityController->setItemDirectlyVisible(item, true);
        item->switchMode(AssignNameWidget::UnconfirmedEditMode);
        d->manuallyAddWrapItem->stackBefore(item);
    }

    cancelAddItem();
}

void FaceGroup::cancelAddItem()
{
    delete d->manuallyAddedItem;
    d->manuallyAddedItem = 0;

    if (d->manuallyAddWrapItem)
    {
        d->view->scene()->removeItem(d->manuallyAddWrapItem);
        d->manuallyAddWrapItem->deleteLater();
        d->manuallyAddWrapItem = 0;
    }
}

void FaceGroup::applyItemGeometryChanges()
{
    foreach(FaceItem* const item, d->items)
    {
        if (item->face().isNull())
        {
            continue;
        }

        TagRegion currentRegion = TagRegion(item->originalRect());

        if (item->face().region() != currentRegion)
        {
            d->editPipeline.editRegion(d->info, d->view->previewItem()->image(), item->face(), currentRegion);
        }
    }
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

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of training faces" << trainList.size();

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

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of suggestions = " << recogList.size();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of faceitems = " << d->faceitems.size();

    // Now find the relevant face items and suggest faces
    for(int i = 0; i < recogList.size(); ++i)
    {
        for(int j = 0; j < d->faceitems.size(); ++j)
        {
            if(recogList[i].toRect() == d->faceitems[j]->originalRect())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Suggesting a name " << recogList[i].name();
                d->faceitems[j]->suggest(recogList[i].name());
                break;
            }
        }
    }
    * /
}
*/

} // namespace Digikam
