/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin to emboss
 *               an image.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "embosstool.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>

// KDE includes

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
#include "embossfilter.h"
#include "imageiface.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;

namespace DigikamFxFiltersImagePlugin
{

class EmbossTool::Private
{
public:

    Private() :
        depthInput(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configDepthAdjustmentEntry;

    RIntNumInput*        depthInput;
    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString EmbossTool::Private::configGroupName("emboss Tool");
const QString EmbossTool::Private::configDepthAdjustmentEntry("DepthAdjustment");

// --------------------------------------------------------

EmbossTool::EmbossTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName("emboss");
    setToolName(i18n("Emboss"));
    setToolIcon(SmallIcon("embosstool"));
    setInitPreview(true);

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings;
    d->previewWidget = new ImageRegionWidget;

    // -------------------------------------------------------------

    QLabel* label1 = new QLabel(i18n("Depth:"));
    d->depthInput  = new RIntNumInput;
    d->depthInput->setRange(10, 300, 1);
    d->depthInput->setSliderEnabled(true);
    d->depthInput->setDefaultValue(30);
    d->depthInput->setWhatsThis( i18n("Set here the depth of the embossing image effect.") );

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,        0, 0, 1, 2);
    mainLayout->addWidget(d->depthInput, 1, 0, 1, 2);
    mainLayout->setRowStretch(2, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    connect(d->depthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

EmbossTool::~EmbossTool()
{
    delete d;
}

void EmbossTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->depthInput->blockSignals(true);
    d->depthInput->setValue(group.readEntry(d->configDepthAdjustmentEntry, d->depthInput->defaultValue()));
    d->depthInput->blockSignals(false);
}

void EmbossTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configDepthAdjustmentEntry, d->depthInput->value());
    group.sync();
}

void EmbossTool::slotResetSettings()
{
    d->depthInput->blockSignals(true);
    d->depthInput->slotReset();
    d->depthInput->blockSignals(false);

    slotPreview();
}

void EmbossTool::preparePreview()
{
    DImg image = d->previewWidget->getOriginalRegionImage();
    int depth  = d->depthInput->value();

    setFilter(new EmbossFilter(&image, this, depth));
}

void EmbossTool::prepareFinal()
{
    int depth = d->depthInput->value();

    ImageIface iface;
    setFilter(new EmbossFilter(iface.original(), this, depth));
}

void EmbossTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void EmbossTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Emboss"), filter()->filterAction(), filter()->getTargetImage());
}

}  // namespace DigikamFxFiltersImagePlugin
