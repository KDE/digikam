/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-7-30
 * Description : Widget for displaying HTML in the backends
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#include "webwidget_qwebengine.h"

// Qt includes

#include <QCloseEvent>
#include <QtWebEngineWidgetsVersion>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN WebWidget::Private
{
public:

    explicit Private()
      : parent(0)
    {
    }

    QWidget* parent;
};

WebWidget::WebWidget(QWidget* const parent)
    : QWebEngineView(parent),
      d(new Private())
{
    d->parent = parent;
#if QTWEBENGINEWIDGETS_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
#endif
}

void WebWidget::closeEvent(QCloseEvent* event)
{
    emit closeView(false);
    event->accept();
}

WebWidget::~WebWidget()
{
    delete d;
}
}
