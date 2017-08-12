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

#ifndef CANVASLOADINGTHREAD_H
#define CANVASLOADINGTHREAD_H

#include <QThread>
#include <QDomDocument>

#include "ProgressObserver.h"

namespace PhotoLayoutsEditor
{
    class AbstractPhoto;
    class SceneBackground;
    class SceneBorder;
    class CanvasLoadingThread : public QThread, public ProgressObserver
    {
            Q_OBJECT

        public:

            explicit CanvasLoadingThread(QObject *parent = 0);
            ~CanvasLoadingThread();
            virtual void progresChanged(double progress);
            virtual void progresName(const QString & name);
            void addItem(AbstractPhoto * item, QDomElement & element);
            void addBackground(SceneBackground * background, QDomElement & element);
            void addBorder(SceneBorder * border, QDomElement & element);

        protected:

            virtual void run();

        private:

            class CanvasLoadingThreadPrivate;
            CanvasLoadingThreadPrivate * d;
            friend class CanvasLoadingThreadPrivate;
    };
}

#endif // CANVASLOADINGTHREAD_H
