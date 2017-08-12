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

#ifndef LAYERSTREE_H
#define LAYERSTREE_H

// Qt
#include <QMenu>
#include <QWidget>
#include <QAction>
#include <QTreeView>
#include <QAbstractItemModel>

// Local
#include "LayersModel.h"

class QGraphicsItem;

namespace PhotoLayoutsEditor
{
    class LayersTree;
    class LayersTreeMenu;
    class SwitchableIcon;

    class LayersTree : public QTreeView
    {
            Q_OBJECT

        public:

            explicit LayersTree(QWidget * parent = 0);
            virtual void setModel(QAbstractItemModel * model);

        public Q_SLOTS:

            void setSingleSelection();
            void setMultiSelection();

        Q_SIGNALS:

            void selectedRowsAboutToBeRemoved();
            void selectedRowsAboutToBeMovedUp();
            void selectedRowsAboutToBeMovedDown();

        protected:

            void contextMenuEvent(QContextMenuEvent * event);

        protected Q_SLOTS:

            void removeSelectedRows();
            void moveSelectedRowsUp();
            void moveSelectedRowsDown();

        private:

            LayersTreeMenu * m_menu;
    };
}

#endif // LAYERSTREE_H
