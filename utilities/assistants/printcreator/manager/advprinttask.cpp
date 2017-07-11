/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprinttask.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QImage>
#include <QSize>
#include <QPainter>
#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dfileoperations.h"
#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

class AdvPrintTask::Private
{
public:

    Private()
    {
        settings = 0;
    }

    ~Private()
    {
    }

public:

    AdvPrintSettings*           settings;
};

// -------------------------------------------------------

AdvPrintTask::AdvPrintTask(AdvPrintSettings* const settings)
    : ActionJob(),
      d(new Private)
{
    d->settings = settings;
}

AdvPrintTask::~AdvPrintTask()
{
    cancel();
    delete d;
}

void AdvPrintTask::run()
{

    emit signalDone(!m_cancel);
}

} // namespace Digikam
