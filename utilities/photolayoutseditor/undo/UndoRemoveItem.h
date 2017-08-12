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

#ifndef UNDOREMOVEITEM_H
#define UNDOREMOVEITEM_H

#include <QUndoCommand>
#include <QModelIndex>
#include <QGraphicsItem>

namespace PhotoLayoutsEditor
{
    class Scene;
    class LayersModel;
    class AbstractPhoto;

    class UndoRemoveItem : public QUndoCommand
    {
            AbstractPhoto * m_item;
            AbstractPhoto * m_parentItem;
            QGraphicsScene * m_scene;
            LayersModel * m_model;
            QModelIndex m_parentIndex;
            QModelIndex m_itemIndex;
            int m_row;

        public:

            UndoRemoveItem(AbstractPhoto * item, Scene * scene, LayersModel * model, QUndoCommand * parent = 0);
            ~UndoRemoveItem();
            virtual void redo();
            virtual void undo();

        private:

            void appendChild(AbstractPhoto * item, const QModelIndex & parent);
            static bool compareGraphicsItems(QGraphicsItem * i1, QGraphicsItem * i2);
    };
}

#endif // UNDOREMOVEITEM_H
