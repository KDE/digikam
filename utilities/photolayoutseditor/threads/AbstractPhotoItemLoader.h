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

#ifndef ABSTRACTPHOTOITEMLOADER_H
#define ABSTRACTPHOTOITEMLOADER_H

#include <QThread>
#include <QDomDocument>

namespace PhotoLayoutsEditor
{
    class AbstractPhoto;
    class CanvasLoadingThread;
    class ProgressObserver;
    class AbstractPhotoItemLoader : public QThread
    {
            Q_OBJECT

            AbstractPhoto * m_item;
            QDomElement m_element;
            ProgressObserver * m_observer;

        public:

            explicit AbstractPhotoItemLoader(AbstractPhoto * item, QDomElement & element, QObject * parent = 0);
            AbstractPhoto * item() const;
            QDomElement element() const;
            void setObserver(ProgressObserver * observer);
            ProgressObserver * observer() const;

        protected:

            virtual void run();

        friend class CanvasLoadingThread;
    };
}

#endif // ABSTRACTPHOTOITEMLOADER_H
