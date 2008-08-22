/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image plugin to add a border
 *               around an image.
 *
 * Copyright 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>

// KDE includes.

#include <kseparator.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kcolorbutton.h>
#include <kglobal.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "border.h"
#include "imageeffect_border.h"
#include "imageeffect_border.moc"

using namespace KDcrawIface;

namespace DigikamBorderImagesPlugin
{

ImageEffect_Border::ImageEffect_Border(QWidget* parent)
                  : Digikam::ImageGuideDlg(parent, i18n("Add Border Around Photograph"),
                                           "border", false, false, false,
                                           Digikam::ImageGuideWidget::HVGuideMode)
{
    // No need Abort button action.
    showButton(User1, false);

   QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Add Border"),
                                       digiKamVersion().toAscii(),
                                       ki18n("A digiKam image plugin to add a border around an image."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout* gridSettings = new QGridLayout(gboxSettings);

    QLabel *label1 = new QLabel(i18n("Type:"), gboxSettings);

    m_borderType   = new QComboBox( gboxSettings );
    m_borderType->addItem( i18n("Solid") );
    // NOTE: Niepce is Real name. This is the first guy in the world to have built a camera.
    m_borderType->addItem( "Niepce" );
    m_borderType->addItem( i18n("Beveled") );
    m_borderType->addItem( i18n("Decorative Pine") );
    m_borderType->addItem( i18n("Decorative Wood") );
    m_borderType->addItem( i18n("Decorative Paper") );
    m_borderType->addItem( i18n("Decorative Parquet") );
    m_borderType->addItem( i18n("Decorative Ice") );
    m_borderType->addItem( i18n("Decorative Leaf") );
    m_borderType->addItem( i18n("Decorative Marble") );
    m_borderType->addItem( i18n("Decorative Rain") );
    m_borderType->addItem( i18n("Decorative Craters") );
    m_borderType->addItem( i18n("Decorative Dried") );
    m_borderType->addItem( i18n("Decorative Pink") );
    m_borderType->addItem( i18n("Decorative Stone") );
    m_borderType->addItem( i18n("Decorative Chalk") );
    m_borderType->addItem( i18n("Decorative Granite") );
    m_borderType->addItem( i18n("Decorative Rock") );
    m_borderType->addItem( i18n("Decorative Wall") );
    m_borderType->setWhatsThis( i18n("<p>Select the border type to add around the image here."));

    KSeparator *line1 = new KSeparator(Qt::Horizontal, gboxSettings);

    // -------------------------------------------------------------------

    m_preserveAspectRatio = new QCheckBox(gboxSettings);
    m_preserveAspectRatio->setText(i18n("Preserve Aspect Ratio"));
    m_preserveAspectRatio->setWhatsThis( i18n("Enable this option if you want to preserve the aspect "
                                              "ratio of image. If enabled, the border width will be "
                                              "in percent of the image size, else the border width will be "
                                              "in pixels."));

    m_labelBorderPercent  = new QLabel(i18n("Width (%):"), gboxSettings);
    m_borderPercent       = new KIntNumInput(gboxSettings);
    m_borderPercent->setRange(1, 50, 1);
    m_borderPercent->setSliderEnabled(true);
    m_borderPercent->setWhatsThis( i18n("<p>Set here the border width in percent of the image size."));

    m_labelBorderWidth = new QLabel(i18n("Width (pixels):"), gboxSettings);
    m_borderWidth      = new KIntNumInput(gboxSettings);
    m_borderWidth->setSliderEnabled(true);
    m_borderWidth->setWhatsThis( i18n("<p>Set here the border width in pixels to add around the image."));

    Digikam::ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();

    if (w > h)
        m_borderWidth->setRange(1, h/2, 1);
    else
        m_borderWidth->setRange(1, w/2, 1);

    KSeparator *line2 = new KSeparator(Qt::Horizontal, gboxSettings);

    // -------------------------------------------------------------------

    m_labelForeground   = new QLabel(gboxSettings);
    m_firstColorButton  = new KColorButton( QColor::QColor( 192, 192, 192 ), gboxSettings );
    m_labelBackground   = new QLabel(gboxSettings);
    m_secondColorButton = new KColorButton( QColor::QColor( 128, 128, 128 ), gboxSettings );

    // -------------------------------------------------------------------

    gridSettings->addWidget(label1, 0, 0, 1, 3 );
    gridSettings->addWidget(m_borderType, 1, 0, 1, 3 );
    gridSettings->addWidget(line1, 2, 0, 1, 3 );
    gridSettings->addWidget(m_preserveAspectRatio, 3, 0, 1, 3 );
    gridSettings->addWidget(m_labelBorderPercent, 4, 0, 1, 3 );
    gridSettings->addWidget(m_borderPercent, 5, 0, 1, 3 );
    gridSettings->addWidget(m_labelBorderWidth, 6, 0, 1, 3 );
    gridSettings->addWidget(m_borderWidth, 7, 0, 1, 3 );
    gridSettings->addWidget(line2, 8, 0, 1, 3 );
    gridSettings->addWidget(m_labelForeground, 9, 0, 1, 1);
    gridSettings->addWidget(m_firstColorButton, 9, 1, 1, 2);
    gridSettings->addWidget(m_labelBackground, 10, 0, 1, 1);
    gridSettings->addWidget(m_secondColorButton, 10, 1, 1, 2);
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_preserveAspectRatio, SIGNAL(toggled(bool)),
            this, SLOT(slotPreserveAspectRatioToggled(bool)));

    connect(m_borderType, SIGNAL(activated(int)),
            this, SLOT(slotBorderTypeChanged(int)));

    connect(m_borderPercent, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_borderWidth, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_firstColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorForegroundChanged(const QColor &)));

    connect(m_secondColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorBackgroundChanged(const QColor &)));
}

ImageEffect_Border::~ImageEffect_Border()
{
}

void ImageEffect_Border::readUserSettings(void)
{
    m_borderType->blockSignals(true);
    m_borderPercent->blockSignals(true);
    m_borderWidth->blockSignals(true);
    m_firstColorButton->blockSignals(true);
    m_secondColorButton->blockSignals(true);
    m_preserveAspectRatio->blockSignals(true);

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group =  config->group("border Tool Dialog");

    m_borderType->setCurrentIndex(group.readEntry("Border Type",(int)Border::SolidBorder));
    m_borderPercent->setValue(group.readEntry("Border Percent", 10) );
    m_borderWidth->setValue(group.readEntry("Border Width", 100) );
    m_preserveAspectRatio->setChecked(group.readEntry("Preserve Aspect Ratio", true) );

    QColor black(0, 0, 0);
    QColor white(255, 255, 255);
    QColor gray1(192, 192, 192);
    QColor gray2(128, 128, 128);

    m_solidColor = group.readEntry("Solid Color", black);
    m_niepceBorderColor = group.readEntry("Niepce Border Color", white);
    m_niepceLineColor = group.readEntry("Niepce Line Color", black);
    m_bevelUpperLeftColor = group.readEntry("Bevel Upper Left Color", gray1);
    m_bevelLowerRightColor = group.readEntry("Bevel Lower Right Color", gray2);
    m_decorativeFirstColor = group.readEntry("Decorative First Color", black);
    m_decorativeSecondColor = group.readEntry("Decorative Second Color", black);

    m_borderType->blockSignals(false);
    m_borderPercent->blockSignals(false);
    m_borderWidth->blockSignals(false);
    m_firstColorButton->blockSignals(false);
    m_secondColorButton->blockSignals(false);
    m_preserveAspectRatio->blockSignals(false);

    slotBorderTypeChanged(m_borderType->currentIndex());
}

void ImageEffect_Border::writeUserSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("border Tool Dialog");

    group.writeEntry("Border Type", m_borderType->currentIndex());
    group.writeEntry("Border Percent", m_borderPercent->value());
    group.writeEntry("Border Width", m_borderWidth->value());
    group.writeEntry("Preserve Aspect Ratio", m_preserveAspectRatio->isChecked());

    group.writeEntry("Solid Color", m_solidColor);
    group.writeEntry("Niepce Border Color", m_niepceBorderColor);
    group.writeEntry("Niepce Line Color", m_niepceLineColor);
    group.writeEntry("Bevel Upper Left Color", m_bevelUpperLeftColor);
    group.writeEntry("Bevel Lower Right Color", m_bevelLowerRightColor);
    group.writeEntry("Decorative First Color", m_decorativeFirstColor);
    group.writeEntry("Decorative Second Color", m_decorativeSecondColor);

    group.sync();
}

void ImageEffect_Border::resetValues()
{
    m_borderType->blockSignals(true);
    m_borderPercent->blockSignals(true);
    m_borderWidth->blockSignals(true);
    m_firstColorButton->blockSignals(true);
    m_secondColorButton->blockSignals(true);
    m_preserveAspectRatio->blockSignals(true);

    m_borderType->setCurrentIndex(Border::SolidBorder);
    m_borderPercent->setValue(10);
    m_borderWidth->setValue(100);
    m_preserveAspectRatio->setChecked(true);

    m_solidColor            = QColor(0, 0, 0);
    m_niepceBorderColor     = QColor(255, 255, 255);
    m_niepceLineColor       = QColor(0, 0, 0);
    m_bevelUpperLeftColor   = QColor(192, 192, 192);
    m_bevelLowerRightColor  = QColor(128, 128, 128);
    m_decorativeFirstColor  = QColor(0, 0, 0);
    m_decorativeSecondColor = QColor(0, 0, 0);

    m_borderType->blockSignals(false);
    m_borderPercent->blockSignals(false);
    m_borderWidth->blockSignals(false);
    m_firstColorButton->blockSignals(false);
    m_secondColorButton->blockSignals(false);
    m_preserveAspectRatio->blockSignals(false);

    slotBorderTypeChanged(Border::SolidBorder);
}

void ImageEffect_Border::renderingFinished()
{
    m_preserveAspectRatio->setEnabled(true);
    m_borderType->setEnabled(true);
    m_borderPercent->setEnabled(true);
    m_borderWidth->setEnabled(true);
    m_firstColorButton->setEnabled(true);
    m_secondColorButton->setEnabled(true);
    toggleBorderSlider(m_preserveAspectRatio->isChecked());
}

void ImageEffect_Border::slotColorForegroundChanged(const QColor &color)
{
    switch (m_borderType->currentIndex())
       {
       case Border::SolidBorder:
          m_solidColor = color;
          break;

       case Border::NiepceBorder:
          m_niepceBorderColor = color;
          break;

       case Border::BeveledBorder:
          m_bevelUpperLeftColor = color;
          break;

       case Border::PineBorder:
       case Border::WoodBorder:
       case Border::PaperBorder:
       case Border::ParqueBorder:
       case Border::IceBorder:
       case Border::LeafBorder:
       case Border::MarbleBorder:
       case Border::RainBorder:
       case Border::CratersBorder:
       case Border::DriedBorder:
       case Border::PinkBorder:
       case Border::StoneBorder:
       case Border::ChalkBorder:
       case Border::GraniteBorder:
       case Border::RockBorder:
       case Border::WallBorder:
          m_decorativeFirstColor = color;
          break;
       }

    slotEffect();
}

void ImageEffect_Border::slotColorBackgroundChanged(const QColor &color)
{
    switch (m_borderType->currentIndex())
       {
       case Border::SolidBorder:
          m_solidColor = color;
          break;

       case Border::NiepceBorder:
          m_niepceLineColor = color;
          break;

       case Border::BeveledBorder:
          m_bevelLowerRightColor = color;
          break;

       case Border::PineBorder:
       case Border::WoodBorder:
       case Border::PaperBorder:
       case Border::ParqueBorder:
       case Border::IceBorder:
       case Border::LeafBorder:
       case Border::MarbleBorder:
       case Border::RainBorder:
       case Border::CratersBorder:
       case Border::DriedBorder:
       case Border::PinkBorder:
       case Border::StoneBorder:
       case Border::ChalkBorder:
       case Border::GraniteBorder:
       case Border::RockBorder:
       case Border::WallBorder:
          m_decorativeSecondColor = color;
          break;
       }

    slotEffect();
}

