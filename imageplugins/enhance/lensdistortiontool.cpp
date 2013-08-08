/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-27
 * Description : a plugin to reduce lens distortions to an image.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "lensdistortiontool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QBrush>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>

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
#include "imageiface.h"
#include "imageguidewidget.h"
#include "lensdistortionfilter.h"

using namespace KDcrawIface;

namespace DigikamEnhanceImagePlugin
{

class LensDistortionTool::Private
{
public:

    Private() :
        maskPreviewLabel(0),
        mainInput(0),
        edgeInput(0),
        rescaleInput(0),
        brightenInput(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString config2ndOrderDistortionEntry;
    static const QString config4thOrderDistortionEntry;
    static const QString configZoomFactorEntry;
    static const QString configBrightenEntry;

    QLabel*              maskPreviewLabel;

    RDoubleNumInput*     mainInput;
    RDoubleNumInput*     edgeInput;
    RDoubleNumInput*     rescaleInput;
    RDoubleNumInput*     brightenInput;

    DImg                 previewRasterImage;

    ImageGuideWidget*    previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString LensDistortionTool::Private::configGroupName("lensdistortion Tool");
const QString LensDistortionTool::Private::config2ndOrderDistortionEntry("2nd Order Distortion");
const QString LensDistortionTool::Private::config4thOrderDistortionEntry("4th Order Distortion");
const QString LensDistortionTool::Private::configZoomFactorEntry("Zoom Factor");
const QString LensDistortionTool::Private::configBrightenEntry("Brighten");

// --------------------------------------------------------

LensDistortionTool::LensDistortionTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName("lensdistortion");
    setToolName(i18n("Lens Distortion"));
    setToolIcon(SmallIcon("lensdistortion"));

    d->previewWidget = new ImageGuideWidget(0, true, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::ColorGuide);

    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());

    d->maskPreviewLabel = new QLabel(d->gboxSettings->plainPage());
    d->maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    d->maskPreviewLabel->setWhatsThis( i18n("You can see here a thumbnail preview of the "
                                            "distortion correction applied to a cross pattern.") );

    // -------------------------------------------------------------

    QLabel* label1 = new QLabel(i18nc("value for amount of distortion", "Main:"), d->gboxSettings->plainPage());

    d->mainInput = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->mainInput->setDecimals(1);
    d->mainInput->input()->setRange(-100.0, 100.0, 0.1, true);
    d->mainInput->setDefaultValue(0.0);
    d->mainInput->setWhatsThis( i18n("This value controls the amount of distortion. Negative values "
                                     "correct lens barrel distortion, while positive values correct lens "
                                     "pincushion distortion."));

    // -------------------------------------------------------------

    QLabel* label2 = new QLabel(i18n("Edge:"), d->gboxSettings->plainPage());

    d->edgeInput = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->edgeInput->setDecimals(1);
    d->edgeInput->input()->setRange(-100.0, 100.0, 0.1, true);
    d->edgeInput->setDefaultValue(0.0);
    d->edgeInput->setWhatsThis( i18n("This value controls in the same manner as the Main control, "
                                     "but has more effect at the edges of the image than at the center."));

    // -------------------------------------------------------------

    QLabel* label3 = new QLabel(i18n("Zoom:"), d->gboxSettings->plainPage());

    d->rescaleInput = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->rescaleInput->setDecimals(1);
    d->rescaleInput->input()->setRange(-100.0, 100.0, 0.1, true);
    d->rescaleInput->setDefaultValue(0.0);
    d->rescaleInput->setWhatsThis( i18n("This value rescales the overall image size."));

    // -------------------------------------------------------------

    QLabel* label4 = new QLabel(i18n("Brighten:"), d->gboxSettings->plainPage());

    d->brightenInput = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->brightenInput->setDecimals(1);
    d->brightenInput->input()->setRange(-100.0, 100.0, 0.1, true);
    d->brightenInput->setDefaultValue(0.0);
    d->brightenInput->setWhatsThis( i18n("This value adjusts the brightness in image corners."));

    // -------------------------------------------------------------

    gridSettings->addWidget(d->maskPreviewLabel, 0, 0, 1, 2);
    gridSettings->addWidget(label1,              1, 0, 1, 2);
    gridSettings->addWidget(d->mainInput,        2, 0, 1, 2);
    gridSettings->addWidget(label2,              3, 0, 1, 2);
    gridSettings->addWidget(d->edgeInput,        4, 0, 1, 2);
    gridSettings->addWidget(label3,              5, 0, 1, 2);
    gridSettings->addWidget(d->rescaleInput,     6, 0, 1, 2);
    gridSettings->addWidget(label4,              7, 0, 1, 2);
    gridSettings->addWidget(d->brightenInput,    8, 0, 1, 2);
    gridSettings->setRowStretch(9, 10);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->mainInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->edgeInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->rescaleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->brightenInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(d->gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));

    // -------------------------------------------------------------

    /* Calc transform preview.
       We would like a checkered area to demonstrate the effect.
       We do not have any drawing support in DImg, so we let Qt draw.
       First we create a white QImage. We convert this to a QPixmap,
       on which we can draw. Then we convert back to QImage,
       convert the QImage to a DImg which we only need to create once, here.
       Later, we apply the effect on a copy and convert the DImg to QPixmap.
       Longing for Qt4 where we can paint directly on the QImage...
    */

    QPixmap pix(120, 120);
    pix.fill(Qt::white);
    QPainter pt(&pix);
    pt.setPen(QPen(Qt::black, 1));
    pt.fillRect(0, 0, pix.width(), pix.height(), QBrush(Qt::black, Qt::CrossPattern));
    pt.drawRect(0, 0, pix.width(), pix.height());
    pt.end();
    QImage preview       = pix.toImage();
    d->previewRasterImage = DImg(preview.width(), preview.height(), false, false, preview.bits());
}

LensDistortionTool::~LensDistortionTool()
{
    delete d;
}

void LensDistortionTool::slotColorGuideChanged()
{
    d->previewWidget->slotChangeGuideColor(d->gboxSettings->guideColor());
    d->previewWidget->slotChangeGuideSize(d->gboxSettings->guideSize());
}

void LensDistortionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    d->mainInput->setValue(group.readEntry(d->config2ndOrderDistortionEntry, d->mainInput->defaultValue()));
    d->edgeInput->setValue(group.readEntry(d->config4thOrderDistortionEntry, d->edgeInput->defaultValue()));
    d->rescaleInput->setValue(group.readEntry(d->configZoomFactorEntry,      d->rescaleInput->defaultValue()));
    d->brightenInput->setValue(group.readEntry(d->configBrightenEntry,       d->brightenInput->defaultValue()));

    blockWidgetSignals(false);

    slotColorGuideChanged();
    slotPreview();
}

void LensDistortionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->config2ndOrderDistortionEntry, d->mainInput->value());
    group.writeEntry(d->config4thOrderDistortionEntry, d->edgeInput->value());
    group.writeEntry(d->configZoomFactorEntry,         d->rescaleInput->value());
    group.writeEntry(d->configBrightenEntry,           d->brightenInput->value());

    config->sync();
}

void LensDistortionTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->mainInput->slotReset();
    d->edgeInput->slotReset();
    d->rescaleInput->slotReset();
    d->brightenInput->slotReset();

    blockWidgetSignals(false);

    slotPreview();
}

void LensDistortionTool::preparePreview()
{
    double m = d->mainInput->value();
    double e = d->edgeInput->value();
    double r = d->rescaleInput->value();
    double b = d->brightenInput->value();

    LensDistortionFilter transformPreview(&d->previewRasterImage, 0, m, e, r, b, 0, 0);
    transformPreview.startFilterDirectly();
    d->maskPreviewLabel->setPixmap(transformPreview.getTargetImage().convertToPixmap());

    ImageIface* const iface = d->previewWidget->imageIface();

    setFilter(new LensDistortionFilter(iface->original(), this, m, e, r, b, 0, 0));
}

void LensDistortionTool::prepareFinal()
{
    double m = d->mainInput->value();
    double e = d->edgeInput->value();
    double r = d->rescaleInput->value();
    double b = d->brightenInput->value();

    ImageIface iface;
    setFilter(new LensDistortionFilter(iface.original(), this, m, e, r, b, 0, 0));
}

void LensDistortionTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg imDest             = filter()->getTargetImage().smoothScale(iface->previewSize());
    iface->setPreview(imDest);

    d->previewWidget->updatePreview();
}

void LensDistortionTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Lens Distortion"), filter()->filterAction(), filter()->getTargetImage());
}

void LensDistortionTool::blockWidgetSignals(bool b)
{
    d->mainInput->blockSignals(b);
    d->edgeInput->blockSignals(b);
    d->rescaleInput->blockSignals(b);
    d->brightenInput->blockSignals(b);
}

}  // namespace DigikamEnhanceImagePlugin
