/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digikam image editor plugin to
 *               simulate charcoal drawing.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "charcoaltool.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "charcoalfilter.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;

namespace DigikamFxFiltersImagePlugin
{

class CharcoalTool::Private
{
public:

    Private() :
        pencilInput(0),
        smoothInput(0),
        previewWidget(0),
        gboxSettings(0)
    {}


    static const QString configGroupName;
    static const QString configPencilAdjustmentEntry;
    static const QString configSmoothAdjustmentEntry;

    RIntNumInput*        pencilInput;
    RIntNumInput*        smoothInput;

    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString CharcoalTool::Private::configGroupName("charcoal Tool");
const QString CharcoalTool::Private::configPencilAdjustmentEntry("PencilAdjustment");
const QString CharcoalTool::Private::configSmoothAdjustmentEntry("SmoothAdjustment");

// --------------------------------------------------------

CharcoalTool::CharcoalTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName("charcoal");
    setToolName(i18n("Charcoal"));
    setToolIcon(SmallIcon("charcoaltool"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    d->previewWidget = new ImageRegionWidget;

    // -------------------------------------------------------------

    QLabel* label1 = new QLabel(i18n("Pencil size:"));
    d->pencilInput = new RIntNumInput;
    d->pencilInput->setRange(1, 100, 1);
    d->pencilInput->setSliderEnabled(true);
    d->pencilInput->setDefaultValue(5);
    d->pencilInput->setWhatsThis( i18n("Set here the charcoal pencil size used to simulate the drawing."));

    // -------------------------------------------------------------

    QLabel* label2 = new QLabel(i18nc("smoothing value of the pencil", "Smooth:"));
    d->smoothInput = new RIntNumInput;
    d->smoothInput->setRange(1, 100, 1);
    d->smoothInput->setSliderEnabled(true);
    d->smoothInput->setDefaultValue(10);
    d->smoothInput->setWhatsThis( i18n("This value controls the smoothing effect of the pencil "
                                       "under the canvas."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,         0, 0, 1, 2);
    mainLayout->addWidget(d->pencilInput, 1, 0, 1, 2);
    mainLayout->addWidget(label2,         2, 0, 1, 2);
    mainLayout->addWidget(d->smoothInput, 3, 0, 1, 2);
    mainLayout->setRowStretch(4, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setPreviewModeMask(PreviewToolBar::AllPreviewModes);
    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
}

CharcoalTool::~CharcoalTool()
{
    delete d;
}

void CharcoalTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->pencilInput->blockSignals(true);
    d->smoothInput->blockSignals(true);
    d->pencilInput->setValue(group.readEntry(d->configPencilAdjustmentEntry, d->pencilInput->defaultValue()));
    d->smoothInput->setValue(group.readEntry(d->configSmoothAdjustmentEntry, d->smoothInput->defaultValue()));
    d->pencilInput->blockSignals(false);
    d->smoothInput->blockSignals(false);
}

void CharcoalTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configPencilAdjustmentEntry, d->pencilInput->value());
    group.writeEntry(d->configSmoothAdjustmentEntry, d->smoothInput->value());
    config->sync();
}

void CharcoalTool::slotResetSettings()
{
    d->pencilInput->blockSignals(true);
    d->smoothInput->blockSignals(true);
    d->pencilInput->slotReset();
    d->smoothInput->slotReset();
    d->pencilInput->blockSignals(false);
    d->smoothInput->blockSignals(false);
}

void CharcoalTool::preparePreview()
{
    double pencil = (double)d->pencilInput->value()/10.0;
    double smooth = (double)d->smoothInput->value();
    DImg image    = d->previewWidget->getOriginalRegionImage();

    setFilter(new CharcoalFilter(&image, this, pencil, smooth));
}

void CharcoalTool::prepareFinal()
{
    double pencil = (double)d->pencilInput->value()/10.0;
    double smooth = (double)d->smoothInput->value();

    ImageIface iface;
    setFilter(new CharcoalFilter(iface.original(), this, pencil, smooth));
}

void CharcoalTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void CharcoalTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Charcoal"), filter()->filterAction(), filter()->getTargetImage());
}

}  // namespace DigikamFxFiltersImagePlugin
