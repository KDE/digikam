/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to blur an image
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
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


#include "blurtool.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimggaussianblur.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

class BlurToolPriv
{
public:

    BlurToolPriv() :
        configGroupName("gaussianblur Tool"),
        configRadiusAdjustmentEntry("RadiusAdjustment"),

        radiusInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configRadiusAdjustmentEntry;

    RDoubleNumInput*    radiusInput;
    ImagePanelWidget*   previewWidget;
    EditorToolSettings* gboxSettings;
};

BlurTool::BlurTool(QObject* parent)
        : EditorToolThreaded(parent),
          d(new BlurToolPriv)
{
    setObjectName("gaussianblur");
    setToolName(i18n("Blur"));
    setToolIcon(SmallIcon("blurimage"));
    setToolHelp("blursharpentool.anchor");

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::PanIcon);

    d->previewWidget = new ImagePanelWidget(470, 350, "gaussianblur Tool", d->gboxSettings->panIconView());

    // --------------------------------------------------------

    QLabel *label  = new QLabel(i18n("Smoothness:"));
    d->radiusInput = new RDoubleNumInput();
    d->radiusInput->setRange(0.0, 120.0, 0.1);
    d->radiusInput->setDefaultValue(0.0);
    d->radiusInput->setWhatsThis(i18n("A smoothness of 0 has no effect, "
                                      "1 and above determine the Gaussian blur matrix radius "
                                      "that determines how much to blur the image."));

    // --------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout( );
    mainLayout->addWidget(label,          0, 0, 1, 2);
    mainLayout->addWidget(d->radiusInput, 1, 0, 1, 2);
    mainLayout->setRowStretch(2, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // --------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    init();

    // --------------------------------------------------------

    connect(d->radiusInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotTimer()));
}

BlurTool::~BlurTool()
{
    delete d;
}

void BlurTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->radiusInput->setValue(group.readEntry(d->configRadiusAdjustmentEntry, d->radiusInput->defaultValue()));
}

void BlurTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configRadiusAdjustmentEntry, d->radiusInput->value());
    d->previewWidget->writeSettings();
    config->sync();
}

void BlurTool::slotResetSettings()
{
    d->radiusInput->blockSignals(true);
    d->radiusInput->slotReset();
    d->radiusInput->blockSignals(false);
}

void BlurTool::prepareEffect()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->radiusInput->setEnabled(false);
    DImg img = d->previewWidget->getOriginalRegionImage();
    setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgGaussianBlur(&img, this, d->radiusInput->value())));
}

void BlurTool::prepareFinal()
{
    d->radiusInput->setEnabled(false);

    ImageIface iface(0, 0);
    uchar *data      = iface.getOriginalImage();
    int w            = iface.originalWidth();
    int h            = iface.originalHeight();
    bool sixteenBit  = iface.originalSixteenBit();
    bool hasAlpha    = iface.originalHasAlpha();
    DImg orgImage = DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;
    setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgGaussianBlur(&orgImage, this, d->radiusInput->value())));
}

void BlurTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(imDest);
}

void BlurTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg imDest = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Gaussian Blur"), imDest.bits());
}

void BlurTool::renderingFinished(void)
{
    QApplication::restoreOverrideCursor();
    d->radiusInput->setEnabled(true);
}

}  // namespace DigikamImagesPluginCore
