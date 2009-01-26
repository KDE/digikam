/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-27
 * Description : a plugin to reduce lens distorsions to an image.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// C++ includes.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qbrush.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qimage.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "editortoolsettings.h"
#include "lensdistortion.h"
#include "lensdistortiontool.h"
#include "lensdistortiontool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamLensDistortionImagesPlugin
{

LensDistortionTool::LensDistortionTool(QObject* parent)
                  : EditorToolThreaded(parent)
{
    setName("lensdistortion");
    setToolName(i18n("Lens Distortion"));
    setToolIcon(SmallIcon("lensdistortion"));

    m_previewWidget = new ImageWidget("lensdistortion Tool", 0, QString(),
                                      false, ImageGuideWidget::HVGuideMode);

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::ColorGuide);
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage(), 9, 1);

    m_maskPreviewLabel = new QLabel( m_gboxSettings->plainPage() );
    m_maskPreviewLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
    QWhatsThis::add( m_maskPreviewLabel, i18n("<p>You can see here a thumbnail preview of the distortion correction "
                                              "applied to a cross pattern.") );

    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Main:"), m_gboxSettings->plainPage());

    m_mainInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_mainInput->setPrecision(1);
    m_mainInput->setRange(-100.0, 100.0, 0.1);
    m_mainInput->setDefaultValue(0.0);
    QWhatsThis::add(m_mainInput, i18n("<p>This value controls the amount of distortion. Negative values correct lens barrel "
                                      "distortion, while positive values correct lens pincushion distortion."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Edge:"), m_gboxSettings->plainPage());

    m_edgeInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_edgeInput->setPrecision(1);
    m_edgeInput->setRange(-100.0, 100.0, 0.1);
    m_edgeInput->setDefaultValue(0.0);
    QWhatsThis::add(m_edgeInput, i18n("<p>This value controls in the same manner as the Main control, but has more effect "
                                      "at the edges of the image than at the center."));

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Zoom:"), m_gboxSettings->plainPage());

    m_rescaleInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_rescaleInput->setPrecision(1);
    m_rescaleInput->setRange(-100.0, 100.0, 0.1);
    m_rescaleInput->setDefaultValue(0.0);
    QWhatsThis::add(m_rescaleInput, i18n("<p>This value rescales the overall image size."));

    // -------------------------------------------------------------

    QLabel *label4 = new QLabel(i18n("Brighten:"), m_gboxSettings->plainPage());

    m_brightenInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_brightenInput->setPrecision(1);
    m_brightenInput->setRange(-100.0, 100.0, 0.1);
    m_brightenInput->setDefaultValue(0.0);
    QWhatsThis::add(m_brightenInput, i18n("<p>This value adjusts the brightness in image corners."));

    grid->addMultiCellWidget(m_maskPreviewLabel, 0, 0, 0, 1);
    grid->addMultiCellWidget(label1,             1, 1, 0, 1);
    grid->addMultiCellWidget(m_mainInput,        2, 2, 0, 1);
    grid->addMultiCellWidget(label2,             3, 3, 0, 1);
    grid->addMultiCellWidget(m_edgeInput,        4, 4, 0, 1);
    grid->addMultiCellWidget(label3,             5, 5, 0, 1);
    grid->addMultiCellWidget(m_rescaleInput,     6, 6, 0, 1);
    grid->addMultiCellWidget(label4,             7, 7, 0, 1);
    grid->addMultiCellWidget(m_brightenInput,    8, 8, 0, 1);
    grid->setRowStretch(9, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_mainInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_edgeInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_rescaleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_brightenInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_gboxSettings, SIGNAL(signalColorGuideChanged()),
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

    QImage preview(120, 120, 32);
    memset(preview.bits(), 255, preview.numBytes());
    QPixmap pix (preview);
    QPainter pt(&pix);
    pt.setPen( QPen(Qt::black, 1) );
    pt.fillRect( 0, 0, pix.width(), pix.height(), QBrush(Qt::black, Qt::CrossPattern) );
    pt.drawRect( 0, 0, pix.width(), pix.height() );
    pt.end();
    QImage preview2(pix.convertToImage());
    m_previewRasterImage = DImg(preview2.width(), preview2.height(), false, false, preview2.bits());
}

LensDistortionTool::~LensDistortionTool()
{
}

void LensDistortionTool::slotColorGuideChanged()
{
    m_previewWidget->slotChangeGuideColor(m_gboxSettings->guideColor());
    m_previewWidget->slotChangeGuideSize(m_gboxSettings->guideSize());
}

void LensDistortionTool::readSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("lensdistortion Tool");

    m_mainInput->blockSignals(true);
    m_edgeInput->blockSignals(true);
    m_rescaleInput->blockSignals(true);
    m_brightenInput->blockSignals(true);

    m_mainInput->setValue(config->readDoubleNumEntry("2nd Order Distortion", m_mainInput->defaultValue()));
    m_edgeInput->setValue(config->readDoubleNumEntry("4th Order Distortion",m_edgeInput->defaultValue()));
    m_rescaleInput->setValue(config->readDoubleNumEntry("Zoom Factor", m_rescaleInput->defaultValue()));
    m_brightenInput->setValue(config->readDoubleNumEntry("Brighten", m_brightenInput->defaultValue()));
    m_gboxSettings->setGuideColor(config->readColorEntry("Guide Color", &Qt::red));
    m_gboxSettings->setGuideSize(config->readNumEntry("Guide Width", 1));

    m_mainInput->blockSignals(false);
    m_edgeInput->blockSignals(false);
    m_rescaleInput->blockSignals(false);
    m_brightenInput->blockSignals(false);

    slotColorGuideChanged();
    slotEffect();
}

void LensDistortionTool::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("lensdistortion Tool");
    config->writeEntry("2nd Order Distortion", m_mainInput->value());
    config->writeEntry("4th Order Distortion", m_edgeInput->value());
    config->writeEntry("Zoom Factor", m_rescaleInput->value());
    config->writeEntry("Brighten", m_brightenInput->value());
    config->writeEntry("Guide Color", m_gboxSettings->guideColor());
    config->writeEntry("Guide Width", m_gboxSettings->guideSize());
    m_previewWidget->writeSettings();
    config->sync();
}

