/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a tool to add rain drop over an image
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

#include "raindroptool.h"

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "raindropfilter.h"

namespace Digikam
{

class RainDropTool::Private
{
public:

    Private() :
        dropInput(0),
        amountInput(0),
        coeffInput(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configDropAdjustmentEntry;
    static const QString configAmountAdjustmentEntry;
    static const QString configCoeffAdjustmentEntry;

    DIntNumInput*        dropInput;
    DIntNumInput*        amountInput;
    DIntNumInput*        coeffInput;

    ImageGuideWidget*    previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString RainDropTool::Private::configGroupName(QLatin1String("raindrops Tool"));
const QString RainDropTool::Private::configDropAdjustmentEntry(QLatin1String("DropAdjustment"));
const QString RainDropTool::Private::configAmountAdjustmentEntry(QLatin1String("AmountAdjustment"));
const QString RainDropTool::Private::configCoeffAdjustmentEntry(QLatin1String("CoeffAdjustment"));

// --------------------------------------------------------

RainDropTool::RainDropTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("raindrops"));
    setToolName(i18n("Raindrops"));
    setToolIcon(QIcon::fromTheme(QLatin1String("raindrop")));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    d->previewWidget->setWhatsThis(i18n("This is the preview of the Raindrop effect."
                                        "<p>Note: if you have previously selected an area in the editor, "
                                        "this will be unaffected by the filter. You can use this method to "
                                        "disable the Raindrops effect on a human face, for example.</p>"));

    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Try|
                                EditorToolSettings::Cancel);


    // -------------------------------------------------------------

    QLabel* label1 = new QLabel(i18n("Drop size:"));
    d->dropInput   = new DIntNumInput;
    d->dropInput->setRange(0, 200, 1);
    d->dropInput->setDefaultValue(80);
    d->dropInput->setWhatsThis( i18n("Set here the raindrops' size."));

    // -------------------------------------------------------------

    QLabel* label2 = new QLabel(i18n("Number:"));
    d->amountInput = new DIntNumInput;
    d->amountInput->setRange(1, 500, 1);
    d->amountInput->setDefaultValue(150);
    d->amountInput->setWhatsThis( i18n("This value controls the maximum number of raindrops."));

    // -------------------------------------------------------------

    QLabel* label3 = new QLabel(i18n("Fish eyes:"));
    d->coeffInput  = new DIntNumInput;
    d->coeffInput->setRange(1, 100, 1);
    d->coeffInput->setDefaultValue(30);
    d->coeffInput->setWhatsThis( i18n("This value is the fish-eye-effect optical "
                                      "distortion coefficient."));

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,         0, 0, 1, 3);
    mainLayout->addWidget(d->dropInput,   1, 0, 1, 3);
    mainLayout->addWidget(label2,         2, 0, 1, 3);
    mainLayout->addWidget(d->amountInput, 3, 0, 1, 3);
    mainLayout->addWidget(label3,         4, 0, 1, 3);
    mainLayout->addWidget(d->coeffInput,  5, 0, 1, 3);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
}

RainDropTool::~RainDropTool()
{
    delete d;
}

void RainDropTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    d->dropInput->setValue(group.readEntry(d->configDropAdjustmentEntry,     d->dropInput->defaultValue()));
    d->amountInput->setValue(group.readEntry(d->configAmountAdjustmentEntry, d->amountInput->defaultValue()));
    d->coeffInput->setValue(group.readEntry(d->configCoeffAdjustmentEntry,   d->coeffInput->defaultValue()));

    blockWidgetSignals(false);
}

void RainDropTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configDropAdjustmentEntry,   d->dropInput->value());
    group.writeEntry(d->configAmountAdjustmentEntry, d->amountInput->value());
    group.writeEntry(d->configCoeffAdjustmentEntry,  d->coeffInput->value());

    group.sync();
}

void RainDropTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->dropInput->slotReset();
    d->amountInput->slotReset();
    d->coeffInput->slotReset();

    blockWidgetSignals(false);

    slotPreview();
}

void RainDropTool::preparePreview()
{
    int drop                = d->dropInput->value();
    int amount              = d->amountInput->value();
    int coeff               = d->coeffInput->value();

    ImageIface* const iface = d->previewWidget->imageIface();

    // Selected data from the image
    QRect selection         = iface->selectionRect();

    setFilter(new RainDropFilter(iface->original(), this, drop, amount, coeff, selection));
}

void RainDropTool::prepareFinal()
{
    int drop        = d->dropInput->value();
    int amount      = d->amountInput->value();
    int coeff       = d->coeffInput->value();

    ImageIface iface;

    // Selected data from the image
    QRect selection = iface.selectionRect();

    setFilter(new RainDropFilter(iface.original(), this, drop, amount, coeff, selection));
}

void RainDropTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg imDest             = filter()->getTargetImage().smoothScale(iface->previewSize());
    iface->setPreview(imDest);

    d->previewWidget->updatePreview();
}

void RainDropTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("RainDrop"), filter()->filterAction(), filter()->getTargetImage());
}

void RainDropTool::blockWidgetSignals(bool b)
{
    d->dropInput->blockSignals(b);
    d->amountInput->blockSignals(b);
    d->coeffInput->blockSignals(b);
}

}  // namespace Digikam
