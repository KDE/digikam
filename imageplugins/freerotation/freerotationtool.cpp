/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-28
 * Description : a digiKam image editor plugin to process image
 *               free rotation.
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
#include <libkdcraw/rcombobox.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "editortoolsettings.h"
#include "freerotation.h"
#include "freerotationtool.h"
#include "freerotationtool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamFreeRotationImagesPlugin
{

FreeRotationTool::FreeRotationTool(QObject* parent)
                : EditorToolThreaded(parent)
{
    setName("freerotation");
    setToolName(i18n("Free Rotation"));
    setToolIcon(SmallIcon("freerotation"));

    m_previewWidget = new ImageWidget("freerotation Tool", 0,
                                      i18n("<p>This is the free rotation operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the free rotation correction. "
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
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage(), 9, 2);

    QLabel *label1  = new QLabel(i18n("New width:"), m_gboxSettings->plainPage());
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newWidthLabel->setAlignment( AlignBottom | AlignRight );

    QLabel *label2   = new QLabel(i18n("New height:"), m_gboxSettings->plainPage());
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newHeightLabel->setAlignment( AlignBottom | AlignRight );

    KSeparator *line = new KSeparator (Horizontal, m_gboxSettings->plainPage());

    QLabel *label3 = new QLabel(i18n("Main angle:"), m_gboxSettings->plainPage());
    m_angleInput   = new RIntNumInput(m_gboxSettings->plainPage());
    m_angleInput->setRange(-180, 180, 1);
    m_angleInput->setDefaultValue(0);
    QWhatsThis::add( m_angleInput, i18n("<p>An angle in degrees by which to rotate the image. "
                                        "A positive angle rotates the image clockwise; "
                                        "a negative angle rotates it counter-clockwise."));

    QLabel *label4   = new QLabel(i18n("Fine angle:"), m_gboxSettings->plainPage());
    m_fineAngleInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_fineAngleInput->setRange(-5.0, 5.0, 0.01);
    m_fineAngleInput->setDefaultValue(0);
    QWhatsThis::add( m_fineAngleInput, i18n("<p>This value in degrees will be added to main angle value "
                                            "to set fine target angle."));

    m_antialiasInput = new QCheckBox(i18n("Anti-Aliasing"), m_gboxSettings->plainPage());
    QWhatsThis::add( m_antialiasInput, i18n("<p>Enable this option to apply the anti-aliasing filter "
                                            "to the rotated image. "
                                            "In order to smooth the target image, it will be blurred a little."));

    QLabel *label5 = new QLabel(i18n("Auto-crop:"), m_gboxSettings->plainPage());
    m_autoCropCB   = new RComboBox(m_gboxSettings->plainPage());
    m_autoCropCB->insertItem( i18n("None") );
    m_autoCropCB->insertItem( i18n("Widest Area") );
    m_autoCropCB->insertItem( i18n("Largest Area") );
    m_autoCropCB->setDefaultItem(FreeRotation::NoAutoCrop);
    QWhatsThis::add( m_autoCropCB, i18n("<p>Select the method to process image auto-cropping "
                                        "to remove black frames around a rotated image."));

    grid->addMultiCellWidget(label1,           0, 0, 0, 0);
    grid->addMultiCellWidget(m_newWidthLabel,  0, 0, 1, 2);
    grid->addMultiCellWidget(label2,           1, 1, 0, 0);
    grid->addMultiCellWidget(m_newHeightLabel, 1, 1, 1, 2);
    grid->addMultiCellWidget(line,             2, 2, 0, 2);
    grid->addMultiCellWidget(label3,           3, 3, 0, 2);
    grid->addMultiCellWidget(m_angleInput,     4, 4, 0, 2);
    grid->addMultiCellWidget(label4,           5, 5, 0, 2);
    grid->addMultiCellWidget(m_fineAngleInput, 6, 6, 0, 2);
    grid->addMultiCellWidget(m_antialiasInput, 7, 7, 0, 2);
    grid->addMultiCellWidget(label5,           8, 8, 0, 0);
    grid->addMultiCellWidget(m_autoCropCB,     8, 8, 1, 2);
    grid->setRowStretch(9, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_angleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_fineAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_antialiasInput, SIGNAL(toggled(bool)),
            this, SLOT(slotEffect()));

    connect(m_autoCropCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(m_gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));
}

FreeRotationTool::~FreeRotationTool()
{
}

void FreeRotationTool::slotColorGuideChanged()
{
    m_previewWidget->slotChangeGuideColor(m_gboxSettings->guideColor());
    m_previewWidget->slotChangeGuideSize(m_gboxSettings->guideSize());
}

void FreeRotationTool::readSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("freerotation Tool");
    m_angleInput->setValue(config->readNumEntry("Main Angle", m_angleInput->defaultValue()));
    m_fineAngleInput->setValue(config->readDoubleNumEntry("Fine Angle", m_fineAngleInput->defaultValue()));
    m_autoCropCB->setCurrentItem(config->readNumEntry("Auto Crop Type", m_autoCropCB->defaultItem()));
    m_antialiasInput->setChecked(config->readBoolEntry("Anti Aliasing", true));
    m_gboxSettings->setGuideColor(config->readColorEntry("Guide Color", &Qt::red));
    m_gboxSettings->setGuideSize(config->readNumEntry("Guide Width", 1));

    slotColorGuideChanged();
    slotEffect();
}

void FreeRotationTool::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("freerotation Tool");
    config->writeEntry("Main Angle", m_angleInput->value());
    config->writeEntry("Fine Angle", m_fineAngleInput->value());
    config->writeEntry("Auto Crop Type", m_autoCropCB->currentItem());
    config->writeEntry("Anti Aliasing", m_antialiasInput->isChecked());
    config->writeEntry("Guide Color", m_gboxSettings->guideColor());
    config->writeEntry("Guide Width", m_gboxSettings->guideSize());
    m_previewWidget->writeSettings();
    config->sync();
}

