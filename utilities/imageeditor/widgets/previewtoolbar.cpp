/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-10
 * Description : a tool bar for preview mode
 *
 * Copyright (C) 2010 by Gilles Caulier<caulier dot gilles at gmail dot com>
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

#include "previewtoolbar.moc"

// Qt includes

#include <QButtonGroup>
#include <QLayout>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

namespace Digikam
{

class PreviewToolBarPriv
{

public:

    PreviewToolBarPriv()
    {
        previewButtons = 0;
    }

    QButtonGroup* previewButtons;
};

PreviewToolBar::PreviewToolBar(QWidget* parent)
              : QWidget(parent), d(new PreviewToolBarPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QHBoxLayout *hlay = new QHBoxLayout(this);
    d->previewButtons = new QButtonGroup(this);
    d->previewButtons->setExclusive(true);
    hlay->setSpacing(0);
    hlay->setMargin(0);

    QToolButton *previewOriginalButton = new QToolButton(this);
    d->previewButtons->addButton(previewOriginalButton, PreviewOriginalImage);
    hlay->addWidget(previewOriginalButton);
    previewOriginalButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/original.png")));
    previewOriginalButton->setCheckable(true);
    previewOriginalButton->setWhatsThis( i18n( "If this option is enabled, the original image "
                                               "will be shown." ) );

    QToolButton *previewBothButtonVert = new QToolButton(this);
    d->previewButtons->addButton(previewBothButtonVert, PreviewBothImagesVertCont);
    hlay->addWidget(previewBothButtonVert);
    previewBothButtonVert->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/bothvert.png")));
    previewBothButtonVert->setCheckable(true);
    previewBothButtonVert->setWhatsThis( i18n( "If this option is enabled, the preview area will "
                                               "split vertically. "
                                               "A contiguous area of the image will be shown, "
                                               "with one half from the original image, "
                                               "the other half from the target image.") );

    QToolButton *previewBothButtonHorz = new QToolButton(this);
    d->previewButtons->addButton(previewBothButtonHorz, PreviewBothImagesHorzCont);
    hlay->addWidget(previewBothButtonHorz);
    previewBothButtonHorz->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/bothhorz.png")));
    previewBothButtonHorz->setCheckable(true);
    previewBothButtonHorz->setWhatsThis( i18n( "If this option is enabled, the preview area will "
                                               "split horizontally. "
                                               "A contiguous area of the image will be shown, "
                                               "with one half from the original image, "
                                               "the other half from the target image.") );

    QToolButton *previewDuplicateBothButtonVert = new QToolButton(this);
    d->previewButtons->addButton(previewDuplicateBothButtonVert, PreviewBothImagesVert);
    hlay->addWidget(previewDuplicateBothButtonVert);
    previewDuplicateBothButtonVert->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/duplicatebothvert.png")));
    previewDuplicateBothButtonVert->setCheckable(true);
    previewDuplicateBothButtonVert->setWhatsThis( i18n( "If this option is enabled, the preview area will "
                                                        "split vertically. "
                                                        "The same part of the original and the target image "
                                                        "will be shown side by side.") );

    QToolButton *previewDupplicateBothButtonHorz = new QToolButton(this);
    d->previewButtons->addButton(previewDupplicateBothButtonHorz, PreviewBothImagesHorz);
    hlay->addWidget(previewDupplicateBothButtonHorz);
    previewDupplicateBothButtonHorz->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/duplicatebothhorz.png")));
    previewDupplicateBothButtonHorz->setCheckable(true);
    previewDupplicateBothButtonHorz->setWhatsThis( i18n( "If this option is enabled, the preview area will "
                                                         "split horizontally. "
                                                         "The same part of the original and the target image "
                                                         "will be shown side by side.") );

    QToolButton *previewtargetButton = new QToolButton(this);
    d->previewButtons->addButton(previewtargetButton, PreviewTargetImage);
    hlay->addWidget(previewtargetButton);
    previewtargetButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/target.png")));
    previewtargetButton->setCheckable(true);
    previewtargetButton->setWhatsThis( i18n( "If this option is enabled, the target image "
                                             "will be shown." ) );

    QToolButton *previewToggleMouseOverButton = new QToolButton(this);
    d->previewButtons->addButton(previewToggleMouseOverButton, PreviewToggleOnMouseOver);
    hlay->addWidget(previewToggleMouseOverButton);
    previewToggleMouseOverButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/togglemouseover.png")));
    previewToggleMouseOverButton->setCheckable(true);
    previewToggleMouseOverButton->setWhatsThis( i18n( "If this option is enabled, the original image will "
                                                      "be shown when the mouse is over image area; otherwise, "
                                                      "the target image will be shown." ) );

    connect(d->previewButtons, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalPreviewModeChanged(int)));
}

PreviewToolBar::~PreviewToolBar()
{
    delete d;
}

}  // namespace Digikam