void LensDistortionTool::slotResetSettings()
{
    m_mainInput->blockSignals(true);
    m_edgeInput->blockSignals(true);
    m_rescaleInput->blockSignals(true);
    m_brightenInput->blockSignals(true);

    m_mainInput->slotReset();
    m_edgeInput->slotReset();
    m_rescaleInput->slotReset();
    m_brightenInput->slotReset();

    m_mainInput->blockSignals(false);
    m_edgeInput->blockSignals(false);
    m_rescaleInput->blockSignals(false);
    m_brightenInput->blockSignals(false);
}

void LensDistortionTool::prepareEffect()
{
    m_mainInput->setEnabled(false);
    m_edgeInput->setEnabled(false);
    m_rescaleInput->setEnabled(false);
    m_brightenInput->setEnabled(false);

    double m = m_mainInput->value();
    double e = m_edgeInput->value();
    double r = m_rescaleInput->value();
    double b = m_brightenInput->value();

    LensDistortion transformPreview(&m_previewRasterImage, 0L, m, e, r, b, 0, 0);
    m_maskPreviewLabel->setPixmap(QPixmap(transformPreview.getTargetImage().convertToPixmap()));

    ImageIface* iface = m_previewWidget->imageIface();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new LensDistortion(iface->getOriginalImg(), this, m, e, r, b, 0, 0)));
}

void LensDistortionTool::prepareFinal()
{
    m_mainInput->setEnabled(false);
    m_edgeInput->setEnabled(false);
    m_rescaleInput->setEnabled(false);
    m_brightenInput->setEnabled(false);

    double m = m_mainInput->value();
    double e = m_edgeInput->value();
    double r = m_rescaleInput->value();
    double b = m_brightenInput->value();

    ImageIface iface(0, 0);

    setFilter(dynamic_cast<DImgThreadedFilter*>(new LensDistortion(iface.getOriginalImg(), this, m, e, r, b, 0, 0)));
}

void LensDistortionTool::putPreviewData()
{
    ImageIface* iface = m_previewWidget->imageIface();

    DImg imDest = filter()->getTargetImage().smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    m_previewWidget->updatePreview();
}

void LensDistortionTool::putFinalData()
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Lens Distortion"), filter()->getTargetImage().bits());
}

void LensDistortionTool::renderingFinished()
{
    m_mainInput->setEnabled(true);
    m_edgeInput->setEnabled(true);
    m_rescaleInput->setEnabled(true);
    m_brightenInput->setEnabled(true);
}

}  // NameSpace DigikamLensDistortionImagesPlugin
