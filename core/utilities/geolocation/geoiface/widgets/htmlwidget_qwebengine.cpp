/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Widget for displaying HTML in the backends
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2015      by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "htmlwidget_qwebengine.h"

// Qt includes

#include <QEventLoop>
#include <QResizeEvent>
#include <QCoreApplication>
#include <QWebEngineSettings>

// Local includes

#include "geoifacecommon.h"
#include "geoifacetypes.h"
#include "digikam_debug.h"

namespace Digikam
{

HTMLWidgetPage::HTMLWidgetPage(HTMLWidget* const parent)
    : QWebEnginePage(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    m_timer->setSingleShot(true);

    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(slotSendHTMLEvents()),
            Qt::QueuedConnection);
}

HTMLWidgetPage::~HTMLWidgetPage()
{
}

void HTMLWidgetPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel /*level*/, const QString& message, int /*lineNumber*/, const QString& /*sourceID*/)
{
    if (!message.startsWith(QLatin1String("(event)")))
        return;

    qCDebug(DIGIKAM_GEOIFACE_LOG) << message;

    const QString eventString = message.mid(7);

    if (eventString.isEmpty())
        return;

    m_events << eventString;

    m_timer->start();
}

void HTMLWidgetPage::slotSendHTMLEvents()
{
    emit signalHTMLEvents(m_events);
    m_events.clear();
}

// ---------------------------------------------------------------------------------------------

class HTMLWidget::Private
{
public:

    Private()
      : parent(0),
        child(0),
        hpage(0),
        isReady(false),
        selectionStatus(false),
        firstSelectionPoint(),
        intermediateSelectionPoint(),
        firstSelectionScreenPoint(),
        intermediateSelectionScreenPoint()
    {
    }

    QWidget*        parent;
    QWidget*        child;
    HTMLWidgetPage* hpage;

    bool            isReady;
    bool            selectionStatus;

    GeoCoordinates  firstSelectionPoint;
    GeoCoordinates  intermediateSelectionPoint;

    QPoint          firstSelectionScreenPoint;
    QPoint          intermediateSelectionScreenPoint;
};

HTMLWidget::HTMLWidget(QWidget* const parent)
    : QWebEngineView(parent),
      d(new Private()),
      s(0)
{
    d->parent = parent;
    setAcceptDrops(false);
    setFocusPolicy(Qt::WheelFocus);
    // settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);

    d->hpage = new HTMLWidgetPage(this);
    setPage(d->hpage);

    d->parent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(progress(int)));

    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(slotHTMLCompleted(bool)));

    connect(d->hpage, SIGNAL(signalHTMLEvents(QStringList)),
            this, SIGNAL(signalHTMLEvents(QStringList)));

    if (d->parent)
    {
        d->parent->installEventFilter(this);
    }

    d->child = findChild<QWidget*>();

    if (d->child)
    {
        d->child->installEventFilter(this);
    }
}

HTMLWidget::~HTMLWidget()
{
    delete d;
}

void HTMLWidget::progress(int progress)
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Maps Loading Progress: " << progress << "%";
}

void HTMLWidget::slotHTMLCompleted(bool ok)
{
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Map Loading Completed: " << ok;
    d->isReady = ok;

    emit signalJavaScriptReady();
}

/**
 * @brief Wrapper around executeScript to catch more errors
 */
QVariant HTMLWidget::runScript(const QString& scriptCode, bool async)
{
    GEOIFACE_ASSERT(d->isReady);

    if (!d->isReady)
        return QVariant();

    //qCDebug(DIGIKAM_GEOIFACE_LOG) << scriptCode;

    if(async)
    {
        page()->runJavaScript(scriptCode);
    }
    else
    {
        QVariant ret;
        QEventLoop loop;
        // lambda c11 function capturing value returned by java script code which is not synchro with QWebEngineView.
        // See https://wiki.qt.io/Porting_from_QtWebKit_to_QtWebEngine.
        page()->runJavaScript(scriptCode, [&ret, &loop](const QVariant& result){ ret.setValue(result); loop.quit(); });

        loop.exec();

        return ret;
    }
    
    return true;
}

/**
 * @brief Execute a script which returns coordinates and parse these
 */
bool HTMLWidget::runScript2Coordinates(const QString& scriptCode, GeoCoordinates* const coordinates)
{
    const QVariant scriptResult = runScript(scriptCode, false);

    return GeoIfaceHelperParseLatLonString(scriptResult.toString(), coordinates);
}

bool HTMLWidget::eventFilter(QObject* object, QEvent* event)
{
    if (d->parent && object == d->parent)
    {

        if (event->type() == QEvent::Resize)
        {
            QResizeEvent* const resizeEvent = dynamic_cast<QResizeEvent*>(event);

            if (resizeEvent)
            {
                resize(resizeEvent->size());
            }
        }
    }
    else if (d->child && object == d->child)
    {
        if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* const e = dynamic_cast<QMouseEvent*>(event);

            if (s->currentMouseMode == MouseModeRegionSelection)
            {
                if (!d->firstSelectionPoint.hasCoordinates())
                {
                    runScript2Coordinates(QString::fromLatin1("kgeomapPixelToLatLng(%1, %2);")
                                          .arg(e->x())
                                          .arg(e->y()),
                                          &d->firstSelectionPoint);

                    d->firstSelectionScreenPoint = QPoint(e->x(), e->y());
                }
                else
                {
                    runScript2Coordinates(QString::fromLatin1("kgeomapPixelToLatLng(%1, %2);")
                                          .arg(e->x())
                                          .arg(e->y()),
                                          &d->intermediateSelectionPoint);

                    d->intermediateSelectionScreenPoint = QPoint(e->x(), e->y());

                    qreal lonWest, latNorth, lonEast, latSouth;

                    if (d->firstSelectionScreenPoint.x() < d->intermediateSelectionScreenPoint.x())
                    {
                        lonWest  = d->firstSelectionPoint.lon();
                        lonEast  = d->intermediateSelectionPoint.lon();
                    }
                    else
                    {
                        lonEast  = d->firstSelectionPoint.lon();
                        lonWest  = d->intermediateSelectionPoint.lon();
                    }

                    if (d->firstSelectionScreenPoint.y() < d->intermediateSelectionScreenPoint.y())
                    {
                        latNorth = d->firstSelectionPoint.lat();
                        latSouth = d->intermediateSelectionPoint.lat();
                    }
                    else
                    {
                        latNorth = d->intermediateSelectionPoint.lat();
                        latSouth = d->firstSelectionPoint.lat();
                    }

                    runScript(QLatin1String("kgeomapRemoveTemporarySelectionRectangle();"));
                    runScript(QString::fromLatin1("kgeomapSetSelectionRectangle(%1, %2, %3, %4);")
                                .arg(lonWest)
                                .arg(latNorth)
                                .arg(lonEast)
                                .arg(latSouth));

                    const GeoCoordinates::Pair selectionCoordinates(
                            GeoCoordinates(latNorth, lonWest),
                            GeoCoordinates(latSouth, lonEast));

                    d->firstSelectionPoint.clear();
                    d->intermediateSelectionPoint.clear();

                    emit selectionHasBeenMade(selectionCoordinates);
                }
            }
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent* const e = dynamic_cast<QMouseEvent*>(event);

            if (s->currentMouseMode == MouseModeRegionSelection &&
                d->firstSelectionPoint.hasCoordinates())
            {
                runScript2Coordinates(QString::fromLatin1("kgeomapPixelToLatLng(%1, %2);")
                                      .arg(e->x())
                                      .arg(e->y()),
                                      &d->intermediateSelectionPoint);

                d->intermediateSelectionScreenPoint = QPoint(e->x(), e->y());

                qCDebug(DIGIKAM_GEOIFACE_LOG) << d->firstSelectionScreenPoint << QLatin1String(" ") << d->intermediateSelectionScreenPoint;

                qreal lonWest, latNorth, lonEast, latSouth;

                if (d->firstSelectionScreenPoint.x() < d->intermediateSelectionScreenPoint.x())
                {
                    lonWest  = d->firstSelectionPoint.lon();
                    lonEast  = d->intermediateSelectionPoint.lon();
                }
                else
                {
                    lonEast  = d->firstSelectionPoint.lon();
                    lonWest  = d->intermediateSelectionPoint.lon();
                }

                if (d->firstSelectionScreenPoint.y() < d->intermediateSelectionScreenPoint.y())
                {
                    latNorth = d->firstSelectionPoint.lat();
                    latSouth = d->intermediateSelectionPoint.lat();
                }
                else
                {
                    latNorth = d->intermediateSelectionPoint.lat();
                    latSouth = d->firstSelectionPoint.lat();
                }

                runScript(QString::fromLatin1("kgeomapSetTemporarySelectionRectangle(%1, %2, %3, %4);")
                            .arg(lonWest)
                            .arg(latNorth)
                            .arg(lonEast)
                            .arg(latSouth));
            }
        }
    }

    return false;
}

void HTMLWidget::setSelectionRectangle(const GeoCoordinates::Pair& searchCoordinates)
{
    if (!searchCoordinates.first.hasCoordinates())
    {
        runScript(QString::fromLatin1("kgeomapRemoveSelectionRectangle();"));
        return;
    }

    qreal West  = searchCoordinates.first.lon();
    qreal North = searchCoordinates.first.lat();
    qreal East  = searchCoordinates.second.lon();
    qreal South = searchCoordinates.second.lat();

    runScript(QString::fromLatin1("kgeomapSetSelectionRectangle(%1, %2, %3, %4);")
                .arg(West).arg(North).arg(East).arg(South));
}

void HTMLWidget::removeSelectionRectangle()
{
    runScript(QLatin1String("kgeomapRemoveSelectionRectangle();"));
}

void HTMLWidget::mouseModeChanged(const GeoMouseModes mouseMode)
{
    const bool inSelectionMode = (mouseMode == MouseModeRegionSelection);

    if (inSelectionMode)
    {
        d->firstSelectionPoint.clear();
        d->intermediateSelectionPoint.clear();
        runScript(QString::fromLatin1("kgeomapSelectionModeStatus(%1);").arg(inSelectionMode));
    }
    else
    {
        runScript(QString::fromLatin1("kgeomapSelectionModeStatus(%1);").arg(inSelectionMode));
    }
}

void HTMLWidget::centerOn(const qreal west, const qreal north,
                          const qreal east, const qreal south,
                          const bool useSaneZoomLevel)
{
/*
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "West:" << west
                                  << " North:" << north
                                  << " East:" << east
                                  << " South:" << south;
*/
    runScript(QString::fromLatin1("kgeomapSetMapBoundaries(%1, %2, %3, %4, %5);")
                .arg(west)
                .arg(north)
                .arg(east)
                .arg(south)
                .arg(useSaneZoomLevel ? 1 : 0));
}

void HTMLWidget::setSharedGeoIfaceObject(GeoIfaceSharedData* const sharedData)
{
    s = sharedData;
}

} // namespace Digikam