void ImageEffect_Border::slotBorderTypeChanged(int borderType)
{
    m_labelForeground->setText(i18n("First:"));
    m_labelBackground->setText(i18n("Second:"));
    m_firstColorButton->setWhatsThis( i18n("<p>Set here the foreground color of the border."));
    m_secondColorButton->setWhatsThis( i18n("<p>Set here the Background color of the border."));
    m_firstColorButton->setEnabled(true);
    m_secondColorButton->setEnabled(true);
    m_labelForeground->setEnabled(true);
    m_labelBackground->setEnabled(true);
    m_borderPercent->setEnabled(true);

    switch (borderType)
       {
       case Border::SolidBorder:
          m_firstColorButton->setColor( m_solidColor );
          m_secondColorButton->setEnabled(false);
          m_labelBackground->setEnabled(false);
          break;

       case Border::NiepceBorder:
          m_firstColorButton->setWhatsThis( i18n("<p>Set here the color of the main border."));
          m_secondColorButton->setWhatsThis( i18n("<p>Set here the color of the line."));
          m_firstColorButton->setColor( m_niepceBorderColor );
          m_secondColorButton->setColor( m_niepceLineColor );
          break;

       case Border::BeveledBorder:
          m_firstColorButton->setWhatsThis( i18n("<p>Set here the color of the upper left area."));
          m_secondColorButton->setWhatsThis( i18n("<p>Set here the color of the lower right area."));
          m_firstColorButton->setColor( m_bevelUpperLeftColor );
          m_secondColorButton->setColor( m_bevelLowerRightColor );
          break;

       case Border::PineBorder:
       case Border::WoodBorder:
       case Border::PaperBorder:
       case Border::ParqueBorder:
       case Border::IceBorder:
       case Border::LeafBorder:
       case Border::MarbleBorder:
       case Border::RainBorder:
       case Border::CratersBorder:
       case Border::DriedBorder:
       case Border::PinkBorder:
       case Border::StoneBorder:
       case Border::ChalkBorder:
       case Border::GraniteBorder:
       case Border::RockBorder:
       case Border::WallBorder:
          m_firstColorButton->setWhatsThis( i18n("<p>Set here the color of the first line."));
          m_secondColorButton->setWhatsThis( i18n("<p>Set here the color of the second line."));
          m_firstColorButton->setColor( m_decorativeFirstColor );
          m_secondColorButton->setColor( m_decorativeSecondColor );
          break;
       }

    slotEffect();
}

void ImageEffect_Border::prepareEffect()
{
    m_borderType->setEnabled(false);
    m_borderPercent->setEnabled(false);
    m_borderWidth->setEnabled(false);
    m_firstColorButton->setEnabled(false);
    m_secondColorButton->setEnabled(false);
    m_preserveAspectRatio->setEnabled(false);

    Digikam::ImageIface* iface = m_imagePreviewWidget->imageIface();
    int orgWidth               = iface->originalWidth();
    int orgHeight              = iface->originalHeight();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sixteenBit            = iface->previewSixteenBit();
    uchar *data                = iface->getPreviewImage();
    Digikam::DImg previewImage(w, h, sixteenBit,
                               iface->previewHasAlpha(), data);
    delete [] data;

    int borderType  = m_borderType->currentIndex();
    float ratio     = (float)w/(float)orgWidth;
    int borderWidth = (int)((float)m_borderWidth->value()*ratio);
    QString border  = getBorderPath( m_borderType->currentIndex() );

    if (m_preserveAspectRatio->isChecked())
    {
        m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                           new Border(&previewImage, this, orgWidth, orgHeight,
                                      border, borderType, m_borderPercent->value()/100.0,
                                      Digikam::DColor(m_solidColor, sixteenBit),
                                      Digikam::DColor(m_niepceBorderColor, sixteenBit),
                                      Digikam::DColor(m_niepceLineColor, sixteenBit),
                                      Digikam::DColor(m_bevelUpperLeftColor, sixteenBit),
                                      Digikam::DColor(m_bevelLowerRightColor, sixteenBit),
                                      Digikam::DColor(m_decorativeFirstColor, sixteenBit),
                                      Digikam::DColor(m_decorativeSecondColor, sixteenBit)) );
    }
    else
    {
        m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                           new Border(&previewImage, this, orgWidth, orgHeight,
                                      border, borderType, borderWidth,
                                      (int)(20.0*ratio), (int)(20.0*ratio), 3,
                                      Digikam::DColor(m_solidColor, sixteenBit),
                                      Digikam::DColor(m_niepceBorderColor, sixteenBit),
                                      Digikam::DColor(m_niepceLineColor, sixteenBit),
                                      Digikam::DColor(m_bevelUpperLeftColor, sixteenBit),
                                      Digikam::DColor(m_bevelLowerRightColor, sixteenBit),
                                      Digikam::DColor(m_decorativeFirstColor, sixteenBit),
                                      Digikam::DColor(m_decorativeSecondColor, sixteenBit)) );
    }
}

void ImageEffect_Border::prepareFinal()
{
    m_borderType->setEnabled(false);
    m_borderPercent->setEnabled(false);
    m_borderWidth->setEnabled(false);
    m_firstColorButton->setEnabled(false);
    m_secondColorButton->setEnabled(false);

    int borderType    = m_borderType->currentIndex();
    int borderWidth   = m_borderWidth->value();
    float borderRatio = m_borderPercent->value()/100.0;
    QString border    = getBorderPath( m_borderType->currentIndex() );

    Digikam::ImageIface iface(0, 0);
    int orgWidth    = iface.originalWidth();
    int orgHeight   = iface.originalHeight();
    bool sixteenBit = iface.previewSixteenBit();
    uchar *data     = iface.getOriginalImage();
    Digikam::DImg orgImage(orgWidth, orgHeight, sixteenBit,
                           iface.originalHasAlpha(), data);
    delete [] data;

    if (m_preserveAspectRatio->isChecked())
    {
        m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                           new Border(&orgImage, this, orgWidth, orgHeight,
                                    border, borderType, borderRatio,
                                    Digikam::DColor(m_solidColor, sixteenBit),
                                    Digikam::DColor(m_niepceBorderColor, sixteenBit),
                                    Digikam::DColor(m_niepceLineColor, sixteenBit),
                                    Digikam::DColor(m_bevelUpperLeftColor, sixteenBit),
                                    Digikam::DColor(m_bevelLowerRightColor, sixteenBit),
                                    Digikam::DColor(m_decorativeFirstColor, sixteenBit),
                                    Digikam::DColor(m_decorativeSecondColor, sixteenBit)) );
    }
    else
    {
        m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                           new Border(&orgImage, this, orgWidth, orgHeight,
                                    border, borderType, borderWidth, 15, 15, 10,
                                    Digikam::DColor(m_solidColor, sixteenBit),
                                    Digikam::DColor(m_niepceBorderColor, sixteenBit),
                                    Digikam::DColor(m_niepceLineColor, sixteenBit),
                                    Digikam::DColor(m_bevelUpperLeftColor, sixteenBit),
                                    Digikam::DColor(m_bevelLowerRightColor, sixteenBit),
                                    Digikam::DColor(m_decorativeFirstColor, sixteenBit),
                                    Digikam::DColor(m_decorativeSecondColor, sixteenBit)) );
    }
}

void ImageEffect_Border::putPreviewData(void)
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

    iface->putPreviewImage(imDest.bits());
    m_imagePreviewWidget->updatePreview();
}

void ImageEffect_Border::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    Digikam::DImg targetImage = m_threadedFilter->getTargetImage();
    iface.putOriginalImage(i18n("Add Border"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

QString ImageEffect_Border::getBorderPath(int border)
{
    QString pattern;

    switch (border)
       {
       case Border::PineBorder:
          pattern = "pine-pattern";
          break;

       case Border::WoodBorder:
          pattern = "wood-pattern";
          break;

       case Border::PaperBorder:
          pattern = "paper-pattern";
          break;

       case Border::ParqueBorder:
          pattern = "parque-pattern";
          break;

       case Border::IceBorder:
          pattern = "ice-pattern";
          break;

       case Border::LeafBorder:
          pattern = "leaf-pattern";
          break;

       case Border::MarbleBorder:
          pattern = "marble-pattern";
          break;

       case Border::RainBorder:
          pattern = "rain-pattern";
          break;

       case Border::CratersBorder:
          pattern = "craters-pattern";
          break;

       case Border::DriedBorder:
          pattern = "dried-pattern";
          break;

       case Border::PinkBorder:
          pattern = "pink-pattern";
          break;

       case Border::StoneBorder:
          pattern = "stone-pattern";
          break;

       case Border::ChalkBorder:
          pattern = "chalk-pattern";
          break;

       case Border::GraniteBorder:
          pattern = "granit-pattern";
          break;

       case Border::RockBorder:
          pattern = "rock-pattern";
          break;

       case Border::WallBorder:
          pattern = "wall-pattern";
          break;

       default:
          return QString();
       }

    return (KStandardDirs::locate("data", QString("digikam/data/") + pattern + QString(".png")));
}

void ImageEffect_Border::slotPreserveAspectRatioToggled(bool b)
{
    toggleBorderSlider(b);
    slotTimer();
}

void ImageEffect_Border::toggleBorderSlider(bool b)
{
    m_borderPercent->setEnabled(b);
    m_borderWidth->setEnabled(!b);
    m_labelBorderPercent->setEnabled(b);
    m_labelBorderWidth->setEnabled(!b);
}

}  // NameSpace DigikamBorderImagesPlugin
