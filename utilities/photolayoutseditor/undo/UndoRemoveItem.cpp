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

#include "UndoRemoveItem.h"
#include "Scene.h"
#include "AbstractPhoto.h"
#include "LayersModel.h"
#include "LayersModelItem.h"

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

UndoRemoveItem::UndoRemoveItem(AbstractPhoto * item, Scene * scene, LayersModel * model, QUndoCommand * parent) :
    QUndoCommand(QString("Remove item"), parent),
    m_item(item),
    m_parentItem(static_cast<AbstractPhoto*>(item->QGraphicsItem::parentItem())),
    m_scene(scene->toGraphicsScene()),
    m_model(model)
{
}

UndoRemoveItem::~UndoRemoveItem()
{
    if (!m_item->scene())
        delete m_item;
}

void UndoRemoveItem::redo()
{
    // Remove from model
    m_parentIndex = m_model->findIndex(m_parentItem);
    m_itemIndex = m_model->findIndex(m_item, m_parentIndex);
    m_row = m_itemIndex.row();
    if (m_itemIndex.isValid())
        m_model->removeRow(m_row,m_parentIndex);
    // Remove from scene
    if (m_item->scene() == m_scene)
        m_scene->removeItem(m_item);
}

void UndoRemoveItem::undo()
{
    // Add to scene
    if (m_item->scene() != m_scene)
        m_scene->addItem(m_item);
    m_item->setParentItem(m_parentItem);
    // Add to model
    m_parentIndex = m_model->findIndex(m_parentItem);
    if (!m_model->hasIndex(m_row,0,m_parentIndex) || static_cast<LayersModelItem*>(m_model->index(m_row,0,m_parentIndex).internalPointer())->photo() != m_item)
    {
        if (m_model->insertRow(m_row,m_parentIndex))
        {
            static_cast<LayersModelItem*>(m_model->index(m_row,0,m_parentIndex).internalPointer())->setPhoto(m_item);
            // Add items children to model
            appendChild(m_item,m_model->index(m_row,0,m_parentIndex));
        }
    }
}

bool UndoRemoveItem::compareGraphicsItems(QGraphicsItem * i1, QGraphicsItem * i2)
{
    if ((i1 && i2) && (i1->zValue() < i2->zValue()))
        return true;
    return false;
}

void UndoRemoveItem::appendChild(AbstractPhoto * item, const QModelIndex & parent)
{
    QList<QGraphicsItem*> items = item->childItems();
    if (items.count())
    {
        // Sort using z-Values (z-Value == models row)
        qSort(items.begin(), items.end(), UndoRemoveItem::compareGraphicsItems);
        int i = 0;
        foreach(QGraphicsItem* childItem, items)
        {
            AbstractPhoto * photo = dynamic_cast<AbstractPhoto*>(childItem);
            if (photo)
            {
                if (m_model->insertRow(i,parent))
                {
                    static_cast<LayersModelItem*>(m_model->index(i, 0, parent).internalPointer())->setPhoto(photo);
                    this->appendChild(photo, m_model->index(i, 0, parent));
                    ++i;
                }
            }
        }
    }
}
