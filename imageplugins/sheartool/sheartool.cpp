/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-23
 * Description : a plugin to shear an image
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qcheckbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "shear.h"
#include "sheartool.h"
#include "sheartool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamShearToolImagesPlugin
{

ShearTool::ShearTool(QObject* parent)
         : EditorToolThreaded(parent)
{
    setName("sheartool");
    setToolName(i18n("Shear Tool"));
    setToolIcon(SmallIcon("sheartool"));

    m_previewWidget = new ImageWidget("sheartool Tool", 0,
                                      i18n("<p>This is the shear operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the shear correction. "
                                           "Release the left mouse button to freeze the dashed "
                                           "line's position."),
                                      false, ImageGuideWidget::HVGuideMode);

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::ColorGuide);
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage(), 12, 2);

    QLabel *label1 = new QLabel(i18n("New width:"), m_gboxSettings->plainPage());
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newWidthLabel->setAlignment( AlignBottom | AlignRight );

    QLabel *label2 = new QLabel(i18n("New height:"), m_gboxSettings->plainPage());
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newHeightLabel->setAlignment( AlignBottom | AlignRight );

    KSeparator *line = new KSeparator (Horizontal, m_gboxSettings->plainPage());

    QLabel *label3    = new QLabel(i18n("Main horizontal angle:"), m_gboxSettings->plainPage());
    m_mainHAngleInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_mainHAngleInput->setRange(-45, 45, 1);
    m_mainHAngleInput->setDefaultValue(0);
    QWhatsThis::add( m_mainHAngleInput, i18n("<p>The main horizontal shearing angle, in degrees."));

    QLabel *label4    = new QLabel(i18n("Fine horizontal angle:"), m_gboxSettings->plainPage());
    m_fineHAngleInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_fineHAngleInput->setRange(-5.0, 5.0, 0.01);
    m_fineHAngleInput->setDefaultValue(0);
    QWhatsThis::add( m_fineHAngleInput, i18n("<p>This value in degrees will be added to main horizontal angle value "
                                             "to set fine adjustments."));

    QLabel *label5    = new QLabel(i18n("Main vertical angle:"), m_gboxSettings->plainPage());
    m_mainVAngleInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_mainVAngleInput->setRange(-45, 45, 1);
    m_mainVAngleInput->setDefaultValue(0);
    QWhatsThis::add( m_mainVAngleInput, i18n("<p>The main vertical shearing angle, in degrees."));

    QLabel *label6    = new QLabel(i18n("Fine vertical angle:"), m_gboxSettings->plainPage());
    m_fineVAngleInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_fineVAngleInput->setRange(-5.0, 5.0, 0.01);
    m_fineVAngleInput->setDefaultValue(0);
    QWhatsThis::add( m_fineVAngleInput, i18n("<p>This value in degrees will be added to main vertical angle value "
                                             "to set fine adjustments."));

    m_antialiasInput = new QCheckBox(i18n("Anti-Aliasing"), m_gboxSettings->plainPage());
    QWhatsThis::add( m_antialiasInput, i18n("<p>Enable this option to apply the anti-aliasing filter "
                                            "to the sheared image. "
                                            "To smooth the target image, it will be blurred a little."));

    grid->addMultiCellWidget(label1,            0, 0, 0, 0);
    grid->addMultiCellWidget(m_newWidthLabel,   0, 0, 1, 2);
    grid->addMultiCellWidget(label2,            1, 1, 0, 0);
    grid->addMultiCellWidget(m_newHeightLabel,  1, 1, 1, 2);
    grid->addMultiCellWidget(line,              2, 2, 0, 2);
    grid->addMultiCellWidget(label3,            3, 3, 0, 2);
    grid->addMultiCellWidget(m_mainHAngleInput, 4, 4, 0, 2);
    grid->addMultiCellWidget(label4,            5, 5, 0, 2);
    grid->addMultiCellWidget(m_fineHAngleInput, 6, 6, 0, 2);
    grid->addMultiCellWidget(label5,            7, 7, 0, 0);
    grid->addMultiCellWidget(m_mainVAngleInput, 8, 8, 0, 2);
    grid->addMultiCellWidget(label6,            9, 9, 0, 2);
    grid->addMultiCellWidget(m_fineVAngleInput, 10, 10, 0, 2);
    grid->addMultiCellWidget(m_antialiasInput,  11, 11, 0, 2);
    grid->setRowStretch(12, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_mainHAngleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_fineHAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_mainVAngleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_fineVAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_antialiasInput, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    connect(m_gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));
}

ShearTool::~ShearTool()
{
}

void ShearTool::slotColorGuideChanged()
{
    m_previewWidget->slotChangeGuideColor(m_gboxSettings->guideColor());
    m_previewWidget->slotChangeGuideSize(m_gboxSettings->guideSize());
}

void ShearTool::readSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("sheartool Tool");
    m_mainHAngleInput->setValue(config->readNumEntry("Main HAngle", m_mainHAngleInput->defaultValue()));
    m_mainVAngleInput->setValue(config->readNumEntry("Main VAngle", m_mainVAngleInput->defaultValue()));
    m_fineHAngleInput->setValue(config->readDoubleNumEntry("Fine HAngle", m_fineHAngleInput->defaultValue()));
    m_fineVAngleInput->setValue(config->readDoubleNumEntry("Fine VAngle", m_fineVAngleInput->defaultValue()));
    m_antialiasInput->setChecked(config->readBoolEntry("Anti Aliasing", true));
    m_gboxSettings->setGuideColor(config->readColorEntry("Guide Color", &Qt::red));
    m_gboxSettings->setGuideSize(config->readNumEntry("Guide Width", 1));

    slotColorGuideChanged();
    slotEffect();
}

void ShearTool::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("sheartool Tool");
    config->writeEntry("Main HAngle", m_mainHAngleInput->value());
    config->writeEntry("Main VAngle", m_mainVAngleInput->value());
    config->writeEntry("Fine HAngle", m_fineHAngleInput->value());
    config->writeEntry("Fine VAngle", m_fineVAngleInput->value());
    config->writeEntry("Anti Aliasing", m_antialiasInput->isChecked());
    config->writeEntry("Guide Color", m_gboxSettings->guideColor());
    config->writeEntry("Guide Width", m_gboxSettings->guideSize());
    m_previewWidget->writeSettings();
    config->sync();
}

void ShearTool::slotResetSettings()
{
    m_mainHAngleInput->blockSignals(true);
    m_mainVAngleInput->blockSignals(true);
    m_fineHAngleInput->blockSignals(true);
    m_fineVAngleInput->blockSignals(true);
    m_antialiasInput->blockSignals(true);

    m_mainHAngleInput->slotReset();
    m_mainVAngleInput->slotReset();
    m_fineHAngleInput->slotReset();
    m_fineVAngleInput->slotReset();
    m_antialiasInput->setChecked(true);

    m_mainHAngleInput->blockSignals(false);
    m_mainVAngleInput->blockSignals(false);
    m_fineHAngleInput->blockSignals(false);
    m_fineVAngleInput->blockSignals(false);
    m_antialiasInput->blockSignals(false);
}

void ShearTool::prepareEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_mainHAngleInput->setEnabled(false);
    m_mainVAngleInput->setEnabled(false);
    m_fineHAngleInput->setEnabled(false);
    m_fineVAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);

    float hAngle      = m_mainHAngleInput->value() + m_fineHAngleInput->value();
    float vAngle      = m_mainVAngleInput->value() + m_fineVAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    QColor background = m_previewWidget->paletteBackgroundColor().rgb();
    ImageIface* iface = m_previewWidget->imageIface();
    int orgW          = iface->originalWidth();
    int orgH          = iface->originalHeight();
    uchar *data       = iface->getPreviewImage();
    DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
               iface->previewHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Shear(&image, this, hAngle, vAngle, antialiasing,
                                                background, orgW, orgH)));
}

void ShearTool::prepareFinal()
{
    m_mainHAngleInput->setEnabled(false);
    m_mainVAngleInput->setEnabled(false);
    m_fineHAngleInput->setEnabled(false);
    m_fineVAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);

    float hAngle      = m_mainHAngleInput->value() + m_fineHAngleInput->value();
    float vAngle      = m_mainVAngleInput->value() + m_fineVAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    QColor background = Qt::black;

    ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();

    uchar *data = iface.getOriginalImage();
    DImg orgImage(orgW, orgH, iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Shear(&orgImage, this, hAngle, vAngle, antialiasing,
                                                          background, orgW, orgH)));
}

void ShearTool::putPreviewData()
{
    ImageIface* iface = m_previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    DImg imTemp       = filter()->getTargetImage().smoothScale(w, h, QSize::ScaleMin);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(),
                       filter()->getTargetImage().hasAlpha() );

    imDest.fill(DColor(m_previewWidget->paletteBackgroundColor().rgb(),
                       filter()->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());

    m_previewWidget->updatePreview();
    QSize newSize = dynamic_cast<Shear*>(filter())->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void ShearTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg targetImage = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Shear Tool"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

void ShearTool::renderingFinished()
{
    m_mainHAngleInput->setEnabled(true);
    m_mainVAngleInput->setEnabled(true);
    m_fineHAngleInput->setEnabled(true);
    m_fineVAngleInput->setEnabled(true);
    m_antialiasInput->setEnabled(true);
    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamShearToolImagesPlugin
