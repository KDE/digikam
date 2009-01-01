/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-28
 * Description : a digiKam image editor plugin to process image
 *               free rotation.
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "freerotationtool.h"
#include "freerotationtool.moc"

// Qt includes.

#include <QCheckBox>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLayout>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "editortoolsettings.h"
#include "freerotation.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamFreeRotationImagesPlugin
{

FreeRotationTool::FreeRotationTool(QObject* parent)
                        : EditorToolThreaded(parent)
{
    setObjectName("freerotation");
    setToolName(i18n("Free Rotation"));
    setToolIcon(SmallIcon("freerotation"));

    m_previewWidget = new ImageWidget("freerotation Tool", 0,
                                      i18n("This is the free rotation operation preview. "
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

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());

    QLabel *label1  = new QLabel(i18n("New width:"), m_gboxSettings->plainPage());
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newWidthLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel *label2   = new QLabel(i18n("New height:"), m_gboxSettings->plainPage());
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newHeightLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    KSeparator *line = new KSeparator(Qt::Horizontal, m_gboxSettings->plainPage());

    QLabel *label3 = new QLabel(i18n("Main angle:"), m_gboxSettings->plainPage());
    m_angleInput   = new RIntNumInput(m_gboxSettings->plainPage());
    m_angleInput->setRange(-180, 180, 1);
    m_angleInput->setSliderEnabled(true);
    m_angleInput->setDefaultValue(0);
    m_angleInput->setWhatsThis( i18n("An angle in degrees by which to rotate the image. "
                                     "A positive angle rotates the image clockwise; "
                                     "a negative angle rotates it counter-clockwise."));

    QLabel *label4   = new QLabel(i18n("Fine angle:"), m_gboxSettings->plainPage());
    m_fineAngleInput = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_fineAngleInput->input()->setRange(-5.0, 5.0, 0.01, true);
    m_fineAngleInput->setDefaultValue(0);
    m_fineAngleInput->setWhatsThis( i18n("This value in degrees will be added to main angle value "
                                         "to set fine target angle."));

    m_antialiasInput = new QCheckBox(i18n("Anti-Aliasing"), m_gboxSettings->plainPage());
    m_antialiasInput->setWhatsThis( i18n("Enable this option to apply the anti-aliasing filter "
                                         "to the rotated image. "
                                         "In order to smooth the target image, it will be blurred a little."));

    QLabel *label5 = new QLabel(i18n("Auto-crop:"), m_gboxSettings->plainPage());
    m_autoCropCB   = new RComboBox(m_gboxSettings->plainPage());
    m_autoCropCB->addItem( i18nc("no autocrop", "None") );
    m_autoCropCB->addItem( i18n("Widest Area") );
    m_autoCropCB->addItem( i18n("Largest Area") );
    m_autoCropCB->setDefaultIndex(FreeRotation::NoAutoCrop);
    m_autoCropCB->setWhatsThis( i18n("Select the method to process image auto-cropping "
                                     "to remove black frames around a rotated image here."));

    // -------------------------------------------------------------

    gridSettings->addWidget(label1,             0, 0, 1, 1);
    gridSettings->addWidget(m_newWidthLabel,    0, 1, 1, 2);
    gridSettings->addWidget(label2,             1, 0, 1, 1);
    gridSettings->addWidget(m_newHeightLabel,   1, 1, 1, 2);
    gridSettings->addWidget(line,               2, 0, 1, 3);
    gridSettings->addWidget(label3,             3, 0, 1, 3);
    gridSettings->addWidget(m_angleInput,       4, 0, 1, 3);
    gridSettings->addWidget(label4,             5, 0, 1, 3);
    gridSettings->addWidget(m_fineAngleInput,   6, 0, 1, 3);
    gridSettings->addWidget(m_antialiasInput,   7, 0, 1, 3);
    gridSettings->addWidget(label5,             8, 0, 1, 1);
    gridSettings->addWidget(m_autoCropCB,       8, 1, 1, 2);
    gridSettings->setRowStretch(9, 10);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(0);

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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("freerotation Tool");
    m_angleInput->setValue(group.readEntry("Main Angle", m_angleInput->defaultValue()));
    m_fineAngleInput->setValue(group.readEntry("Fine Angle", m_fineAngleInput->defaultValue()));
    m_autoCropCB->setCurrentIndex(group.readEntry("Auto Crop Type", m_autoCropCB->defaultIndex()));
    m_antialiasInput->setChecked(group.readEntry("Anti Aliasing", true));

    slotColorGuideChanged();
    slotEffect();
}

void FreeRotationTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("freerotation Tool");
    group.writeEntry("Main Angle", m_angleInput->value());
    group.writeEntry("Fine Angle", m_fineAngleInput->value());
    group.writeEntry( "Auto Crop Type", m_autoCropCB->currentIndex() );
    group.writeEntry( "Anti Aliasing", m_antialiasInput->isChecked() );
    group.sync();
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

    slotEffect();
}

void FreeRotationTool::prepareEffect()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    m_angleInput->setEnabled(false);
    m_fineAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);
    m_autoCropCB->setEnabled(false);

    double angle = m_angleInput->value() + m_fineAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    int autocrop = m_autoCropCB->currentIndex();
    QColor background = toolView()->backgroundRole();

    ImageIface* iface = m_previewWidget->imageIface();
    int orgW = iface->originalWidth();
    int orgH = iface->originalHeight();

    uchar *data = iface->getPreviewImage();
    DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
                        iface->previewHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new FreeRotation(&image, this, angle, antialiasing, autocrop,
                                        background, orgW, orgH)));
}

void FreeRotationTool::prepareFinal()
{
    m_angleInput->setEnabled(false);
    m_fineAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);
    m_autoCropCB->setEnabled(false);

    double angle      = m_angleInput->value() + m_fineAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    int autocrop      = m_autoCropCB->currentIndex();
    QColor background = Qt::black;

    ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();

    uchar *data = iface.getOriginalImage();
    DImg orgImage(orgW, orgH, iface.originalSixteenBit(),
                           iface.originalHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new FreeRotation(&orgImage, this, angle, antialiasing, autocrop,
                                        background, orgW, orgH)));
}

void FreeRotationTool::putPreviewData(void)
{
    ImageIface* iface = m_previewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();

    DImg imTemp = filter()->getTargetImage().smoothScale(w, h, Qt::ScaleMin);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(),
                                filter()->getTargetImage().hasAlpha() );

    QColor background = toolView()->backgroundRole();
    imDest.fill( DColor(background, filter()->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());

    m_previewWidget->updatePreview();
    QSize newSize = dynamic_cast<FreeRotation *>(filter())->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void FreeRotationTool::putFinalData(void)
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

}  // namespace DigikamFreeRotationImagesPlugin
