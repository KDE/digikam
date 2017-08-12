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

#include "CanvasSavingThread.h"

#include "Canvas.h"
#include "Canvas_p.h"
#include "Scene.h"
#include "ProgressEvent.h"
#include "photolayoutswindow.h"

#include <QFile>
#include <QDomDocument>
#include <QCoreApplication>

#include "digikam_debug.h"

using namespace PhotoLayoutsEditor;

CanvasSavingThread::CanvasSavingThread(QObject* parent) :
    QThread(parent),
    m_canvas(0),
    m_template(false)
{
}

void CanvasSavingThread::save(Canvas* canvas, const QUrl & url)
{
    m_canvas = canvas;
    m_url    = url;
    this->start();
}

void CanvasSavingThread::saveAsTemplate(Canvas * canvas, const QUrl& url)
{
    m_canvas = canvas;
    m_url    = url;
    m_template = true;
    this->start();
}

void CanvasSavingThread::progresChanged(double progress)
{
    this->sendProgressUpdate(0.05 + progress * 0.75);
}

void CanvasSavingThread::progresName(const QString & name)
{
    this->sendActionUpdate(name);
}

void CanvasSavingThread::run()
{
    if (!m_canvas || !m_url.isValid())
        return;

    //---------------------------------------------------------------------------

    ProgressEvent * startEvent = new ProgressEvent(this);
    startEvent->setData(ProgressEvent::Init, 0);
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), startEvent);
    QCoreApplication::processEvents();

    this->sendActionUpdate( i18n("Creating canvas...") );

    QRect sceneRect = m_canvas->sceneRect().toRect();
    QDomDocument document(QLatin1String(" svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\""));
    QDomElement svg = document.createElement(QLatin1String("svg"));
    document.appendChild(svg);

    svg.setAttribute(QLatin1String("xmlns"), QLatin1String("http://www.w3.org/2000/svg"));
    svg.setAttribute(QLatin1String("viewBox"), QString::number(sceneRect.x()) + QLatin1Char(' ') +
                                QString::number(sceneRect.y()) + QLatin1Char(' ') +
                                QString::number(sceneRect.width()) + QLatin1Char(' ') +
                                QString::number(sceneRect.height()));
    svg.setAttribute(QLatin1String("xmlns:xlink"), QLatin1String("http://www.w3.org/1999/xlink"));
    svg.setAttribute(QLatin1String("version"), QLatin1String("1.2"));
    svg.setAttribute(QLatin1String("baseProfile"), QLatin1String("tiny"));
    QString j;
    switch (m_canvas->d->m_size.sizeUnit())
    {
        case CanvasSize::Centimeters:
            j = QLatin1String("cm");
            break;
        case CanvasSize::Milimeters:
            j = QLatin1String("mm");
            break;
        case CanvasSize::Inches:
            j = QLatin1String("in");
            break;
        case CanvasSize::Picas:
            j = QLatin1String("pc");
            break;
        case CanvasSize::Points:
            j = QLatin1String("pt");
            break;
        case CanvasSize::Pixels:
            j = QLatin1String("px");
            break;
        default:
            j = QLatin1String("px");
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unhandled size unit at:" << __FILE__ << ":" << __LINE__;
            break;
    }
    svg.setAttribute(QLatin1String("width"), QString::number(m_canvas->d->m_size.size().width()) + j);
    svg.setAttribute(QLatin1String("height"), QString::number(m_canvas->d->m_size.size().height()) + j);
    QDomElement resolution = document.createElementNS(m_template ? PhotoLayoutsEditor::templateUri() : PhotoLayoutsEditor::uri(), QLatin1String("page"));
    resolution.setAttribute(QLatin1String("width"), QString::number(m_canvas->d->m_size.resolution().width()));
    resolution.setAttribute(QLatin1String("height"), QString::number(m_canvas->d->m_size.resolution().height()));
    resolution.setAttribute(QLatin1String("unit"), CanvasSize::resolutionUnitName(m_canvas->d->m_size.resolutionUnit()));
    svg.appendChild(resolution);

    this->sendProgressUpdate( 0.05 );
    this->sendActionUpdate( i18n("Saving scene...") );

    //---------------------------------------------------------------------------

    Scene * scene = dynamic_cast<Scene*>(m_canvas->scene());
    if (!scene)
    {
        this->exit(1);
        return;
    }
    QDomDocument sceneDocument = m_template ? scene->toTemplateSvg(this) : scene->toSvg(this);
    QDomElement sceneElement = sceneDocument.documentElement();
    if (sceneElement.isNull())
        this->exit(1);
    svg.appendChild(sceneElement);

    //---------------------------------------------------------------------------

    this->sendProgressUpdate( 0.8 );
    this->sendActionUpdate( i18n("Encoding data...") );

    QFile file(m_url.path());
    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        QByteArray result = document.toByteArray();
        const char * data = result.data();
        int i = 0;
        const int limit = result.size();
        int j = 1000;
        j = (j > limit ? limit : j);
        this->sendActionUpdate( i18n("Writing data to file...") );
        while (i < limit)
        {
            i += file.write(data+i, (i+j <= limit ? j : limit-i));
            this->sendProgressUpdate( 0.8 + 0.2 * ((double)i / (double)limit) );
        }
        file.close();
        emit saved();
    }

    //---------------------------------------------------------------------------

    ProgressEvent* finishEvent = new ProgressEvent(this);
    finishEvent->setData(ProgressEvent::Finish, 0);
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), finishEvent);
    QCoreApplication::processEvents();

    this->exit(0);
}

void CanvasSavingThread::bytesWritten(qint64 b)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "writen" << b;
}

void CanvasSavingThread::sendProgressUpdate(double v)
{
    ProgressEvent* event = new ProgressEvent(this);
    event->setData(ProgressEvent::ProgressUpdate, v);
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), event);
    QCoreApplication::processEvents();
}

void CanvasSavingThread::sendActionUpdate(const QString& str)
{
    ProgressEvent* event = new ProgressEvent(this);
    event->setData(ProgressEvent::ActionUpdate, str);
    QCoreApplication::postEvent(PhotoLayoutsWindow::instance(), event);
    QCoreApplication::processEvents();
}
