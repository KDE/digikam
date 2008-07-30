/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-23
 * Description : a plugin to shear an image
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

// Qt includes.

#include <QLabel>
#include <QCheckBox>
#include <QImage>
#include <QGridLayout>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kcursor.h>
#include <kconfig.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "sheartool.h"
#include "imageeffect_sheartool.h"
#include "imageeffect_sheartool.moc"

namespace DigikamShearToolImagesPlugin
{

ImageEffect_ShearTool::ImageEffect_ShearTool(QWidget* parent)
                     : Digikam::ImageGuideDlg(parent, i18n("Shear Tool"), "sheartool",
                                              false, true, true,
                                              Digikam::ImageGuideWidget::HVGuideMode)
{
    // No need Abort button action.
    showButton(User1, false);

    QString whatsThis;

    // About data and help button.

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Shear Tool"),
                                       digiKamVersion().toAscii(),
                                       ki18n("A digiKam image plugin to shear an image."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2008, Gilles Caulier"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Pieter Z. Voloshyn"), ki18n("Shear algorithm"),
                     "pieter dot voloshyn at gmail dot com");

    setAboutData(about);

    m_imagePreviewWidget->setWhatsThis( i18n("<p>This is the shearing image operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the shearing correction. "
                                           "Release the left mouse button to freeze the dashed "
                                           "line's position."));

    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout* gridSettings = new QGridLayout(gboxSettings);

    QLabel *label1  = new QLabel(i18n("New width:"), gboxSettings);
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), gboxSettings);
    m_newWidthLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel *label2   = new QLabel(i18n("New height:"), gboxSettings);
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), gboxSettings);
    m_newHeightLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    KSeparator *line = new KSeparator (Qt::Horizontal, gboxSettings);

    QLabel *label3    = new QLabel(i18n("Main horizontal angle:"), gboxSettings);
    m_mainHAngleInput = new KIntNumInput(gboxSettings);
    m_mainHAngleInput->setRange(-45, 45, 1);
    m_mainHAngleInput->setSliderEnabled(true);
    m_mainHAngleInput->setValue(0);
    m_mainHAngleInput->setWhatsThis( i18n("<p>The main horizontal shearing angle, in degrees."));

    QLabel *label4 = new QLabel(i18n("Fine horizontal angle:"), gboxSettings);
    m_fineHAngleInput = new KDoubleNumInput(gboxSettings);
    m_fineHAngleInput->setRange(-5.0, 5.0, 0.01, true);
    m_fineHAngleInput->setValue(0);
    m_fineHAngleInput->setWhatsThis( i18n("<p>This value in degrees will be added to main "
                                          "horizontal angle value to set fine adjustments."));
    QLabel *label5 = new QLabel(i18n("Main vertical angle:"), gboxSettings);
    m_mainVAngleInput = new KIntNumInput(gboxSettings);
    m_mainVAngleInput->setRange(-45, 45, 1);
    m_mainVAngleInput->setSliderEnabled(true);
    m_mainVAngleInput->setValue(0);
    m_mainVAngleInput->setWhatsThis( i18n("<p>The main vertical shearing angle, in degrees."));

    QLabel *label6 = new QLabel(i18n("Fine vertical angle:"), gboxSettings);
    m_fineVAngleInput = new KDoubleNumInput(gboxSettings);
    m_fineVAngleInput->setRange(-5.0, 5.0, 0.01, true);
    m_fineVAngleInput->setValue(0);
    m_fineVAngleInput->setWhatsThis( i18n("<p>This value in degrees will be added to main vertical "
                                          "angle value to set fine adjustments."));

    m_antialiasInput = new QCheckBox(i18n("Anti-Aliasing"), gboxSettings);
    m_antialiasInput->setWhatsThis( i18n("<p>Enable this option to apply the anti-aliasing filter "
                                         "to the sheared image. "
                                         "To smooth the target image, it will be blurred a little."));

    // -------------------------------------------------------------

    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(0);
    gridSettings->addWidget(label1, 0, 0, 1, 1);
    gridSettings->addWidget(m_newWidthLabel, 0, 1, 1, 2);
    gridSettings->addWidget(label2, 1, 0, 1, 1);
    gridSettings->addWidget(m_newHeightLabel, 1, 1, 1, 2);
    gridSettings->addWidget(line, 2, 0, 1, 3 );
    gridSettings->addWidget(label3, 3, 0, 1, 3 );
    gridSettings->addWidget(m_mainHAngleInput, 4, 0, 1, 3 );
    gridSettings->addWidget(label4, 5, 0, 1, 3 );
    gridSettings->addWidget(m_fineHAngleInput, 6, 0, 1, 3 );
    gridSettings->addWidget(label5, 7, 0, 1, 1);
    gridSettings->addWidget(m_mainVAngleInput, 8, 0, 1, 3 );
    gridSettings->addWidget(label6, 9, 0, 1, 3 );
    gridSettings->addWidget(m_fineVAngleInput, 10, 0, 1, 3 );
    gridSettings->addWidget(m_antialiasInput, 11, 0, 1, 3 );

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_mainHAngleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_fineHAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_mainVAngleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_fineVAngleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotTimer()));

    connect(m_antialiasInput, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));
}

ImageEffect_ShearTool::~ImageEffect_ShearTool()
{
}

void ImageEffect_ShearTool::readUserSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("sheartool Tool Dialog");
    m_mainHAngleInput->setValue(group.readEntry("Main HAngle", 0));
    m_mainVAngleInput->setValue(group.readEntry("Main VAngle", 0));
    m_fineHAngleInput->setValue(group.readEntry("Fine HAngle", 0.0));
    m_fineVAngleInput->setValue(group.readEntry("Fine VAngle", 0.0));
    m_antialiasInput->setChecked(group.readEntry("Anti Aliasing", true));
    slotEffect();
}

void ImageEffect_ShearTool::writeUserSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("sheartool Tool Dialog");
    group.writeEntry("Main HAngle", m_mainHAngleInput->value());
    group.writeEntry("Main VAngle", m_mainVAngleInput->value());
    group.writeEntry("Fine HAngle", m_fineHAngleInput->value());
    group.writeEntry("Fine VAngle", m_fineVAngleInput->value());
    group.writeEntry("Anti Aliasing", m_antialiasInput->isChecked());
    config->sync();
}

void ImageEffect_ShearTool::resetValues()
{
    m_mainHAngleInput->blockSignals(true);
    m_mainVAngleInput->blockSignals(true);
    m_fineHAngleInput->blockSignals(true);
    m_fineVAngleInput->blockSignals(true);
    m_antialiasInput->blockSignals(true);
    m_mainHAngleInput->setValue(0);
    m_mainVAngleInput->setValue(0);
    m_fineHAngleInput->setValue(0.0);
    m_fineVAngleInput->setValue(0.0);
    m_antialiasInput->setChecked(true);
    m_mainHAngleInput->blockSignals(false);
    m_mainVAngleInput->blockSignals(false);
    m_fineHAngleInput->blockSignals(false);
    m_fineVAngleInput->blockSignals(false);
    m_antialiasInput->blockSignals(false);
}

void ImageEffect_ShearTool::prepareEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    m_mainHAngleInput->setEnabled(false);
    m_mainVAngleInput->setEnabled(false);
    m_fineHAngleInput->setEnabled(false);
    m_fineVAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);

    float hAngle      = m_mainHAngleInput->value() + m_fineHAngleInput->value();
    float vAngle      = m_mainVAngleInput->value() + m_fineVAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    QColor background = palette().color(QPalette::Background);

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int orgW = iface->originalWidth();
    int orgH = iface->originalHeight();

    uchar *data = iface->getPreviewImage();
    Digikam::DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
                        iface->previewHasAlpha(), data);
    delete [] data;

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new ShearTool(&image, this, hAngle, vAngle, antialiasing, background, orgW, orgH));
}

void ImageEffect_ShearTool::prepareFinal()
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

    Digikam::ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();

    uchar *data = iface.getOriginalImage();
    Digikam::DImg orgImage(orgW, orgH, iface.originalSixteenBit(),
                           iface.originalHasAlpha(), data);
    delete [] data;

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new ShearTool(&orgImage, this, hAngle, vAngle, antialiasing, background, orgW, orgH));
}

void ImageEffect_ShearTool::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();

    Digikam::DImg imTemp = m_threadedFilter->getTargetImage().smoothScale(w, h, Qt::ScaleMin);
    Digikam::DImg imDest( w, h, m_threadedFilter->getTargetImage().sixteenBit(),
                                m_threadedFilter->getTargetImage().hasAlpha() );

    imDest.fill( Digikam::DColor(palette().color(QPalette::Background),
                                 m_threadedFilter->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());

    m_imagePreviewWidget->updatePreview();
    QSize newSize = dynamic_cast<ShearTool *>(m_threadedFilter)->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void ImageEffect_ShearTool::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    Digikam::DImg targetImage = m_threadedFilter->getTargetImage();
    iface.putOriginalImage(i18n("Shear Tool"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

void ImageEffect_ShearTool::renderingFinished()
{
    m_mainHAngleInput->setEnabled(true);
    m_mainVAngleInput->setEnabled(true);
    m_fineHAngleInput->setEnabled(true);
    m_fineVAngleInput->setEnabled(true);
    m_antialiasInput->setEnabled(true);
    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamShearToolImagesPlugin
