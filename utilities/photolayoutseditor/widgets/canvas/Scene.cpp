/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "Scene.h"

// C++ std includes

#include <limits>

// Qt
#include <QGraphicsTextItem>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>
#include <QGraphicsSceneDragDropEvent>
#include <QStyleOptionGraphicsItem>
#include <QMap>
#include <QGraphicsWidget>
#include <qmath.h>
#include <QUndoCommand>
#include <QImageReader>
#include <QtAlgorithms>
#include <QBuffer>
#include <QUrl>
#include <QApplication>
#include <QMimeData>
#include <QTemporaryFile>

// KDE
#include <klocalizedstring.h>

#include "global.h"
#include "RotationWidgetItem.h"
#include "ScalingWidgetItem.h"
#include "CropWidgetItem.h"
#include "SceneBackground.h"
#include "MousePressListener.h"
#include "ToolsDockWidget.h"
#include "photolayoutswindow.h"
#include "PLEConfigSkeleton.h"
#include "ImageLoadingThread.h"
#include "ProgressEvent.h"
#include "CanvasLoadingThread.h"
#include "PhotoItem.h"
#include "SceneBorder.h"
#include "digikam_debug.h"
#include "imagedialog.h"
#include "LayersModel.h"
#include "LayersModelItem.h"
#include "LayersSelectionModel.h"

using namespace Digikam;

namespace PhotoLayoutsEditor
{

QColor Scene::OUTSIDE_SCENE_COLOR;

class ScenePrivate
{
    ScenePrivate(Scene * scene) :
        m_scene(scene),
        model(new LayersModel(scene)),
        selection_model(new LayersSelectionModel(model, scene)),
        m_pressed_object(0),
        m_pressed_item(0),
        m_selected_items_all_movable(true),
        m_selection_visible(true),
        m_rot_item(0),
        m_scale_item(0),
        m_crop_item(0),
//        m_blend_active(false),
        m_readSceneMousePress_listener(0),
        m_readSceneMousePress_enabled(false),
        m_hovered_photo(0)
    {
        // Background of the scene
        m_background = new SceneBackground(m_scene);
        // Border of the scene
        m_border = new SceneBorder(m_scene);
    }

    QList<QGraphicsItem*> itemsAtPosition(const QPointF & scenePos, QWidget * widget)
    {
        QGraphicsView * view = widget ? qobject_cast<QGraphicsView*>(widget->parentWidget()) : 0;
        if (!view)
            return m_scene->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
        const QRectF pointRect(scenePos, QSizeF(1, 1));
        if (!view->isTransformed())
            return m_scene->items(pointRect, Qt::IntersectsItemShape, Qt::DescendingOrder);
        const QTransform viewTransform = view->viewportTransform();
        return m_scene->items(pointRect, Qt::IntersectsItemShape, Qt::DescendingOrder, viewTransform);
    }

    AbstractItemInterface * itemAt(const QPointF & scenePos, QWidget * widget)
    {
        QList<QGraphicsItem*> items = itemsAtPosition(scenePos, widget);
        return items.count() ? dynamic_cast<AbstractItemInterface*>(items.first()) : 0;
    }

    QList<AbstractItemInterface*> itemsAt(const QPointF & scenePos, QWidget * widget)
    {
        QList<QGraphicsItem*> items = itemsAtPosition(scenePos, widget);
        QList<AbstractItemInterface*> r;
        foreach(QGraphicsItem* i, items)
        {
            AbstractItemInterface * iface = dynamic_cast<AbstractItemInterface*>(i);
            if (iface)
                r.push_back(iface);
        }
        return r;
    }

    void sendPressEventToItem(AbstractItemInterface * item, QGraphicsSceneMouseEvent * event)
    {
        if (!item)
            return;

        // Send mousepressevent to the pressed item
        event->setPos(item->mapFromScene(event->scenePos()));
        event->setButtonDownPos(event->button(),
                                item->mapFromScene(event->buttonDownScenePos(event->button())));
        event->setLastPos(item->mapFromScene(event->lastScenePos()));
        item->mousePressEvent(event);
    }

    void sendMoveEventToItem(AbstractItemInterface * item, QGraphicsSceneMouseEvent * event)
    {
        if (!item)
            return;

        // Send mousepressevent to the pressed item
        event->setPos(item->mapFromScene(event->scenePos()));
        event->setButtonDownPos(event->button(),
                                item->mapFromScene(event->buttonDownScenePos(event->button())));
        event->setLastPos(item->mapFromScene(event->lastScenePos()));
        item->mouseMoveEvent(event);
    }

    void sendReleaseEventToItem(AbstractItemInterface * item, QGraphicsSceneMouseEvent * event)
    {
        if (!item)
            return;

        // Send mousepressevent to the pressed item
        event->setPos(item->mapFromScene(event->scenePos()));
        event->setButtonDownPos(event->button(),
                                item->mapFromScene(event->buttonDownScenePos(event->button())));
        event->setLastPos(item->mapFromScene(event->lastScenePos()));
        item->mouseReleaseEvent(event);
    }

    void sendDoubleClickEventToItem(AbstractItemInterface * item, QGraphicsSceneMouseEvent * event)
    {
        if (!item)
            return;

        // Send mousepressevent to the pressed item
        event->setPos(item->mapFromScene(event->scenePos()));
        event->setButtonDownPos(event->button(),
                                item->mapFromScene(event->buttonDownScenePos(event->button())));
        event->setLastPos(item->mapFromScene(event->lastScenePos()));
        item->mouseDoubleClickEvent(event);
    }

    // Used for selecting items
    void deselectSelected()
    {
        m_selected_items_all_movable = true;
        foreach(AbstractItemInterface* photo, m_selected_items.keys())
        {
            photo->setSelected(false);
            if (photo->hasFocus())
                photo->clearFocus();
        }
        m_selected_items.clear();
        m_selected_items_path = QPainterPath();
    }

    bool selectPressed()
    {
        if (m_pressed_item)
        {
            // Select if not selested
            if (!m_pressed_item->isSelected())
            {
                m_selected_items.insert(m_pressed_item, m_pressed_item->pos());
                m_selected_items_path        = m_selected_items_path.united(m_pressed_item->mapToScene(m_pressed_item->shape()));
                m_selected_items_all_movable = ((m_pressed_item->flags() & QGraphicsItem::ItemIsMovable) != 0) && m_selected_items_all_movable;
                m_pressed_item->setSelected(true);
                setSelectionInitialPosition();
            }
            return true;
        }
        return false;
    }

    void focusPressed()
    {
        if (!m_pressed_item)
            return;
        // If is selectable and focusable try to set focus and post mousepressevent to it
        if (m_pressed_item->flags() & QGraphicsItem::ItemIsFocusable)
            m_pressed_item->setFocus(Qt::MouseFocusReason);
    }

    void setSelectionInitialPosition()
    {
        QMap<AbstractPhoto*,QPointF>::iterator it = m_selected_items.begin();
        while (it != m_selected_items.end())
        {
            it.value() = it.key()->pos();
            ++it;
        }
        m_selected_items_path_initial_pos = m_selected_items_path.boundingRect().topLeft();
    }

    bool wasMoved()
    {
        QMap<AbstractPhoto*,QPointF>::iterator it = m_selected_items.begin();
        while (it != m_selected_items.end())
        {
            if (it.value() != it.key()->pos())
                return true;
            ++it;
        }
        return false;
    }

    // Parent scene
    QGraphicsScene*              m_scene;
    // Scene's model
    LayersModel*                 model;
    LayersSelectionModel*        selection_model;
    // Background item
    SceneBackground*             m_background;
    // Border item
    SceneBorder*                 m_border;

    QMap<AbstractPhoto*,QPointF> m_selected_items;
    AbstractItemInterface*       m_pressed_object;
    AbstractPhoto*               m_pressed_item;
    QPainterPath                 m_selected_items_path;
    QPointF                      m_selected_items_path_initial_pos;
    bool                         m_selected_items_all_movable;
    bool                         m_selection_visible;
    QList<const char*>           m_selection_filters;
    QPointF                      paste_scene_pos;

    // Used for rotating items
    RotationWidgetItem*          m_rot_item;

    // Used for scaling item
    ScalingWidgetItem*           m_scale_item;

    // Used for cropping items
    CropWidgetItem*              m_crop_item;
//    bool                         m_blend_active;

    // For reading mouse press
    MousePressListener*          m_readSceneMousePress_listener;
    bool                         m_readSceneMousePress_enabled;

    // Used for drag&drop images
    PhotoItem*                   m_hovered_photo;

    friend class Scene;
};

class AddItemsCommand
    : public QUndoCommand
{
        QList<AbstractPhoto*> items;
        int position;
        Scene * scene;
        bool done;
    public:
        AddItemsCommand(AbstractPhoto * item, int position, Scene * scene, QUndoCommand * parent = 0) :
            QUndoCommand(i18n("Add item"), parent),
            position(position),
            scene(scene),
            done(false)
        {
            items << item;
        }
        AddItemsCommand(const QList<AbstractPhoto*> & items, int position, Scene * scene, QUndoCommand * parent = 0) :
            QUndoCommand(i18np("Add item", "Add items", items.count()), parent),
            items(items),
            position(position),
            scene(scene),
            done(false)
        {}
        ~AddItemsCommand()
        {
            if (done)
                return;
            foreach(AbstractPhoto* item, items)
                item->deleteLater();
            items.clear();
        }
        virtual void redo()
        {
            foreach(AbstractPhoto* item, items)
                scene->QGraphicsScene::addItem(item);
            scene->model()->insertItems(items, position);
            done = true;
        }
        virtual void undo()
        {
            QRectF region;
            foreach(AbstractPhoto* item, items)
            {
                region = region.united( item->mapRectToScene(item->boundingRect()) );
                if (item->isSelected())
                    item->setSelected(false);
                scene->QGraphicsScene::removeItem(item);
            }
            scene->model()->removeRows(position, items.count());
            scene->update(region);
            done = false;
        }
};

class MoveItemsCommand
    : public QUndoCommand
{
        QMap<AbstractPhoto*,QPointF> m_items;
        Scene * m_scene;
        bool done;
    public:
        MoveItemsCommand(QMap<AbstractPhoto*,QPointF> items, Scene * scene, QUndoCommand * parent = 0) :
            QUndoCommand(i18np("Move item", "Move items", items.count()), parent),
            m_items(items),
            m_scene(scene),
            done(true)
        {}
        virtual void redo()
        {
            if (!done)
            {
                QMap<AbstractPhoto*,QPointF>::iterator it = m_items.begin();
                while(it != m_items.end())
                {
                    QPointF temp = it.key()->pos();
                    it.key()->setPos( it.value() );
                    it.value() = temp;
                    ++it;
                }
                done = !done;
                m_scene->calcSelectionBoundingRect();
            }
        }
        virtual void undo()
        {
            if (done)
            {
                QMap<AbstractPhoto*,QPointF>::iterator it = m_items.begin();
                while(it != m_items.end())
                {
                    QPointF temp = it.key()->pos();
                    it.key()->setPos( it.value() );
                    it.value() = temp;
                    ++it;
                }
                done = !done;
                m_scene->calcSelectionBoundingRect();
            }
        }
};

class RemoveItemsCommand
    : public QUndoCommand
{
        AbstractPhoto* item;
        int            item_row;
        AbstractPhoto* item_parent;
        Scene*         m_scene;
        bool           done;

    public:

        RemoveItemsCommand(AbstractPhoto * item, Scene * scene, QUndoCommand * parent = 0) :
            QUndoCommand(QLatin1String("Remove item"), parent),
            item(item),
            item_row(0),
            m_scene(scene),
            done(false)
        {
            item_parent = dynamic_cast<AbstractPhoto*>(item->parentItem());
        }

        ~RemoveItemsCommand()
        {
            if (done)
            {
                if (item && !item->scene() && !item->parentItem())
                    delete item;
            }
        }

        virtual void redo()
        {
            QPersistentModelIndex parentIndex = QPersistentModelIndex(m_scene->model()->findIndex( item_parent ));
            if (item_parent && !(parentIndex.isValid() && item_parent->scene()))
                return;

            // Remove from model
            QModelIndex itemIndex = m_scene->model()->findIndex(item, parentIndex);
            item_row = itemIndex.row();
            if (itemIndex.isValid())
                m_scene->model()->removeRow(item_row, parentIndex);

            // Remove from scene
            if (item->scene() == m_scene)
                m_scene->QGraphicsScene::removeItem(item);
            done = true;
        }

        virtual void undo()
        {
            if (!done)
                return;

            // Add to scene
            if (item->scene() != m_scene)
                m_scene->QGraphicsScene::addItem( item );
            item->setParentItem( item_parent );

            // Add to model
            QPersistentModelIndex parentIndex = QPersistentModelIndex( m_scene->model()->findIndex( item_parent ) );
            if (!m_scene->model()->hasIndex(item_row, 0, parentIndex) ||
                    static_cast<LayersModelItem*>(m_scene->model()->index(item_row, 0, parentIndex).internalPointer())->photo() != item)
            {
                if (m_scene->model()->insertRow(item_row, parentIndex))
                {
                    static_cast<LayersModelItem*>(m_scene->model()->index(item_row, 0, parentIndex).internalPointer())->setPhoto(item);
                    // Add items children to model
                    appendChild(item, m_scene->model()->index(item_row, 0, parentIndex));
                }
            }
            done = false;
        }

    private:

        static bool compareGraphicsItems(QGraphicsItem * i1, QGraphicsItem * i2)
        {
            if ((i1 && i2) && (i1->zValue() < i2->zValue()))
                return true;
            return false;
        }

        void appendChild(AbstractPhoto * item, const QModelIndex & parent)
        {
            QList<QGraphicsItem*> items = item->childItems();
            if (items.count())
            {
                // Sort using z-Values (z-Value == models row)
                qSort(items.begin(), items.end(), PhotoLayoutsEditor::RemoveItemsCommand::compareGraphicsItems);
                int i = 0;
                foreach(QGraphicsItem* childItem, items)
                {
                    AbstractPhoto * photo = dynamic_cast<AbstractPhoto*>(childItem);
                    if (photo)
                    {
                        if (m_scene->model()->insertRow(i,parent))
                        {
                            static_cast<LayersModelItem*>(m_scene->model()->index(i, 0, parent).internalPointer())->setPhoto(photo);
                            this->appendChild(photo, m_scene->model()->index(i, 0, parent));
                            ++i;
                        }
                    }
                }
            }
        }
};

class CropItemsCommand
    : public QUndoCommand
{
    QMap<AbstractPhoto*,QPainterPath> data;
public:
    CropItemsCommand(const QPainterPath & path, const QList<AbstractPhoto*> & items, QUndoCommand * parent = 0) :
        QUndoCommand(i18np("Crop item", "Crop items", items.count()), parent)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "scene crop shape" << path.boundingRect();
        foreach(AbstractPhoto* item, items)
            data.insert(item, item->mapFromScene(path));
    }
    virtual void redo()
    {
        this->run();
    }
    virtual void undo()
    {
        this->run();
    }
private:
    void run()
    {
        for (QMap<AbstractPhoto*,QPainterPath>::iterator it = data.begin(); it != data.end(); ++it)
        {
            QPainterPath temp = it.key()->cropShape();
            it.key()->setCropShape( it.value() );
            it.value() = temp;
        }
    }
};

Scene::Scene(const QRectF & dimension, QObject * parent) :
    QGraphicsScene(dimension, parent),
    d(new ScenePrivate(this)),
    x_grid(0),
    y_grid(0),
    grid_item(0),
    grid_changed(true)
{
    if (!OUTSIDE_SCENE_COLOR.isValid())
    {
        QPalette pal = this->palette();
        OUTSIDE_SCENE_COLOR = pal.color(QPalette::Window);
        OUTSIDE_SCENE_COLOR.setAlpha(190);
    }

    this->setBackgroundBrush(Qt::transparent);

    // Mouse interaction mode
    setInteractionMode(DEFAULT_EDITING_MODE);

    // Create default grid
    setGrid(PLEConfigSkeleton::self()->horizontalGrid(), PLEConfigSkeleton::self()->verticalGrid());
    grid_visible = !PLEConfigSkeleton::self()->showGrid();
    setGridVisible(PLEConfigSkeleton::self()->showGrid());

    // Indexing method
    this->setItemIndexMethod(QGraphicsScene::NoIndex);

    // Signal connections
    connect(this, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
    connect(PLEConfigSkeleton::self(), SIGNAL(showGridChanged(bool)), this, SLOT(setGridVisible(bool)));
    connect(PLEConfigSkeleton::self(), SIGNAL(horizontalGridChanged(double)), this, SLOT(setHorizontalGrid(double)));
    connect(PLEConfigSkeleton::self(), SIGNAL(verticalGridChanged(double)), this, SLOT(setVerticalGrid(double)));
}


Scene::~Scene()
{
    delete d;
}


SceneBackground * Scene::background()
{
    return d->m_background;
}


SceneBorder * Scene::border()
{
    return d->m_border;
}


LayersModel * Scene::model() const
{
    return d->model;
}


LayersSelectionModel * Scene::selectionModel() const
{
    return d->selection_model;
}


void Scene::addItem(AbstractPhoto * item)
{
    // Prevent multiple addition of the item
    if (item->scene() == this)
        return;

    QModelIndexList selectedIndexes = d->selection_model->selectedIndexes();
    unsigned insertionRow = -1;
    foreach(QModelIndex index, selectedIndexes)
    {
        if (index.column() != LayersModelItem::NameString)
            continue;
        if (insertionRow > (unsigned)index.row())
            insertionRow = index.row();
    }

    if (insertionRow == (unsigned)-1)
        insertionRow = 0;

    QUndoCommand * command = new AddItemsCommand(item, insertionRow, this);
    PLE_PostUndoCommand(command);
}

void Scene::addItems(const QList<AbstractPhoto*> & items)
{
    // Prevent multiple addition of the item
    QList<AbstractPhoto*> tempItems;
    foreach(AbstractPhoto* item, items)
    {
        if (item->scene() == this && this->model()->findIndex(item).isValid())
            continue;
        tempItems.prepend(item);
    }
    if (tempItems.count() < 1)
        return;

    QModelIndexList selectedIndexes = d->selection_model->selectedIndexes();
    unsigned insertionRow = -1;
    foreach(QModelIndex index, selectedIndexes)
    {
        if (index.column() != LayersModelItem::NameString)
            continue;
        if (insertionRow > (unsigned)index.row())
            insertionRow = index.row();
    }

    if (insertionRow == (unsigned)-1)
        insertionRow = 0;

    QUndoCommand * parent = 0;
    QUndoCommand * command = 0;
    if (items.count() > 1)
        parent = new QUndoCommand( i18np("Add item", "Add items", items.count()) );

    foreach(AbstractPhoto* item, tempItems)
        command = new AddItemsCommand(item, insertionRow++, this, parent);

    if (parent)
        PLE_PostUndoCommand(parent);
    else if (command)
        PLE_PostUndoCommand(command);
}

void Scene::removeItem(AbstractPhoto * item)
{
    if (!askAboutRemoving(1))
        return;
    QUndoCommand * command = new RemoveItemsCommand(item, this);
    PLE_PostUndoCommand(command);
}

void Scene::removeItems(const QList<AbstractPhoto *> & items)
{
    if (!askAboutRemoving(items.count()))
        return;
    QUndoCommand * command = 0;
    QUndoCommand * parent = 0;
    if (items.count() > 1)
        parent = new QUndoCommand( i18np("Remove item", "Remove items", items.count()) );
    foreach (AbstractPhoto * item, items)
        command = new RemoveItemsCommand(item, this, parent);
    if (parent)
        PLE_PostUndoCommand(parent);
    else
        PLE_PostUndoCommand(command);
}

void Scene::removeSelectedItems()
{
    removeItems(selectedItems());
}

void Scene::changeSelectedImage()
{
    QList<AbstractPhoto*> items = selectedItems();
    if (items.count() != 1)
        return;
    PhotoItem * item = dynamic_cast<PhotoItem*>(items.first());
    if (!item)
        return;

    QUrl url = ImageDialog::getImageURL(PhotoLayoutsWindow::instance(), QUrl());
    if (url.isEmpty())
        return;

    ImageLoadingThread * ilt = new ImageLoadingThread(this);
    ilt->setImageUrl(url);
    ilt->setMaximumProgress(1);
    connect(ilt, SIGNAL(imageLoaded(QUrl,QImage)), item, SLOT(imageLoaded(QUrl,QImage)));
    ilt->start();
}


void Scene::contextMenuEvent(QGraphicsSceneMouseEvent * event)
{
    QMenu menu;

    // Remove items
    QList<AbstractPhoto*> items = this->selectedItems();
    if (items.count())
    {
        if (items.count() == 1)
        {
            PhotoItem * item = dynamic_cast<PhotoItem*>(items.first());
            if (item)
            {
                QAction * removeAction = menu.addAction( i18n("Change item's image") );
                connect(removeAction, SIGNAL(triggered()), this, SLOT(changeSelectedImage()));
            }
        }

        QAction * removeAction = menu.addAction( i18np("Delete selected item", "Delete selected items", items.count()) );
        connect(removeAction, SIGNAL(triggered()), this, SLOT(removeSelectedItems()));
        menu.addSeparator();
    }

    // Background
    QAction * background = menu.addAction( i18n("Canvas background") );
    connect(background, SIGNAL(triggered()), ToolsDockWidget::instance(), SLOT(setCanvasWidgetVisible()));

    menu.exec(event->screenPos());
}


void Scene::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
    this->contextMenuEvent( (QGraphicsSceneMouseEvent*) event );
}


void Scene::keyPressEvent(QKeyEvent * event)
{
    if (this->focusItem())
    {
        QGraphicsScene::keyPressEvent(event);
        event->setAccepted(true);
        return;
    }
    switch(event->key())
    {
        case Qt::Key_Delete:
            this->removeItems(selectedItems());
            event->setAccepted(true);
            break;
        case Qt::Key_Escape:
            //disableitemsDrawing();
            break;
    }
    if (event->isAccepted())
        return;
    QGraphicsScene::keyPressEvent(event);
}


void Scene::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Return mouse position to the listener
        if (d->m_readSceneMousePress_enabled)
        {
            d->m_readSceneMousePress_listener->mousePress(event->scenePos());
            event->setAccepted(true);
            return;
        }

        // If moving enabled
        if (m_interaction_mode & Selecting)
        {
            this->calcSelectionBoundingRect();

            // Get initial selection position
            d->setSelectionInitialPosition();

            // If single selection mode, clear CTRL modifier
            if (selectionMode & SingleSelection)
                event->setModifiers(QFlags<Qt::KeyboardModifier>(((int)event->modifiers()) & ~Qt::ControlModifier));

            // Get items under mouse
            d->m_pressed_object = d->m_pressed_item = 0;
            QList<AbstractItemInterface*> itemsList = d->itemsAt(event->scenePos(), event->widget());

            foreach(AbstractItemInterface* i, itemsList)
            {
                // Get pressed item
                d->m_pressed_object = i;
                // Test if this is a photo/text item
                d->m_pressed_item = dynamic_cast<AbstractPhoto*>(d->m_pressed_object);

                // If it is rotation widget
                if ((m_interaction_mode & Rotating) && d->m_pressed_object == d->m_rot_item)
                {
                    d->sendPressEventToItem(d->m_pressed_object, event);
                    if (event->isAccepted())
                        return;
                }
                // If it is scaling widget
                else if ((m_interaction_mode & Scaling) && d->m_pressed_object == d->m_scale_item)
                {
                    d->sendPressEventToItem(d->m_pressed_object, event);
                    if (event->isAccepted())
                        return;
                }
                // If it is cropping widget
                else if ((m_interaction_mode & Cropping) && d->m_pressed_object == d->m_crop_item)
                {
                    d->sendPressEventToItem(d->m_pressed_object, event);
                    if (event->isAccepted())
                        return;
                }
                else
                {
                    break;
                }
            }

            // If event pos is not in current selection shape...
            if (!d->m_selected_items_path.contains(event->scenePos()) || !d->m_selected_items.contains(dynamic_cast<AbstractPhoto*>(d->m_pressed_item)))
            {
                // Clear focus from focused items
                if (this->focusItem())
                    this->focusItem()->clearFocus();
                // Clear this selection
                if (!(event->modifiers() & Qt::ControlModifier))
                    d->deselectSelected();
            }

            // Filtering selection
            if (d->m_pressed_item &&
                    d->m_selection_filters.count() &&
                    !d->m_selection_filters.contains( d->m_pressed_item->metaObject()->className() ))
                d->m_pressed_item = 0;

            // If there is VALID item to select...
            if (d->m_pressed_item)
            {
                // If not selectable -> deselect item
                if (!(d->m_pressed_item->flags() & QGraphicsItem::ItemIsSelectable))
                    d->m_pressed_item = 0;
                else
                    d->sendPressEventToItem(d->m_pressed_item, event);
            }
            // If listeners should know scene press position
            else if (m_interaction_mode & MouseTracking)
                emit mousePressedPoint(event->buttonDownScenePos(event->button()));

            setRotationWidgetVisible(m_interaction_mode & Rotating);
            setScalingWidgetVisible(m_interaction_mode & Scaling);
            setCropWidgetVisible(m_interaction_mode & Cropping);
        }
        event->setAccepted(m_interaction_mode & Selecting);
    }
    else if (event->button() == Qt::RightButton)
        this->contextMenuEvent(event);
}


void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (d->m_readSceneMousePress_enabled)
            return;

        if (d->m_pressed_object)
            d->sendMoveEventToItem(d->m_pressed_object, event);

        if (m_interaction_mode & Moving && !event->isAccepted())
        {
            // Selecting pressed item
            event->setAccepted(d->selectPressed());

            // Moving items
            if (d->m_selected_items_all_movable)
            {
                // Calculate movement
                QPointF distance = event->scenePos() - event->buttonDownScenePos(Qt::LeftButton) + d->m_selected_items_path_initial_pos;
                if (event->modifiers() & Qt::ShiftModifier && this->isGridVisible())
                {
                    distance.setX(x_grid*round(distance.rx()/x_grid));
                    distance.setY(y_grid*round(distance.ry()/y_grid));
                }
                QPointF difference = d->m_selected_items_path.boundingRect().topLeft();
                          d->m_selected_items_path.translate(distance);
                d->m_selected_items_path.translate(-difference);
                difference = distance - difference;
                d->m_selected_items_path.translate(difference);
                foreach(AbstractItemInterface* item, d->m_selected_items.keys())
                    item->moveBy(difference.x(), difference.y());
            }
        }
    }
}


void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (d->m_readSceneMousePress_enabled)
        {
            d->m_readSceneMousePress_listener->mouseRelease(event->scenePos());
            event->setAccepted(true);
            return;
        }

        if (m_interaction_mode & Selecting)
        {
            // Selecting pressed item
            event->setAccepted(d->selectPressed());

            if (m_interaction_mode & OneclickFocusItems)
                d->focusPressed();

            // Send mousereleaseevent to the released item
            if (d->m_pressed_object)
                d->sendReleaseEventToItem(d->m_pressed_object, event);

            // Post move command to QUndoStack
            if ((m_interaction_mode & Moving) && d->wasMoved())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "move command from scene";
                QUndoCommand * command = new MoveItemsCommand(d->m_selected_items, this);
                PLE_PostUndoCommand(command);
            }
        }
    }
}


void Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        // In this app there is no difference between doubleClick and press events.
        // in Qt mouse events are alled in this order:        pressEvent -> releaseEvent -> doubleClickEvent -> releaseEvent
        // So for correct working second call of releaseEvent it is needed to call mousePressEvent here.
        this->mousePressEvent(event);

        // If selecting enabled -> focus item
        if (m_interaction_mode & Selecting)
            d->focusPressed();
    }
}


void Scene::drawBackground(QPainter * painter, const QRectF & rect)
{
    // Transparent scene background
    if (isSelectionVisible())
    {
        QTransform tr = painter->transform().inverted();
        QPixmap pixmap(20,20);
        QPainter p(&pixmap);
        p.fillRect(0,0,20,20,Qt::lightGray);
        p.fillRect(0,10,10,10,Qt::darkGray);
        p.fillRect(10,0,10,10,Qt::darkGray);
        QBrush b(pixmap);
        b.setTransform(tr);
        painter->fillRect(rect, b);
    }

    // Fill scene outside sceneRect with semi-transparent window color
    {
        QPainterPath p;
        p.addRect(rect);
        QPainterPath s;
        s.addRect(this->sceneRect());
        painter->fillPath(p.subtracted(s), OUTSIDE_SCENE_COLOR);
    }
}


void Scene::drawForeground(QPainter * painter, const QRectF & rect)
{
    QGraphicsScene::drawForeground(painter, rect.intersected(this->sceneRect()));

    // Draw selected items shape
    if (isSelectionVisible())
    {
        this->calcSelectionBoundingRect();
        painter->save();
        painter->setPen(Qt::red);
        painter->setCompositionMode(QPainter::RasterOp_NotSourceAndNotDestination);
        painter->drawPath(d->m_selected_items_path);
        painter->restore();
    }
}


void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    if (canDecode(event->mimeData()))
    {
        event->setDropAction(Qt::CopyAction);
        event->setAccepted(true);
    }
    else
    {
        event->setDropAction(Qt::IgnoreAction);
        event->setAccepted(false);
    }
}


void Scene::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
{
    if (d->m_hovered_photo)
    {
        d->m_hovered_photo->dragLeaveEvent(event);
        d->m_hovered_photo = 0;
    }
}


