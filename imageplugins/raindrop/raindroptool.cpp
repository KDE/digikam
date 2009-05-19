/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a plugin to add rain drop over an image
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


#include "raindroptool.h"
#include "raindroptool.moc"

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "daboutdata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "raindrop.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamRainDropImagesPlugin
{

class RainDropToolPriv
{
public:

    RainDropToolPriv()
    {
        dropInput     = 0;
        amountInput   = 0;
        coeffInput    = 0;
        previewWidget = 0;
        gboxSettings  = 0;
    }

    RIntNumInput*       dropInput;
    RIntNumInput*       amountInput;
    RIntNumInput*       coeffInput;

    ImageWidget*        previewWidget;
    EditorToolSettings* gboxSettings;
};

RainDropTool::RainDropTool(QObject* parent)
            : EditorToolThreaded(parent),
              d(new RainDropToolPriv)
{
    setObjectName("raindrops");
    setToolName(i18n("Raindrops"));
    setToolIcon(SmallIcon("raindrop"));

    d->previewWidget = new ImageWidget("raindrops Tool", 0,
                                      i18n("This is the preview of the Raindrop effect."
                                           "<p>Note: if you have previously selected an area in the editor, "
                                           "this will be unaffected by the filter. You can use this method to "
                                           "disable the Raindrops effect on a human face, for example.</p>"),
                                      false);

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel);

    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());

    QLabel *label1 = new QLabel(i18n("Drop size:"), d->gboxSettings->plainPage());

    d->dropInput = new RIntNumInput(d->gboxSettings->plainPage());
    d->dropInput->setRange(0, 200, 1);
    d->dropInput->setSliderEnabled(true);
    d->dropInput->setDefaultValue(80);
    d->dropInput->setWhatsThis( i18n("Set here the raindrops' size."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Number:"), d->gboxSettings->plainPage());

    d->amountInput = new RIntNumInput(d->gboxSettings->plainPage());
    d->amountInput->setRange(1, 500, 1);
    d->amountInput->setSliderEnabled(true);
    d->amountInput->setDefaultValue(150);
    d->amountInput->setWhatsThis( i18n("This value controls the maximum number of raindrops."));

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Fish eyes:"), d->gboxSettings->plainPage());

    d->coeffInput = new RIntNumInput(d->gboxSettings->plainPage());
    d->coeffInput->setRange(1, 100, 1);
    d->coeffInput->setSliderEnabled(true);
    d->coeffInput->setDefaultValue(30);
    d->coeffInput->setWhatsThis( i18n("This value is the fish-eye-effect optical "
                                     "distortion coefficient."));

    gridSettings->addWidget(label1,         0, 0, 1, 3);
    gridSettings->addWidget(d->dropInput,   1, 0, 1, 3);
    gridSettings->addWidget(label2,         2, 0, 1, 3);
    gridSettings->addWidget(d->amountInput, 3, 0, 1, 3);
    gridSettings->addWidget(label3,         4, 0, 1, 3);
    gridSettings->addWidget(d->coeffInput,  5, 0, 1, 3);
    gridSettings->setRowStretch(6, 10);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(0);

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->dropInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->amountInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->coeffInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

RainDropTool::~RainDropTool()
{
}

void RainDropTool::renderingFinished()
{
    d->dropInput->setEnabled(true);
    d->amountInput->setEnabled(true);
    d->coeffInput->setEnabled(true);
}

void RainDropTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("raindrops Tool");

    blockWidgetSignals(true);

    d->dropInput->setValue(group.readEntry("DropAdjustment", d->dropInput->defaultValue()));
    d->amountInput->setValue(group.readEntry("AmountAdjustment", d->amountInput->defaultValue()));
    d->coeffInput->setValue(group.readEntry("CoeffAdjustment", d->coeffInput->defaultValue()));

    blockWidgetSignals(false);

    slotEffect();
}

void RainDropTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("raindrops Tool");
    group.writeEntry("DropAdjustment", d->dropInput->value());
    group.writeEntry("AmountAdjustment", d->amountInput->value());
    group.writeEntry("CoeffAdjustment", d->coeffInput->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void RainDropTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->dropInput->slotReset();
    d->amountInput->slotReset();
    d->coeffInput->slotReset();

    blockWidgetSignals(false);

    slotEffect();
}

void RainDropTool::prepareEffect()
{
    d->dropInput->setEnabled(false);
    d->amountInput->setEnabled(false);
    d->coeffInput->setEnabled(false);

    int drop   = d->dropInput->value();
    int amount = d->amountInput->value();
    int coeff  = d->coeffInput->value();

    ImageIface* iface = d->previewWidget->imageIface();

    // Selected data from the image
    QRect selection( iface->selectedXOrg(), iface->selectedYOrg(),
                     iface->selectedWidth(), iface->selectedHeight() );

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new RainDrop(iface->getOriginalImg(), this, drop, amount, coeff, &selection)));
}

void RainDropTool::prepareFinal()
{
    d->dropInput->setEnabled(false);
    d->amountInput->setEnabled(false);
    d->coeffInput->setEnabled(false);

    int drop   = d->dropInput->value();
    int amount = d->amountInput->value();
    int coeff  = d->coeffInput->value();

    ImageIface iface(0, 0);

    // Selected data from the image
    QRect selection( iface.selectedXOrg(), iface.selectedYOrg(),
                     iface.selectedWidth(), iface.selectedHeight() );

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new RainDrop(iface.getOriginalImg(), this, drop, amount, coeff, &selection)));
}

void RainDropTool::putPreviewData(void)
{
    ImageIface* iface = d->previewWidget->imageIface();

    DImg imDest = filter()->getTargetImage()
            .smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    d->previewWidget->updatePreview();
}

void RainDropTool::putFinalData(void)
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("RainDrop"),
                           filter()->getTargetImage().bits());
}

void RainDropTool::blockWidgetSignals(bool b)
{
    d->dropInput->blockSignals(b);
    d->amountInput->blockSignals(b);
    d->coeffInput->blockSignals(b);
}

}  // namespace DigikamRainDropImagesPlugin
