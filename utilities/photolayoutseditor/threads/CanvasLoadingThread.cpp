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

#include "CanvasLoadingThread.h"
#include "ProgressEvent.h"
#include "AbstractPhotoItemLoader.h"
#include "PhotoItemLoader.h"
#include "TextItemLoader.h"
#include "SceneBackgroundLoader.h"
#include "SceneBorderLoader.h"
#include "AbstractPhoto.h"
#include "PhotoItem.h"
#include "TextItem.h"
#include "SceneBackground.h"
#include "SceneBorder.h"
#include "photolayoutswindow.h"

#include <QCoreApplication>

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

class CanvasLoadingThread::CanvasLoadingThreadPrivate
{
    CanvasLoadingThreadPrivate()
    {
        i     = 0;
        count = 0;
    }

    int                                  i;
    int                                  count;
    QMap<AbstractPhoto*, QDomElement>    data;
    QPair<SceneBackground*, QDomElement> background;
    QPair<SceneBorder*, QDomElement>     border;

    friend class CanvasLoadingThread;
};

CanvasLoadingThread::CanvasLoadingThread(QObject *parent) :
    QThread(parent),
    d(new CanvasLoadingThreadPrivate)
{
}

CanvasLoadingThread::~CanvasLoadingThread()
{
    delete d;
}

void CanvasLoadingThread::progresChanged(double progress)
{
    ProgressEvent * progressUpdateEvent = new ProgressEvent(this);
    progressUpdateEvent->setData(ProgressEvent::ProgressUpdate, ((double)d->i+1)/((double)d->data.count()+1) + (progress / (double)d->data.count()+1) );
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), progressUpdateEvent);
    QCoreApplication::processEvents();
}

void CanvasLoadingThread::progresName(const QString & name)
{
    ProgressEvent * actionUpdateEvent = new ProgressEvent(this);
    actionUpdateEvent->setData(ProgressEvent::ActionUpdate, name);
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), actionUpdateEvent);
    QCoreApplication::processEvents();
}

void CanvasLoadingThread::addItem(AbstractPhoto * item, QDomElement & element)
{
    if (!item || element.isNull())
        return;
    d->data.insert(item, element);
}

void CanvasLoadingThread::addBackground(SceneBackground * background, QDomElement & element)
{
    if (element.attribute(QLatin1String("class")) != QLatin1String("background") || !background)
        return;
    d->background.first = background;
    d->background.second = element;
}

void CanvasLoadingThread::addBorder(SceneBorder * border, QDomElement & element)
{
    if (element.attribute(QLatin1String("class")) != QLatin1String("border") || !border)
        return;
    d->border.first = border;
    d->border.second = element;
}

void CanvasLoadingThread::run()
{
    ProgressEvent * startEvent = new ProgressEvent(this);
    startEvent->setData(ProgressEvent::Init, 0);
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), startEvent);
    QCoreApplication::processEvents();

    // Background
    {
        ProgressEvent * actionUpdateEvent = new ProgressEvent(this);
        actionUpdateEvent->setData(ProgressEvent::ActionUpdate, i18n("Loading background...") );
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), actionUpdateEvent);
        QCoreApplication::processEvents();
        if (d->background.first)
        {
            SceneBackgroundLoader * loader = new SceneBackgroundLoader(d->background.first, d->background.second);
            loader->start();
            loader->wait();
        }
        ProgressEvent * progressUpdateEvent = new ProgressEvent(this);
        progressUpdateEvent->setData(ProgressEvent::ProgressUpdate, 1/((double)d->data.count()+2) );
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), progressUpdateEvent);
        QCoreApplication::processEvents();
    }

    // Items
    int count = d->data.count();
    d->i = 0;
    for (QMap<AbstractPhoto*,QDomElement>::iterator it = d->data.begin(); it != d->data.end(); ++it, ++(d->i))
    {
        ProgressEvent * actionUpdateEvent = new ProgressEvent(this);
        actionUpdateEvent->setData(ProgressEvent::ActionUpdate, i18n("Loading item no. %1...", QString::number(d->i)) );
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), actionUpdateEvent);
        QCoreApplication::processEvents();

        QDomElement e = it.value();
        if (e.attribute(QLatin1String("class")) == QLatin1String("PhotoItem"))
        {
            PhotoItemLoader * loader = new PhotoItemLoader(dynamic_cast<PhotoItem*>(it.key()), it.value());
            loader->setObserver(this);
            loader->start();
            loader->wait();
        }
        else if (e.attribute(QLatin1String("class")) == QLatin1String("TextItem"))
        {
            TextItemLoader * loader = new TextItemLoader(dynamic_cast<TextItem*>(it.key()), it.value());
            loader->setObserver(this);
            loader->start();
            loader->wait();
        }

        ProgressEvent * progressUpdateEvent = new ProgressEvent(this);
        progressUpdateEvent->setData(ProgressEvent::ProgressUpdate, ((double)d->i+1)/((double)count+2) );
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), progressUpdateEvent);
        QCoreApplication::processEvents();
    }

    // Border
    {
        ProgressEvent * actionUpdateEvent = new ProgressEvent(this);
        actionUpdateEvent->setData(ProgressEvent::ActionUpdate, i18n("Loading border...") );
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), actionUpdateEvent);
        QCoreApplication::processEvents();
        if (d->border.first)
        {
            SceneBorderLoader * borderLoader = new SceneBorderLoader(d->border.first, d->border.second);
            borderLoader->start();
            borderLoader->wait();
        }
        ProgressEvent * progressUpdateEvent = new ProgressEvent(this);
        progressUpdateEvent->setData(ProgressEvent::ProgressUpdate, 1/((double)d->data.count()+2) );
        QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), progressUpdateEvent);
        QCoreApplication::processEvents();
    }

    ProgressEvent * finishEvent = new ProgressEvent(this);
    finishEvent->setData(ProgressEvent::Finish, 0);
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), finishEvent);
    QCoreApplication::processEvents();
}
