/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-11-28
 * Description : a digiKam image editor plugin to process image
 *               free rotation.
 *
 * Copyright 2004-2007 by Gilles Caulier 
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

#include <qlabel.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qimage.h>
#include <qcombobox.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kcursor.h>
#include <kseparator.h>
#include <kconfig.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "freerotation.h"
#include "imageeffect_freerotation.h"
#include "imageeffect_freerotation.moc"

namespace DigikamFreeRotationImagesPlugin
{

ImageEffect_FreeRotation::ImageEffect_FreeRotation(QWidget* parent)
                        : Digikam::ImageGuideDlg(parent, i18n("Free Rotation"), "freerotation",
                          false, true, true, Digikam::ImageGuideWidget::HVGuideMode)
{
    // No need Abort button action.
    showButton(User1, false);

    QString whatsThis;

    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Free Rotation"),
                                       digikam_version,
                                       I18N_NOOP("A digiKam image plugin to process free image "
                                       "rotation."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2007, Gilles Caulier",
                                       0,
                                       "http://www.digikam.org");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Free Rotation algorithm"),
                     "pieter dot voloshyn at gmail dot com");

    setAboutData(about);

    QWhatsThis::add( m_imagePreviewWidget, i18n("<p>This is the free image operation preview. "
                                           "If you move the mouse cursor on this preview, "
                                           "a vertical and horizontal dashed line will be drawn "
                                           "to guide you in adjusting the free rotation correction. "
                                           "Release the left mouse button to freeze the dashed "
                                           "line's position."));

    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 9, 2, spacingHint());

    QLabel *label1 = new QLabel(i18n("New width:"), gboxSettings);
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), gboxSettings);
    m_newWidthLabel->setAlignment( AlignBottom | AlignRight );

    QLabel *label2 = new QLabel(i18n("New height:"), gboxSettings);
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), gboxSettings);
    m_newHeightLabel->setAlignment( AlignBottom | AlignRight );

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_newWidthLabel, 0, 0, 1, 2);
    gridSettings->addMultiCellWidget(label2, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_newHeightLabel, 1, 1, 1, 2);

    KSeparator *line = new KSeparator (Horizontal, gboxSettings);
    gridSettings->addMultiCellWidget(line, 2, 2, 0, 2);

    QLabel *label3 = new QLabel(i18n("Main angle:"), gboxSettings);
    m_angleInput = new KIntNumInput(gboxSettings);
    m_angleInput->setRange(-180, 180, 1, true);
    m_angleInput->setValue(0);
    QWhatsThis::add( m_angleInput, i18n("<p>An angle in degrees by which to rotate the image. "
                                        "A positive angle rotates the image clockwise; "
                                        "a negative angle rotates it counter-clockwise."));

    gridSettings->addMultiCellWidget(label3, 3, 3, 0, 2);
    gridSettings->addMultiCellWidget(m_angleInput, 4, 4, 0, 2);

    QLabel *label4 = new QLabel(i18n("Fine angle:"), gboxSettings);
    m_fineAngleInput = new KDoubleNumInput(gboxSettings);
    m_fineAngleInput->setRange(-5.0, 5.0, 0.01, true);
    m_fineAngleInput->setValue(0);
    QWhatsThis::add( m_fineAngleInput, i18n("<p>This value in degrees will be added to main angle value "
                                            "to set fine target angle."));

    gridSettings->addMultiCellWidget(label4, 5, 5, 0, 2);
    gridSettings->addMultiCellWidget(m_fineAngleInput, 6, 6, 0, 2);

    m_antialiasInput = new QCheckBox(i18n("Anti-Aliasing"), gboxSettings);
    QWhatsThis::add( m_antialiasInput, i18n("<p>Enable this option to process anti-aliasing filter "
                                            "to the rotated image. "
                                            "In order to smooth the target image, it will be blurred a little."));
    gridSettings->addMultiCellWidget(m_antialiasInput, 7, 7, 0, 2);

    QLabel *label5 = new QLabel(i18n("Auto-crop:"), gboxSettings);
    m_autoCropCB = new QComboBox(false, gboxSettings);
    m_autoCropCB->insertItem( i18n("None") );
    m_autoCropCB->insertItem( i18n("Widest Area") );
    m_autoCropCB->insertItem( i18n("Largest Area") );
    QWhatsThis::add( m_autoCropCB, i18n("<p>Select here the method to process image auto-cropping "
                                            "to remove black frames around a rotated image."));
    gridSettings->addMultiCellWidget(label5, 8, 8, 0, 0);
    gridSettings->addMultiCellWidget(m_autoCropCB, 8, 8, 1, 2);

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_angleInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(m_fineAngleInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(m_antialiasInput, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));

    connect(m_autoCropCB, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));
}

