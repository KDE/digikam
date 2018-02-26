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
#include <QIcon>
#include <QGroupBox>
#include <QGridLayout>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "gimpbinary.h"
#include "dbinarysearch.h"
#include "advprintwizard.h"

namespace Digikam
{

class AdvPrintIntroPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : imageGetOption(0),
        hbox(0),
        binSearch(0),
        iface(0)
    {
        wizard = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    QComboBox*      imageGetOption;
    DHBox*          hbox;
    GimpBinary      gimpBin;
    DBinarySearch*  binSearch;
    AdvPrintWizard* wizard;
    DInfoInterface* iface;
};

AdvPrintIntroPage::AdvPrintIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
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

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(AdvPrintSettings::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(AdvPrintSettings::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    // ---------------------

    QGroupBox* const binaryBox      = new QGroupBox(vbox);
    QGridLayout* const binaryLayout = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "Optional Gimp Binaries"));
    d->binSearch = new DBinarySearch(binaryBox);
    d->binSearch->addBinary(d->gimpBin);

#ifdef Q_OS_OSX
    // Gimp bundle PKG install
    d->binSearch->addDirectory(QLatin1String("/Applications/Gimp.app/Contents/MacOS"));

    // Std Macports install
    d->binSearch->addDirectory(QLatin1String("/opt/local/bin"));

    // digiKam Bundle PKG install
    d->binSearch->addDirectory(QLatin1String("/opt/digikam/bin"));
#endif

#ifdef Q_OS_WIN
    d->binSearch->addDirectory(QLatin1String("C:/Program Files/GIMP 2/bin"));

    d->binSearch->addDirectory(QLatin1String("C:/Program Files (x86)/GIMP 2/bin"));
#endif

    vbox->setStretchFactor(desc,      2);
    vbox->setStretchFactor(d->hbox,   1);
    vbox->setStretchFactor(binaryBox, 3);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("document-print")));
}

AdvPrintIntroPage::~AdvPrintIntroPage()
{
    delete d;
}

void AdvPrintIntroPage::initializePage()
{
    bool albumSupport = (d->iface && d->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(AdvPrintSettings::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->wizard->settings()->selMode);
    }

    d->binSearch->allBinariesFound();
}

bool AdvPrintIntroPage::validatePage()
{
    d->wizard->settings()->selMode  = (AdvPrintSettings::Selection)d->imageGetOption->currentIndex();
    d->wizard->settings()->gimpPath = d->gimpBin.isValid() ? d->gimpBin.path() : QString();

    return true;
}

} // namespace Digikam
