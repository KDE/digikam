/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "expoblendinglastpage.h"

// Qt includes

#include <QDir>
#include <QLabel>
#include <QPixmap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "enfusebinary.h"
#include "expoblendingmanager.h"
#include "dlayoutbox.h"

namespace Digikam
{

class ExpoBlendingLastPage::Private
{
public:

    explicit Private()
    {
        mngr = 0;
    }

    ExpoBlendingManager* mngr;
};

ExpoBlendingLastPage::ExpoBlendingLastPage(ExpoBlendingManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "Pre-Processing is Complete")),
      d(new Private)
{
    d->mngr                 = mngr;
    DVBox* const vbox      = new DVBox(this);
    QLabel* const title     = new QLabel(vbox);
    title->setOpenExternalLinks(true);
    title->setWordWrap(true);
    title->setText(i18n("<qt>"
                        "<p><h1><b>Bracketed Images Pre-Processing is Done</b></h1></p>"
                        "<p>Congratulations. Your images are ready to be fused. </p>"
                        "<p>To perform this operation, <b>%1</b> program from "
                        "<a href='%2'>Enblend</a> "
                        "project will be used.</p>"
                        "<p>Press \"Finish\" button to fuse your items and make a pseudo HDR image.</p>"
                        "</qt>",
                        QDir::toNativeSeparators(d->mngr->enfuseBinary().path()),
                        d->mngr->enfuseBinary().url().url()));

    vbox->setStretchFactor(new QWidget(vbox), 10);

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-enfuse.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));
}

ExpoBlendingLastPage::~ExpoBlendingLastPage()
{
    delete d;
}

}   // namespace Digikam