ImageEffect_FreeRotation::~ImageEffect_FreeRotation()
{
}

void ImageEffect_FreeRotation::readUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("freerotation Tool Dialog");
    m_angleInput->setValue(config->readNumEntry("Main Angle", 0));
    m_fineAngleInput->setValue(config->readDoubleNumEntry("Fine Angle", 0.0));
    m_autoCropCB->setCurrentItem(config->readNumEntry("Auto Crop Type", FreeRotation::NoAutoCrop));
    m_antialiasInput->setChecked(config->readBoolEntry("Anti Aliasing", true));
    slotEffect();
}

void ImageEffect_FreeRotation::writeUserSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("freerotation Tool Dialog");
    config->writeEntry("Main Angle", m_angleInput->value());
    config->writeEntry("Fine Angle", m_fineAngleInput->value());
    config->writeEntry( "Auto Crop Type", m_autoCropCB->currentItem() );
    config->writeEntry( "Anti Aliasing", m_antialiasInput->isChecked() );
    config->sync();
}

void ImageEffect_FreeRotation::resetValues()
{
    m_angleInput->blockSignals(true);
    m_antialiasInput->blockSignals(true);
    m_autoCropCB->blockSignals(true);
    m_angleInput->setValue(0);
    m_fineAngleInput->setValue(0.0);
    m_antialiasInput->setChecked(true);
    m_autoCropCB->setCurrentItem(FreeRotation::NoAutoCrop);
    m_angleInput->blockSignals(false);
    m_antialiasInput->blockSignals(false);
    m_autoCropCB->blockSignals(false);
}

void ImageEffect_FreeRotation::prepareEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_angleInput->setEnabled(false);
    m_fineAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);
    m_autoCropCB->setEnabled(false);

    double angle      = m_angleInput->value() + m_fineAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    int autocrop      = m_autoCropCB->currentItem();
    QColor background = paletteBackgroundColor().rgb();

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int orgW = iface->originalWidth();
    int orgH = iface->originalHeight();

    uchar *data = iface->getPreviewImage();
    Digikam::DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
                        iface->previewHasAlpha(), data);
    delete [] data;                        

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new FreeRotation(&image, this, angle, antialiasing, autocrop,
                                        background, orgW, orgH));
}

void ImageEffect_FreeRotation::prepareFinal()
{
    m_angleInput->setEnabled(false);
    m_fineAngleInput->setEnabled(false);
    m_antialiasInput->setEnabled(false);
    m_autoCropCB->setEnabled(false);

    double angle      = m_angleInput->value() + m_fineAngleInput->value();
    bool antialiasing = m_antialiasInput->isChecked();
    int autocrop      = m_autoCropCB->currentItem();
    QColor background = Qt::black;

    Digikam::ImageIface iface(0, 0);
    int orgW = iface.originalWidth();
    int orgH = iface.originalHeight();
    
    uchar *data = iface.getOriginalImage();
    Digikam::DImg orgImage(orgW, orgH, iface.originalSixteenBit(),
                           iface.originalHasAlpha(), data);
    delete [] data;
    
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new FreeRotation(&orgImage, this, angle, antialiasing, autocrop,
                                        background, orgW, orgH));
}

void ImageEffect_FreeRotation::putPreviewData(void)
{
    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();

    Digikam::DImg imTemp = m_threadedFilter->getTargetImage().smoothScale(w, h, QSize::ScaleMin);
    Digikam::DImg imDest( w, h, m_threadedFilter->getTargetImage().sixteenBit(),
                                m_threadedFilter->getTargetImage().hasAlpha() );
    
    imDest.fill( Digikam::DColor(paletteBackgroundColor().rgb(),
                                 m_threadedFilter->getTargetImage().sixteenBit()) );
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());

    m_imagePreviewWidget->updatePreview();
    QSize newSize = dynamic_cast<FreeRotation *>(m_threadedFilter)->getNewSize();
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width()) + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );
}

void ImageEffect_FreeRotation::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    Digikam::DImg targetImage = m_threadedFilter->getTargetImage();
    iface.putOriginalImage(i18n("Free Rotation"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

void ImageEffect_FreeRotation::renderingFinished()
{
    m_angleInput->setEnabled(true);
    m_fineAngleInput->setEnabled(true);
    m_antialiasInput->setEnabled(true);
    m_autoCropCB->setEnabled(true);
    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamFreeRotationImagesPlugin

