/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-09-02
 * Description : Start Web Service methods.
 *
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "wsstarter.h"

// Qt includes

#include <QPointer>
#include <QMessageBox>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

class Q_DECL_HIDDEN WSStarter::Private
{
public:

    explicit Private()
    {
    }

};

class Q_DECL_HIDDEN WSStarterCreator
{
public:

    WSStarter object;
};

Q_GLOBAL_STATIC(WSStarterCreator, creator)

// ------------------------------------------------------------------------------------------------

WSStarter* WSStarter::instance()
{
    return &creator->object;
}

void WSStarter::cleanUp()
{
    if (creator.exists())
    {
    }
}

void WSStarter::exportToWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
    instance()->toWebService(tool, iface, parent);
}

void WSStarter::importFromWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
    instance()->fromWebService(tool, iface, parent);
}

// ------------------------------------------------------------------------------------------------

WSStarter::WSStarter()
    : d(new Private)
{
}

WSStarter::~WSStarter()
{
    delete d;
}

void WSStarter::toWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
}

void WSStarter::fromWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
}

bool WSStarter::checkWebService(QWidget* const widget) const
{
    if (widget && (widget->isMinimized() || !widget->isHidden()))
    {
        widget->showNormal();       // krazy:exclude=qmethods
        widget->activateWindow();
        widget->raise();
        return true;
    }

    return false;
}

} // namespace Digikam
