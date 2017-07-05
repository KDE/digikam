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

#include "advprintintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QGroupBox>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "gimpbinary.h"
#include "dbinarysearch.h"

namespace Digikam
{

class AdvPrintIntroPage::Private
{
public:

    Private()
      : binSearch(0)
    {
    }

    GimpBinary     gimpBin;
    DBinarySearch* binSearch;
};

AdvPrintIntroPage::AdvPrintIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                       "<p><h1><b>Welcome to Print Creator</b></h1></p>"
                       "<p>This assistant will guide you to assemble images "
                       "to be printed following specific templates as Photo Album, "
                       "Photo Collage, or Framed Photo.</p>"
                       "<p>An adaptive photo collection page layout can be also used, "
                       "based on Atkins algorithm.</p>"
                       "</qt>"));

    QGroupBox* const binaryBox      = new QGroupBox(vbox);
    QGridLayout* const binaryLayout = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "Optional Gimp Binaries"));
    d->binSearch = new DBinarySearch(binaryBox);
    d->binSearch->addBinary(d->gimpBin);

#ifdef Q_OS_OSX
    d->binSearch->addDirectory(QLatin1String("/Applications/Gimp.app/Contents/MacOS"));                // Gimp bundle PKG install
    d->binSearch->addDirectory(QLatin1String("/opt/local/bin"));                    // Std Macports install
    d->binSearch->addDirectory(QLatin1String("/opt/digikam/bin"));                  // digiKam Bundle PKG install
#endif

#ifdef Q_OS_WIN
    d->binSearch->addDirectory(QLatin1String("C:/Program Files/GIMP 2/bin"));
    
    d->binSearch->addDirectory(QLatin1String("C:/Program Files (x86)/GIMP 2/bin"));
#endif

    vbox->setStretchFactor(desc,      10);
    vbox->setStretchFactor(binaryBox, 5);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("document-print")));
}

AdvPrintIntroPage::~AdvPrintIntroPage()
{
    delete d;
}

QString AdvPrintIntroPage::gimpPath() const
{
    return d->gimpBin.isValid() ? d->gimpBin.path() : QString();
}

void AdvPrintIntroPage::initializePage()
{
    d->binSearch->allBinariesFound();
}

} // namespace Digikam