void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    PhotoItem * tempItem = dynamic_cast<PhotoItem*>(this->itemAt(event->scenePos(), QTransform()));
    // Send event to item
    if (tempItem)
    {
        if (d->m_hovered_photo != tempItem)
        {
            if (tempItem)
                tempItem->dragEnterEvent(event);
            if (d->m_hovered_photo)
                d->m_hovered_photo->dragLeaveEvent(event);
        }
        else
            tempItem->dragMoveEvent(event);
    }
    // Proces event on scene
    else
    {
        if (d->m_hovered_photo)
            d->m_hovered_photo->dragLeaveEvent(event);
        if (canDecode(event->mimeData()))
        {
            event->setDropAction(Qt::CopyAction);
            event->setAccepted(true);
        }
        // Ignore event
        else
        {
            event->setDropAction(Qt::IgnoreAction);
            event->setAccepted(false);
        }
    }

    d->m_hovered_photo = tempItem;
}


void Scene::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    PhotoItem * item = dynamic_cast<PhotoItem*>(this->itemAt(event->scenePos(), QTransform()));
    if (item)
    {
        item->dropEvent(event);
        return;
    }

    d->paste_scene_pos = event->scenePos();

    const QMimeData * mimeData = event->mimeData();
    if ( PhotoLayoutsWindow::instance()->hasInterface() &&
            mimeData->hasFormat(QLatin1String("digikam/item-ids")))
    {
        QList<QUrl> urls;
        QByteArray ba = mimeData->data(QLatin1String("digikam/item-ids"));
        QDataStream ds(&ba, QIODevice::ReadOnly);
        ds >> urls;

        ImageLoadingThread * ilt = new ImageLoadingThread(this);
        ilt->setImagesUrls(urls);
        ilt->setMaximumProgress(0.9);
        connect(ilt, SIGNAL(imageLoaded(QUrl,QImage)), this, SLOT(imageLoaded(QUrl,QImage)));
        ilt->start();
    }
    else if (mimeData->hasFormat(QLatin1String("text/uri-list")))
    {
        QList<QUrl> urls = mimeData->urls();
        QList<QUrl> list;
        foreach(QUrl url, urls)
            list << QUrl(url);

        ImageLoadingThread * ilt = new ImageLoadingThread(this);
        ilt->setImagesUrls(list);
        ilt->setMaximumProgress(0.9);
        connect(ilt, SIGNAL(imageLoaded(QUrl,QImage)), this, SLOT(imageLoaded(QUrl,QImage)));
        ilt->start();
    }

    event->setAccepted( true );
}


void Scene::setGrid(double x, double y)
{
    // Grid can't be 0
    if (x == 0 || y == 0)
        return;

    this->x_grid = x;
    this->y_grid = y;

    if (!grid_visible)
        return;

    if (!grid_item)
    {
        grid_item = new QGraphicsItemGroup();
        grid_item->setZValue(0);
        grid_item->setVisible(true);
        QGraphicsOpacityEffect * effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.5);
        grid_item->setGraphicsEffect(effect);
    }

    qreal width = sceneRect().width();
    qreal height = sceneRect().height();
    QList<QGraphicsItem*> children = grid_item->childItems();
    QList<QGraphicsItem*>::iterator it = children.begin();
    QGraphicsLineItem * temp;

    for (qreal i = x; i < width; i+=x)
    {
        if (it != children.end())
        {
            temp = static_cast<QGraphicsLineItem*>(*it);
            temp->setLine(i,0,i,height);
            ++it;
        }
        else
        {
            temp = new QGraphicsLineItem(i, 0, i, height, 0);
            grid_item->addToGroup(temp);
        }
    }

    for (qreal i = y; i < height; i+=y)
    {
        if (it != children.end())
        {
            temp = static_cast<QGraphicsLineItem*>(*it);
            temp->setLine(0,i,width,i);
            ++it;
        }
        else
        {
            temp = new QGraphicsLineItem(0, i, width, i, 0);
            grid_item->addToGroup(temp);
        }
    }

    QList<QGraphicsItem*> toRemove;
    while (it != children.end())
        toRemove.append(*(it++));
    while (toRemove.count())
    {
        QGraphicsItem * temp = toRemove.takeAt(0);
        grid_item->removeFromGroup(temp);
        delete temp;
    }
}


void Scene::setHorizontalGrid(double x)
{
    setGrid(x, this->y_grid);
}


void Scene::setVerticalGrid(double y)
{
    this->setGrid(this->x_grid, y);
}


void Scene::setGridVisible(bool visible)
{
    if (grid_visible == visible)
        return;

    grid_visible = visible;
    if (visible)
        this->setGrid(x_grid, y_grid);
    else
    {
        delete grid_item;
        grid_item = 0;
    }
}


bool Scene::isGridVisible()
{
    return this->grid_visible;
}


void Scene::setInteractionMode(int mode)
{
    m_interaction_mode = mode;
    setRotationWidgetVisible(mode & Rotating);
    setScalingWidgetVisible(mode & Scaling);
    setCropWidgetVisible(mode & Cropping);
}


void Scene::setSelectionMode(SelectionMode selectionMode)
{
    switch(selectionMode)
    {
        case NoSelection:
            this->setSelectionArea(QPainterPath());
            this->selectionMode = selectionMode;
            break;
        case MultiSelection:
            this->selectionMode = selectionMode;
            break;
        case SingleSelection:
            this->setSelectionArea(QPainterPath());
            this->selectionMode = selectionMode;
            break;
    }
}


void Scene::addSelectingFilter(const QMetaObject & classMeta)
{
    d->m_selection_filters.push_back(classMeta.className());
}


void Scene::clearSelectingFilters()
{
    d->m_selection_filters.clear();
}


void Scene::setRotationWidgetVisible(bool isVisible)
{
    if (d->m_rot_item)
    {
        if (d->m_pressed_object == d->m_rot_item)
            d->m_pressed_object = 0;
        this->QGraphicsScene::removeItem(d->m_rot_item);
        d->m_rot_item->deleteLater();
        d->m_rot_item = 0;
    }

    if (isVisible && d->m_selected_items.count())
    {
        if (!d->m_rot_item)
            d->m_rot_item = new RotationWidgetItem(d->m_selected_items.keys());
        d->m_rot_item->setZValue(std::numeric_limits<double>::infinity());
        this->QGraphicsScene::addItem(d->m_rot_item);
    }
}


void Scene::setScalingWidgetVisible(bool isVisible)
{
    if (d->m_scale_item)
    {
        if (d->m_pressed_object == d->m_scale_item)
            d->m_pressed_object = 0;
        this->QGraphicsScene::removeItem(d->m_scale_item);
        d->m_scale_item->deleteLater();
        d->m_scale_item = 0;
    }

    if (isVisible && d->m_selected_items.count())
    {
        if (!d->m_scale_item)
            d->m_scale_item = new ScalingWidgetItem(d->m_selected_items.keys());
        d->m_scale_item->setZValue(std::numeric_limits<double>::infinity());
        this->QGraphicsScene::addItem(d->m_scale_item);
        this->update(d->m_scale_item->boundingRect());
    }
}


void Scene::setCropWidgetVisible(bool isVisible)
{
    if (d->m_crop_item)
    {
        if (d->m_pressed_object == d->m_crop_item)
            d->m_pressed_object = 0;
        this->QGraphicsScene::removeItem(d->m_crop_item);
        d->m_crop_item->deleteLater();
        d->m_crop_item = 0;
    }

    if (isVisible && d->m_selected_items.count())
    {
        if (!d->m_crop_item)
        {
            d->m_crop_item = new CropWidgetItem();
            connect(d->m_crop_item, SIGNAL(cancelCrop()), this, SLOT(closeCropWidget()));
        }
        d->m_crop_item->setZValue(std::numeric_limits<double>::infinity());
        this->QGraphicsScene::addItem(d->m_crop_item);
        if (d->m_selected_items.count() == 1)
        {
            d->m_crop_item->setItems(d->m_selected_items.keys());
        }
        else
            d->m_crop_item->hide();
    }
    else if (m_interaction_mode & Cropping)
        this->clearSelection();
}


void Scene::closeCropWidget()
{
    this->setCropWidgetVisible(false);
}


qreal Scene::gridHorizontalDistance() const
{
    return this->x_grid;
}


qreal Scene::gridVerticalDistance() const
{
    return this->y_grid;
}


QDomDocument Scene::toSvg(ProgressObserver * observer)
{
    return toSvg(observer, false);
}


QDomDocument Scene::toTemplateSvg(ProgressObserver * observer)
{
    return toSvg(observer, true);
}


