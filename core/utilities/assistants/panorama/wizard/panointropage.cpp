/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending tool
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "panointropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dbinarysearch.h"
#include "autooptimiserbinary.h"
#include "cpcleanbinary.h"
#include "cpfindbinary.h"
#include "enblendbinary.h"
#include "makebinary.h"
#include "nonabinary.h"
#include "panomodifybinary.h"
#include "pto2mkbinary.h"
#include "huginexecutorbinary.h"
#include "dlayoutbox.h"

namespace Digikam
{

class PanoIntroPage::Private
{
public:

    explicit Private(PanoManager* const m)
        : mngr(m),
//        addGPlusMetadataCheckBox(0),

          // TODO HDR
//        hdrCheckBox(0),

          formatGroupBox(0),
          settingsGroupBox(0),
          jpegRadioButton(0),
          tiffRadioButton(0),
          hdrRadioButton(0),
          binariesWidget(0)
    {
    }

    PanoManager*        mngr;

//  QCheckBox*      addGPlusMetadataCheckBox;

    // TODO HDR
//  QCheckBox*      hdrCheckBox;

    QGroupBox*      formatGroupBox;
    QGroupBox*      settingsGroupBox;
    QRadioButton*   jpegRadioButton;
    QRadioButton*   tiffRadioButton;
    QRadioButton*   hdrRadioButton;
    DBinarySearch*  binariesWidget;
};

PanoIntroPage::PanoIntroPage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "<b>Welcome to Panorama Tool</b>")),
      d(new Private(mngr))
{
    DVBox* const vbox   = new DVBox(this);

    QLabel* const title = new QLabel(vbox);
    title->setWordWrap(true);
    title->setOpenExternalLinks(true);
    title->setText(i18n("<qt>"
                        "<p><h1><b>Welcome to Panorama Tool</b></h1></p>"
                        "<p>This tool stitches several images together to create a panorama, making the "
                        "seam between images not visible.</p>"
                        "<p>This assistant will help you to configure how to import images before "
                        "stitching them into a panorama.</p>"
                        "<p>Images must be taken from the same point of view.</p>"
                        "<p>For more information, please take a look at "
                        "<a href='http://hugin.sourceforge.net/tutorials/overview/en.shtml'>this page</a></p>"
                        "</qt>"));

    QGroupBox* const binaryBox        = new QGroupBox(vbox);
    QGridLayout* const binaryLayout   = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "Panorama Binaries"));
    d->binariesWidget = new DBinarySearch(binaryBox);
    d->binariesWidget->addBinary(d->mngr->autoOptimiserBinary());
    d->binariesWidget->addBinary(d->mngr->cpCleanBinary());
    d->binariesWidget->addBinary(d->mngr->cpFindBinary());
    d->binariesWidget->addBinary(d->mngr->enblendBinary());
    d->binariesWidget->addBinary(d->mngr->makeBinary());
    d->binariesWidget->addBinary(d->mngr->nonaBinary());
    d->binariesWidget->addBinary(d->mngr->panoModifyBinary());

    d->mngr->checkForHugin2015();

    if (d->mngr->hugin2015())
    {
        d->binariesWidget->addBinary(d->mngr->huginExecutorBinary());
    }
    else
    {
        d->binariesWidget->addBinary(d->mngr->pto2MkBinary());
    }

    d->mngr->checkBinaries();

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

