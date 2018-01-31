/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor tool to emboss
 *               an image.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "embosstool.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "editortoolsettings.h"
#include "embossfilter.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace Digikam
{

class EmbossTool::Private
{
public:

    Private() :
        depthInput(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configDepthAdjustmentEntry;

    DIntNumInput*        depthInput;
    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString EmbossTool::Private::configGroupName(QLatin1String("emboss Tool"));
const QString EmbossTool::Private::configDepthAdjustmentEntry(QLatin1String("DepthAdjustment"));

// --------------------------------------------------------

EmbossTool::EmbossTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("emboss"));
    setToolName(i18n("Emboss"));
    setToolIcon(QIcon::fromTheme(QLatin1String("embosstool")));
    setInitPreview(true);

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings;
    d->previewWidget = new ImageRegionWidget;

    // -------------------------------------------------------------

    QLabel* label1 = new QLabel(i18n("Depth:"));
    d->depthInput  = new DIntNumInput;
    d->depthInput->setRange(10, 300, 1);
    d->depthInput->setDefaultValue(30);
    d->depthInput->setWhatsThis( i18n("Set here the depth of the embossing image effect.") );

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,        0, 0, 1, 2);
    mainLayout->addWidget(d->depthInput, 1, 0, 1, 2);
    mainLayout->setRowStretch(2, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
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
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->depthInput->blockSignals(true);
    d->depthInput->setValue(group.readEntry(d->configDepthAdjustmentEntry, d->depthInput->defaultValue()));
    d->depthInput->blockSignals(false);
}

void EmbossTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
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

}  // namespace Digikam
