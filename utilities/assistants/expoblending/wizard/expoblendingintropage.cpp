/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "expoblendingintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QGroupBox>

// KDE includes

#include <klocalizedstring.h>

// local includes

#include "dbinarysearch.h"
#include "alignbinary.h"
#include "enfusebinary.h"
#include "dlayoutbox.h"

namespace Digikam
{

class ExpoBlendingIntroPage::Private
{
public:

    explicit Private(ExpoBlendingManager* const m)
      : mngr(m),
        binariesWidget(0)
    {
    }

    ExpoBlendingManager* mngr;
    DBinarySearch*       binariesWidget;
};

ExpoBlendingIntroPage::ExpoBlendingIntroPage(ExpoBlendingManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "Welcome to Stacked Images Tool")),
      d(new Private(mngr))
{
    DVBox* const vbox   = new DVBox(this);
    QLabel* const title = new QLabel(vbox);
    title->setWordWrap(true);
    title->setOpenExternalLinks(true);
    title->setText(i18n("<qt>"
                        "<p><h1><b>Welcome to Stacked Images Tool</b></h1></p>"
                        "<p>This tool fuses bracketed images with different exposure to make pseudo "
                        "<a href='http://en.wikipedia.org/wiki/High_dynamic_range_imaging'>HDR image</a>.</p>"
                        "<p>It can also be used to <a href='https://en.wikipedia.org/wiki/Focus_stacking'>merge focus bracketed stack</a> "
                        "to get a single image with increased depth of field.</p>"
                        "<p>This assistant will help you to configure how to import images before "
                        "merging them to a single one.</p>"
                        "<p>Bracketed images must be taken with the same camera, "
                        "in the same conditions, and if possible using a tripod.</p>"
                        "<p>For more information, please take a look at "
                        "<a href='http://en.wikipedia.org/wiki/Bracketing'>this page</a></p>"
                        "</qt>"));

    QGroupBox* const binaryBox      = new QGroupBox(vbox);
    QGridLayout* const binaryLayout = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "Exposure Blending Binaries"));
    d->binariesWidget = new DBinarySearch(binaryBox);
    d->binariesWidget->addBinary(d->mngr->alignBinary());
    d->binariesWidget->addBinary(d->mngr->enfuseBinary());

#ifdef Q_OS_OSX
    // Hugin bundle PKG install
    d->binariesWidget->addDirectory(QLatin1String("/Applications/Hugin/HuginTools"));
    d->binariesWidget->addDirectory(QLatin1String("/Applications/Hugin/Hugin.app/Contents/MacOS"));
    d->binariesWidget->addDirectory(QLatin1String("/Applications/Hugin/tools_mac"));

    // Std Macports install
    d->binariesWidget->addDirectory(QLatin1String("/opt/local/bin"));

    // digiKam Bundle PKG install
    d->binariesWidget->addDirectory(QLatin1String("/opt/digikam/bin"));
#endif

#ifdef Q_OS_WIN
    d->binariesWidget->addDirectory(QLatin1String("C:/Program Files/Hugin/bin"));
    d->binariesWidget->addDirectory(QLatin1String("C:/Program Files (x86)/Hugin/bin"));
    d->binariesWidget->addDirectory(QLatin1String("C:/Program Files/GnuWin32/bin"));
    d->binariesWidget->addDirectory(QLatin1String("C:/Program Files (x86)/GnuWin32/bin"));
#endif

    connect(d->binariesWidget, SIGNAL(signalBinariesFound(bool)),
            this, SIGNAL(signalExpoBlendingIntroPageIsValid(bool)));

    emit signalExpoBlendingIntroPageIsValid(d->binariesWidget->allBinariesFound());

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-stack.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));
}

ExpoBlendingIntroPage::~ExpoBlendingIntroPage()
{
    delete d;
}

bool ExpoBlendingIntroPage::binariesFound()
{
    return d->binariesWidget->allBinariesFound();
}

} // namespace Digikam