/*
    QVBoxLayout* const settingsVBox = new QVBoxLayout();
    d->settingsGroupBox             = new QGroupBox(i18nc("@title:group", "Panorama Settings"), this);
    d->settingsGroupBox->setLayout(settingsVBox);

    d->addGPlusMetadataCheckBox     = new QCheckBox(i18nc("@option:check", "Add Photosphere Metadata"), d->settingsGroupBox);
    d->addGPlusMetadataCheckBox->setToolTip(i18nc("@info:tooltip", "Add Exif metadata to the output panorama image for Google+ 3D viewer"));
    d->addGPlusMetadataCheckBox->setWhatsThis(i18nc("@info:whatsthis", "<b>Add Photosphere Metadata</b>: Enabling this allows the program to add "
                                                    "metadata to the output image such that when uploaded to Google+, the "
                                                    "Google+ 3D viewer is activated and the panorama can be seen in 3D. Note "
                                                    "that this feature is most insteresting for large panoramas."));
    settingsVBox->addWidget(d->addGPlusMetadataCheckBox);
    vbox->addWidget(d->settingsGroupBox);
*/
    QVBoxLayout* const formatVBox = new QVBoxLayout();
    d->formatGroupBox             = new QGroupBox(i18nc("@title:group", "File Format"), vbox);
    d->formatGroupBox->setLayout(formatVBox);
    QButtonGroup* const group     = new QButtonGroup();

    d->jpegRadioButton            = new QRadioButton(i18nc("@option:radio", "JPEG output"), d->formatGroupBox);
    // The following comment is to get the next string extracted for translation
    // xgettext: no-c-format
    d->jpegRadioButton->setToolTip(i18nc("@info:tooltip", "Selects a JPEG output with 90% compression rate "
                                         "(lossy compression, smaller size)."));
    d->jpegRadioButton->setWhatsThis(i18nc("@info:whatsthis", "<b>JPEG output</b>: Using JPEG output, the panorama file will be smaller "
                                           "at the cost of information loss during compression. This is the easiest "
                                           "way to share the result, or print it online or in a shop."));
    formatVBox->addWidget(d->jpegRadioButton);
    group->addButton(d->jpegRadioButton);

    d->tiffRadioButton          = new QRadioButton(i18nc("@option:radio", "TIFF output"), d->formatGroupBox);
    d->tiffRadioButton->setToolTip(i18nc("@info:tooltip", "Selects a TIFF output compressed using the LZW algorithm "
                                         "(lossless compression, bigger size)."));
    d->tiffRadioButton->setWhatsThis(i18nc("@info:whatsthis", "<b>TIFF output</b>: Using TIFF output, you get the same color depth than "
                                           "your original photos using RAW images at the cost of a bigger panorama file."));
    formatVBox->addWidget(d->tiffRadioButton);
    group->addButton(d->tiffRadioButton);

    // TODO HDR
/*
    d->hdrRadioButton           = new QRadioButton(i18nc("@option:radio", "HDR output"), d->formatGroupBox);
    d->hdrRadioButton->setToolTip(i18nc("@info:tooltip", "Selects an High Dynamic Range (HDR) image, that can be processed further "
                                        "with a dedicated software."));
    d->hdrRadioButton->setWhatsThis(i18nc("@info:whatsthis", "<b>HDR output</b>: Output in High Dynamic Range, meaning that every piece of "
                                          "information contained in the original photos are preserved. Note that you "
                                          "need another software to process the resulting panorama, like "
                                          "<a href=\"http://qtpfsgui.sourceforge.net/\">Luminance HDR</a>"));
    formatVBox->addWidget(d->hdrRadioButton);
    group->addButton(d->hdrRadioButton);
*/

    switch (d->mngr->format())
    {
        case JPEG:
            d->jpegRadioButton->setChecked(true);
            break;
        case TIFF:
            d->tiffRadioButton->setChecked(true);
            break;
        case HDR:
            // TODO HDR
//             d->hdrRadioButton->setChecked(true);
            break;
    }

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-tripod.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

/*
    connect(d->addGPlusMetadataCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotToggleGPano(int)));

    d->addGPlusMetadataCheckBox->setChecked(d->mngr->gPano());
*/
    slotToggleGPano(0);  // Disabled for the moment

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotChangeFileFormat(QAbstractButton*)));

    connect(d->binariesWidget, SIGNAL(signalBinariesFound(bool)),
            this, SLOT(slotBinariesChanged(bool)));

    // TODO HDR
//   d->hdrCheckBox->setChecked(d->mngr->hdr());
}

PanoIntroPage::~PanoIntroPage()
{
    delete d;
}

bool PanoIntroPage::binariesFound()
{
    return d->binariesWidget->allBinariesFound();
}

void PanoIntroPage::slotToggleGPano(int state)
{
    d->mngr->setGPano(state);
}

void PanoIntroPage::slotChangeFileFormat(QAbstractButton* button)
{
    if (button == d->jpegRadioButton)
        d->mngr->setFileFormatJPEG();
    else if (button == d->tiffRadioButton)
        d->mngr->setFileFormatTIFF();
    else if (button == d->hdrRadioButton)
        d->mngr->setFileFormatHDR();
}

void PanoIntroPage::slotBinariesChanged(bool found)
{
    setComplete(found);
    emit completeChanged();
}

    // TODO HDR
/*
void PanoIntroPage::slotShowFileFormat(int state)
{
    d->mngr->setHDR(state);
    if (state)
    {
        d->formatGroupBox->setEnabled(false);
    }
    else
    {
        d->formatGroupBox->setEnabled(true);
    }
}
*/

void PanoIntroPage::initializePage()
{
    setComplete(d->binariesWidget->allBinariesFound());
    emit completeChanged();
}

} // namespace Digikam