void FreeRotationTool::slotResetSettings()
{
    m_angleInput->blockSignals(true);
    m_antialiasInput->blockSignals(true);
    m_autoCropCB->blockSignals(true);

    m_angleInput->slotReset();
    m_fineAngleInput->slotReset();
    m_antialiasInput->setChecked(true);
    m_autoCropCB->slotReset();

    m_angleInput->blockSignals(false);
    m_antialiasInput->blockSignals(false);
    m_autoCropCB->blockSignals(false);
}

void FreeRotationTool::prepareEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_angleInput->setEnabled(false);
    m_fineAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);
    m_autoCropCB->setEnabled(false);

    double angle      = m_angleInput->value() + m_fineAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    int autocrop      = m_autoCropCB->currentItem();
    QColor background = toolView()->paletteBackgroundColor().rgb();
    ImageIface* iface = m_previewWidget->imageIface();
    int orgW          = iface->originalWidth();
    int orgH          = iface->originalHeight();

    uchar *data = iface->getPreviewImage();
    DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
               iface->previewHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter*>(new FreeRotation(&image, this, angle, antialiasing,
                                                autocrop, background, orgW, orgH)));
}

void FreeRotationTool::prepareFinal()
{
    m_angleInput->setEnabled(false);
    m_fineAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);
    m_autoCropCB->setEnabled(false);

    double angle      = m_angleInput->value() + m_fineAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    int autocrop      = m_autoCropCB->currentItem();
    QColor background = Qt::black;

    ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();

    uchar *data = iface.getOriginalImage();
    DImg orgImage(orgW, orgH, iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter *>(new FreeRotation(&orgImage, this, angle, antialiasing,
                                                 autocrop, background, orgW, orgH)));
}

void FreeRotationTool::putPreviewData()
{
    ImageIface* iface = m_previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();

    DImg imTemp = filter()->getTargetImage().smoothScale(w, h, QSize::ScaleMin);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(),
                       filter()->getTargetImage().hasAlpha() );

    imDest.fill(DColor(toolView()->paletteBackgroundColor().rgb(),
                       filter()->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());

    m_previewWidget->updatePreview();
    QSize newSize = dynamic_cast<FreeRotation*>(filter())->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void FreeRotationTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg targetImage = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Free Rotation"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

void FreeRotationTool::renderingFinished()
{
    m_angleInput->setEnabled(true);
    m_fineAngleInput->setEnabled(true);
    m_antialiasInput->setEnabled(true);
    m_autoCropCB->setEnabled(true);
    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamFreeRotationImagesPlugin
