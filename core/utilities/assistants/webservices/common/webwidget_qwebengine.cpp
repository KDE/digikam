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
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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
namespace Digikam
{
class WebWidget::Private
{
public:

    explicit Private()
      : parent(0)
    {
    }

    QWidget*        parent;
};
WebWidget::WebWidget(QWidget* const parent)
    : QWebEngineView(parent),
      d(new Private())
{
    d->parent = parent;

}
WebWidget::~WebWidget()
{
    delete d;
}
}
