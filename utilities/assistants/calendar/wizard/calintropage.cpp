/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to create calendar.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "calintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "calwizard.h"

namespace Digikam
{

CalIntroPage::CalIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title)
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                       "<p><h1><b>Welcome to Calendar Tool</b></h1></p>"
                       "<p>This assistant will guide you to create "
                       "and print a calendar with a selection of images taken "
                       "from your collection.</p>"
#ifdef HAVE_KCALENDAR
                       "<p>This tool will also permit to set specific dates "
                       "on your calendar using external data event files as "
                       "<a href='http://en.wikipedia.org/wiki/VCalendar'>vCalendar</a>, and "
                       "<a href='http://en.wikipedia.org/wiki/Icalendar'>iCalendar</a> formats.</p>"
#endif
                       "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("office-calendar")));
}

CalIntroPage::~CalIntroPage()
{
}

} // namespace Digikam