QDomDocument Scene::toSvg(ProgressObserver * observer, bool asTemplate)
{
    QDomDocument document;

    QDomElement sceneElement = document.createElement(QLatin1String("g"));
    sceneElement.setAttribute(QLatin1String("id"), QLatin1String("Scene"));
    sceneElement.setAttribute(QLatin1String("width"), QString::number(this->width()));
    sceneElement.setAttribute(QLatin1String("height"), QString::number(this->height()));
    document.appendChild(sceneElement);

    if (asTemplate)
    {
        QDomElement previewImage = document.createElement(QLatin1String("defs"));
        previewImage.setAttribute(QLatin1String("id"), QLatin1String("Preview"));
        QDomElement image = document.createElement(QLatin1String("image"));

        QSizeF sceneSize = this->sceneRect().size();
        double imgw = 200, imgh = 200;
        if ((imgw / sceneSize.width()) < (imgh / sceneSize.height()))
            imgh = qRound(sceneSize.height() * imgw / sceneSize.width());
        else
            imgw = qRound(sceneSize.width() * imgh / sceneSize.height());

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        QImage img(QSize(imgw, imgh), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        this->render(&p, QRectF(0, 0, imgw, imgh), this->sceneRect(), Qt::KeepAspectRatio);
        p.end();
        QTemporaryFile temp;

        if (temp.open())
        {
            img.save(temp.fileName());
            img.save(&buffer, "PNG");
        }

        image.appendChild( document.createTextNode( QString::fromUtf8(byteArray.toBase64()) ) );
        image.setAttribute(QLatin1String("width"),QString::number((int)imgw));
        image.setAttribute(QLatin1String("height"),QString::number((int)imgh));

        previewImage.appendChild(image);
        sceneElement.appendChild(previewImage);
    }

    QList<QGraphicsItem*> itemsList = this->items(Qt::AscendingOrder);

    if (observer)
        observer->progresChanged(0);

    //--------------------------------------------------------

    if (observer)
        observer->progresName( i18n("Saving background...") );

    QDomElement background = document.createElement(QLatin1String("g"));
    background.setAttribute(QLatin1String("class"), QLatin1String("background"));
    background.appendChild(d->m_background->toSvg(document));
    sceneElement.appendChild(background);

    if (observer)
        observer->progresChanged(1.0 / (double)(itemsList.count()+1.0));

    //--------------------------------------------------------

    int i = 1;
    foreach(QGraphicsItem* item, itemsList)
    {
        AbstractPhoto * photo = dynamic_cast<AbstractPhoto*>(item);

        if (photo)
        {
            if (observer)
                observer->progresName( i18n("Saving %1...", photo->name()) );

            QDomDocument photoItemDocument = asTemplate ? photo->toTemplateSvg() : photo->toSvg();
            sceneElement.appendChild( photoItemDocument.documentElement() );
        }

        if (observer)
            observer->progresChanged((double)i++ / (double)(itemsList.count()+1.0));
    }

    //--------------------------------------------------------

    if (observer)
        observer->progresName( i18n("Saving border...") );

    QDomElement border = document.createElement(QLatin1String("g"));
    border.setAttribute(QLatin1String("class"), QLatin1String("border"));
    border.appendChild(d->m_border->toSvg(document));
    sceneElement.appendChild(border);

    if (observer)
        observer->progresChanged(1.0 / (double)(itemsList.count()+1.0));

    return document;
}


Scene * Scene::fromSvg(QDomElement & sceneElement)
{
    if (sceneElement.isNull() || sceneElement.tagName() != QLatin1String("g") || sceneElement.attribute(QLatin1String("id")) != QLatin1String("Scene"))
        return 0;

    // Scene dimension
    qreal xSceneRect = 0;
    qreal ySceneRect = 0;
    qreal widthSceneRect = sceneElement.attribute(QLatin1String("width")).toDouble();
    qreal heightSceneRect = sceneElement.attribute(QLatin1String("height")).toDouble();
    QRectF dimension(xSceneRect,ySceneRect,widthSceneRect,heightSceneRect);
    Scene * result = new Scene(dimension);

    // Loading thread
    CanvasLoadingThread * thread = new CanvasLoadingThread(result);

    // Create elements
    int errorsCount = 0;
    QDomNodeList children = sceneElement.childNodes();
    for (int i = 0; i < children.count(); ++i)
    {
        QDomElement element = children.at(i).toElement();
        if (element.isNull() || element.tagName() != QLatin1String("g"))
            continue;
        QString itemClass = element.attribute(QLatin1String("class"));
        AbstractPhoto * item;
        if (itemClass == QLatin1String("PhotoItem"))
        {
            item = new PhotoItem();
            thread->addItem(item, element);
        }
        else if (itemClass == QLatin1String("TextItem"))
        {
            item = new TextItem();
            thread->addItem(item, element);
        }
        else if (itemClass == QLatin1String("background"))
        {
            thread->addBackground(result->d->m_background, element);
            continue;
        }
        else if (itemClass == QLatin1String("border"))
        {
            thread->addBorder(result->d->m_border, element);
            continue;
        }
        else
            continue;

        // If created add item to scene
        if (item)
        {
            result->QGraphicsScene::addItem(item);
            result->model()->insertItem(item, 0, result->model()->findIndex( dynamic_cast<AbstractPhoto*>(item->parentItem()) ));
            item->setZValue(i+1);
        }
        else
            ++errorsCount;
    }

    thread->start();

    // Show error message
    if (errorsCount)
    {
        QMessageBox::critical(0, i18n("Error"), i18np("Unable to create one element", "Unable to create %1 elements", errorsCount));
    }

    return result;
}


void Scene::render(QPainter * painter, const QRectF & target, const QRectF & source, Qt::AspectRatioMode aspectRatioMode)
{
    if (d->m_rot_item)
        d->m_rot_item->hide();
    if (d->m_scale_item)
        d->m_scale_item->hide();
    d->m_selection_visible = false;

    QGraphicsScene::render(painter, target, source, aspectRatioMode);

    d->m_selection_visible = true;
    if (d->m_rot_item)
        d->m_rot_item->show();
    if (d->m_scale_item)
        d->m_scale_item->show();
}


void Scene::readSceneMousePress(MousePressListener * mouseListsner)
{
    d->m_readSceneMousePress_listener = mouseListsner;
    if (mouseListsner)
        d->m_readSceneMousePress_enabled = true;
    else
        d->m_readSceneMousePress_enabled = false;
}


QList<AbstractPhoto*> Scene::selectedItems() const
{
    QList<AbstractPhoto*> result;
    const QList<QGraphicsItem*> & list = QGraphicsScene::selectedItems();
    foreach(QGraphicsItem* item, list)
        result << static_cast<AbstractPhoto*>(item);
    return result;
}


void Scene::updateSelection()
{
    foreach(AbstractPhoto* item, d->m_selected_items.keys())
        if (!item->isSelected())
            d->m_selected_items.remove(item);

    d->m_selected_items_path = QPainterPath();
    QList<AbstractPhoto*> itemsList = this->selectedItems();
    foreach(AbstractPhoto* item, itemsList)
    {
        if (d->m_selection_filters.count() && !d->m_selection_filters.contains( item->metaObject()->className() ))
        {
            item->setSelected(false);
            d->m_selected_items.remove(item);
            continue;
        }
        if (!d->m_selected_items.contains(item))
            d->m_selected_items.insert(item, item->pos());
        d->m_selected_items_path = d->m_selected_items_path.united(item->mapToScene(item->shape()));
    }

    if (d->m_selected_items.count() == 1 && d->m_selected_items.begin().key()->flags() & QGraphicsItem::ItemIsFocusable)
        d->m_selected_items.begin().key()->setFocus(Qt::OtherFocusReason);

    this->setRotationWidgetVisible(m_interaction_mode & Rotating);
    this->setScalingWidgetVisible(m_interaction_mode & Scaling);
    this->setCropWidgetVisible(m_interaction_mode & Cropping);
}


void Scene::imageLoaded(const QUrl & url, const QImage & image)
{
    if (!image.isNull())
    {
        PhotoItem * photo = new PhotoItem(image, url.fileName(), this);
        photo->setPos(d->paste_scene_pos);

        d->paste_scene_pos += QPointF (20, 20);
        if (d->paste_scene_pos.x() >= this->sceneRect().bottomRight().x() ||
            d->paste_scene_pos.y() >= this->sceneRect().bottomRight().y() )
        {
            d->paste_scene_pos = this->sceneRect().topLeft();
        }

        this->addItem(photo);
    }
}


void Scene::calcSelectionBoundingRect()
{
    d->m_selected_items_path = QPainterPath();
    foreach(AbstractItemInterface* item, d->m_selected_items.keys())
        d->m_selected_items_path = d->m_selected_items_path.united(item->mapToScene(item->shape()));
}


bool Scene::askAboutRemoving(int count)
{
    if (count)
    {
        int result = QMessageBox::question(qApp->activeWindow(), 
                                           i18n("Items deleting"),
                                           i18np("Are you sure you want to delete selected item?", "Are you sure you want to delete %1 selected items?", count));
        if (result == QMessageBox::Yes)
            return true;
    }
    return false;
}


bool Scene::canDecode(const QMimeData * mimeData)
{
    if (PhotoLayoutsWindow::instance()->hasInterface() &&
            mimeData->hasFormat(QLatin1String("digikam/item-ids")))
        return true;

    QList<QUrl> urls = mimeData->urls();
    foreach(QUrl url, urls)
    {
        QImageReader ir(url.toLocalFile());
        if (!ir.canRead())
            return false;
    }
    return true;
}


bool Scene::isSelectionVisible()
{
    return d->m_selection_visible;
}


void Scene::setSelectionVisible(bool isVisible)
{
    d->m_selection_visible = isVisible;
}

} // namespace PhotoLayoutsEditor
