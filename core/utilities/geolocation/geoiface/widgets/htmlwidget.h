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

#ifndef HTML_WIDGET_H
#define HTML_WIDGET_H

// Qt includes

#include <QWebView>

// Local includes

#include "geoifacecommon.h"
#include "geoifacetypes.h"
#include "geocoordinates.h"

namespace Digikam
{

class HTMLWidget : public QWebView
{
    Q_OBJECT

public:

    explicit HTMLWidget(QWidget* const parent = 0);
    ~HTMLWidget();

    void loadInitialHTML(const QString& initialHTML);
    QVariant runScript(const QString& scriptCode);
    bool runScript2Coordinates(const QString& scriptCode, GeoCoordinates* const coordinates);
    void mouseModeChanged(const GeoMouseModes mouseMode);
    void setSelectionRectangle(const GeoCoordinates::Pair& searchCoordinates);
    void removeSelectionRectangle();
    void centerOn(const qreal west, const qreal north, const qreal east, const qreal south,
                  const bool useSaneZoomLevel = true);
    void setSharedGeoIfaceObject(GeoIfaceSharedData* const sharedData);

protected:

    bool eventFilter(QObject* object, QEvent* event);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

protected Q_SLOTS:

    void slotHTMLCompleted(bool ok);
    void slotScanForJSMessages(const QString& message);
    void progress(int progress);

Q_SIGNALS:

    void signalHTMLEvents(const QStringList& events);
    void signalJavaScriptReady();
    void selectionHasBeenMade(const Digikam::GeoCoordinates::Pair& coordinatesRect);

private:

    class Private;
    Private* const d;

    GeoIfaceSharedData* s;
};

} // namespace Digikam

#endif // HTML_WIDGET_H
