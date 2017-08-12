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

#ifndef UNDOCOMMANDEVENTFILTER_H
#define UNDOCOMMANDEVENTFILTER_H

#include <QObject>

namespace PhotoLayoutsEditor
{
    class UndoCommandEventFilter : public QObject
    {
            Q_OBJECT

        public:

            explicit UndoCommandEventFilter(QObject* parent = 0) :
                QObject(parent)
            {
            }

            virtual bool eventFilter(QObject* watched, QEvent* event);

        Q_SIGNALS:

        public Q_SLOTS:

    };
}

#endif // UNDOCOMMANDEVENTFILTER_H
