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
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ABSTRACTMOVABLEMODEL_H
#define ABSTRACTMOVABLEMODEL_H

#include <QAbstractItemModel>
#include <QDebug>
#include <QModelIndex>
#include <QtGlobal>

namespace PhotoLayoutsEditor
{
    class MoveRowsCommand;

    class AbstractMovableModel : public QAbstractItemModel
    {

        public:

            AbstractMovableModel(QObject * parent = 0);
            virtual bool moveRowsData(int sourcePosition, int sourceCount, int destPosition) = 0;
            virtual void setItem(QObject * graphicsItem, const QModelIndex & index) = 0;
            virtual QObject * item(const QModelIndex & index) const = 0;
    };
}

#endif // ABSTRACTMOVABLEMODEL_H
