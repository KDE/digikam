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

#ifndef HTML_WIDGET_QWEBENGINE_H
#define HTML_WIDGET_QWEBENGINE_H

// Qt includes

#include <QTimer>
#include <QWebEngineView>
#include <QWebEnginePage>

// Local includes

#include "geoifacecommon.h"
#include "geoifacetypes.h"
#include "geocoordinates.h"

namespace Digikam
{

class HTMLWidget;

class HTMLWidgetPage : public QWebEnginePage
{
    Q_OBJECT

public:

    explicit HTMLWidgetPage(HTMLWidget* const parent = 0);
    virtual ~HTMLWidgetPage();

Q_SIGNALS:

    void signalHTMLEvents(const QStringList& events);

private Q_SLOTS:

    void slotSendHTMLEvents();

protected:

    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel, const QString&, int, const QString&);

private:

    QStringList m_events;
    QTimer*     m_timer;
};

// -------------------------------------------------------------------

class HTMLWidget : public QWebEngineView
{
    Q_OBJECT

public:

    explicit HTMLWidget(QWidget* const parent = 0);
    ~HTMLWidget();

    void loadInitialHTML(const QString& initialHTML);
    QVariant runScript(const QString& scriptCode, bool async = true);
    bool runScript2Coordinates(const QString& scriptCode, GeoCoordinates* const coordinates);
    void mouseModeChanged(const GeoMouseModes mouseMode);
    void setSelectionRectangle(const GeoCoordinates::Pair& searchCoordinates);
    void removeSelectionRectangle();
    void centerOn(const qreal west, const qreal north, const qreal east, const qreal south,
                  const bool useSaneZoomLevel = true);
    void setSharedGeoIfaceObject(GeoIfaceSharedData* const sharedData);

Q_SIGNALS:

    void signalHTMLEvents(const QStringList& events);
    void signalJavaScriptReady();
    void selectionHasBeenMade(const Digikam::GeoCoordinates::Pair& coordinatesRect);

protected:

    bool eventFilter(QObject*, QEvent*);

protected Q_SLOTS:

    void slotHTMLCompleted(bool ok);
    void progress(int progress);

private:

    class Private;
    Private* const d;

    GeoIfaceSharedData* s;
};

} // namespace Digikam

#endif // HTML_WIDGET_QWEBENGINE_H
