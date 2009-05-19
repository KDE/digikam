/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin to emboss
 *               an image.
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


#include "embosstool.h"
#include "embosstool.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "editortoolsettings.h"
#include "emboss.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamEmbossImagesPlugin
{

class EmbossToolPriv
{
public:

    EmbossToolPriv()
    {
        depthInput    = 0;
        previewWidget = 0;
        gboxSettings  = 0;
    }

    RIntNumInput*       depthInput;
    ImagePanelWidget*   previewWidget;
    EditorToolSettings* gboxSettings;
};

EmbossTool::EmbossTool(QObject* parent)
          : EditorToolThreaded(parent),
            d(new EmbossToolPriv)
{
    setObjectName("emboss");
    setToolName(i18n("Emboss"));
    setToolIcon(SmallIcon("embosstool"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel|
                                             EditorToolSettings::PanIcon);

    QGridLayout* grid = new QGridLayout( d->gboxSettings->plainPage() );

    QLabel *label1 = new QLabel(i18n("Depth:"), d->gboxSettings->plainPage());

    d->depthInput   = new RIntNumInput(d->gboxSettings->plainPage());
    d->depthInput->setRange(10, 300, 1);
    d->depthInput->setSliderEnabled(true);
    d->depthInput->setDefaultValue(30);
    d->depthInput->setWhatsThis( i18n("Set here the depth of the embossing image effect.") );

    // -------------------------------------------------------------

    grid->addWidget(label1,        0, 0, 1, 2);
    grid->addWidget(d->depthInput, 1, 0, 1, 2);
    grid->setRowStretch(2, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    d->previewWidget = new ImagePanelWidget(470, 350, "emboss Tool", d->gboxSettings->panIconView());

    setToolView(d->previewWidget);
    init();

    // -------------------------------------------------------------

    connect(d->depthInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
}

EmbossTool::~EmbossTool()
{
    delete d;
}

void EmbossTool::renderingFinished()
{
    d->depthInput->setEnabled(true);
}

void EmbossTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("emboss Tool");
    d->depthInput->blockSignals(true);
    d->depthInput->setValue(group.readEntry("DepthAdjustment", d->depthInput->defaultValue()));
    d->depthInput->blockSignals(false);
}

void EmbossTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("emboss Tool");
    group.writeEntry("DepthAdjustment", d->depthInput->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void EmbossTool::slotResetSettings()
{
    d->depthInput->blockSignals(true);
    d->depthInput->slotReset();
    d->depthInput->blockSignals(false);

    slotEffect();
}

void EmbossTool::prepareEffect()
{
    d->depthInput->setEnabled(false);

    DImg image = d->previewWidget->getOriginalRegionImage();
    int depth  = d->depthInput->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Emboss(&image, this, depth)));
}

void EmbossTool::prepareFinal()
{
    d->depthInput->setEnabled(false);

    int depth = d->depthInput->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new Emboss(iface.getOriginalImg(), this, depth)));
}

void EmbossTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void EmbossTool::putFinalData()
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Emboss"), filter()->getTargetImage().bits());
}

}  // namespace DigikamEmbossImagesPlugin
