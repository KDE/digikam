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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
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
#include "imageiface.h"
#include "imagewidget.h"
#include "editortoolsettings.h"
#include "border.h"
#include "bordertool.h"
#include "bordertool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamBorderImagesPlugin
{

BorderTool::BorderTool(QObject* parent)
          : EditorToolThreaded(parent)
{
    setName("border");
    setToolName(i18n("Add Border"));
    setToolIcon(SmallIcon("bordertool"));

    m_previewWidget = new ImageWidget("bordertool Tool", 0, QString(),
                                      false, ImageGuideWidget::HVGuideMode, false);

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings    = new EditorToolSettings(EditorToolSettings::Default|
                                               EditorToolSettings::Ok|
                                               EditorToolSettings::Cancel);
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage(), 11, 2);

    QLabel *label1    = new QLabel(i18n("Type:"), m_gboxSettings->plainPage());

    m_borderType      = new RComboBox(m_gboxSettings->plainPage());
    m_borderType->insertItem( i18n("Solid") );
    // Niepce is Real name. This is the first guy in the world to have built a camera.
    m_borderType->insertItem( "Niepce" );
    m_borderType->insertItem( i18n("Beveled") );
    m_borderType->insertItem( i18n("Decorative Pine") );
    m_borderType->insertItem( i18n("Decorative Wood") );
    m_borderType->insertItem( i18n("Decorative Paper") );
    m_borderType->insertItem( i18n("Decorative Parquet") );
    m_borderType->insertItem( i18n("Decorative Ice") );
    m_borderType->insertItem( i18n("Decorative Leaf") );
    m_borderType->insertItem( i18n("Decorative Marble") );
    m_borderType->insertItem( i18n("Decorative Rain") );
    m_borderType->insertItem( i18n("Decorative Craters") );
    m_borderType->insertItem( i18n("Decorative Dried") );
    m_borderType->insertItem( i18n("Decorative Pink") );
    m_borderType->insertItem( i18n("Decorative Stone") );
    m_borderType->insertItem( i18n("Decorative Chalk") );
    m_borderType->insertItem( i18n("Decorative Granite") );
    m_borderType->insertItem( i18n("Decorative Rock") );
    m_borderType->insertItem( i18n("Decorative Wall") );
    m_borderType->setDefaultItem(Border::SolidBorder);
    QWhatsThis::add( m_borderType, i18n("<p>Select the border type to add around the image."));

    KSeparator *line1 = new KSeparator(Horizontal, m_gboxSettings->plainPage());

    // -------------------------------------------------------------------

    m_preserveAspectRatio = new QCheckBox(m_gboxSettings->plainPage());
    m_preserveAspectRatio->setText(i18n("Preserve Aspect Ratio"));
    QWhatsThis::add(m_preserveAspectRatio, i18n("Enable this option if you want to preserve the aspect "
                                                "ratio of the image. If enabled, the border width will be "
                                                "in percent of the image size, else the border width will "
                                                "in pixels."));

    m_labelBorderPercent  = new QLabel(i18n("Width (%):"), m_gboxSettings->plainPage());
    m_borderPercent       = new RIntNumInput(m_gboxSettings->plainPage());
    m_borderPercent->setRange(1, 50, 1);
    m_borderPercent->setDefaultValue(10);
    QWhatsThis::add(m_borderPercent, i18n("<p>Set here the border width in percent of the image size."));

    m_labelBorderWidth = new QLabel(i18n("Width (pixels):"), m_gboxSettings->plainPage());
    m_borderWidth      = new RIntNumInput(m_gboxSettings->plainPage());
    m_borderWidth->setDefaultValue(100);
    QWhatsThis::add(m_borderWidth, i18n("<p>Set here the border width in pixels to add around the image."));

    ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();

    if (w > h)
       m_borderWidth->setRange(1, h/2, 1);
    else
       m_borderWidth->setRange(1, w/2, 1);

    KSeparator *line2 = new KSeparator(Horizontal, m_gboxSettings->plainPage());

    // -------------------------------------------------------------------

    m_labelForeground   = new QLabel(m_gboxSettings->plainPage());
    m_firstColorButton  = new KColorButton( QColor::QColor( 192, 192, 192 ), m_gboxSettings->plainPage() );
    m_labelBackground   = new QLabel(m_gboxSettings->plainPage());
    m_secondColorButton = new KColorButton( QColor::QColor( 128, 128, 128 ), m_gboxSettings->plainPage() );

    // -------------------------------------------------------------------

    grid->addMultiCellWidget(label1,                0, 0, 0, 2);
    grid->addMultiCellWidget(m_borderType,          1, 1, 0, 2);
    grid->addMultiCellWidget(line1,                 2, 2, 0, 2);
    grid->addMultiCellWidget(m_preserveAspectRatio, 3, 3, 0, 2);
    grid->addMultiCellWidget(m_labelBorderPercent,  4, 4, 0, 2);
    grid->addMultiCellWidget(m_borderPercent,       5, 5, 0, 2);
    grid->addMultiCellWidget(m_labelBorderWidth,    6, 6, 0, 2);
    grid->addMultiCellWidget(m_borderWidth,         7, 7, 0, 2);
    grid->addMultiCellWidget(line2,                 8, 8, 0, 2);
    grid->addMultiCellWidget(m_labelForeground,     9, 9, 0, 0);
    grid->addMultiCellWidget(m_firstColorButton,    9, 9, 1, 2);
    grid->addMultiCellWidget(m_labelBackground,     10, 10, 0, 0);
    grid->addMultiCellWidget(m_secondColorButton,   10, 10, 1, 2);
    grid->setRowStretch(11, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

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

BorderTool::~BorderTool()
{
}

void BorderTool::readSettings()
{
    m_borderType->blockSignals(true);
    m_borderPercent->blockSignals(true);
    m_borderWidth->blockSignals(true);
    m_firstColorButton->blockSignals(true);
    m_secondColorButton->blockSignals(true);
    m_preserveAspectRatio->blockSignals(true);

    KConfig *config = kapp->config();
    config->setGroup("border Tool");

    m_borderType->setCurrentItem(config->readNumEntry("Border Type", m_borderType->defaultItem()));
    m_borderPercent->setValue(config->readNumEntry("Border Percent", m_borderPercent->defaultValue()));
    m_borderWidth->setValue(config->readNumEntry("Border Width", m_borderWidth->defaultValue()));
    m_preserveAspectRatio->setChecked(config->readBoolEntry("Preserve Aspect Ratio", true));

    QColor black(0, 0, 0);
    QColor white(255, 255, 255);
    QColor gray1(192, 192, 192);
    QColor gray2(128, 128, 128);

    m_solidColor            = config->readColorEntry("Solid Color", &black);
    m_niepceBorderColor     = config->readColorEntry("Niepce Border Color", &white);
    m_niepceLineColor       = config->readColorEntry("Niepce Line Color", &black);
    m_bevelUpperLeftColor   = config->readColorEntry("Bevel Upper Left Color", &gray1);
    m_bevelLowerRightColor  = config->readColorEntry("Bevel Lower Right Color", &gray2);
    m_decorativeFirstColor  = config->readColorEntry("Decorative First Color", &black);
    m_decorativeSecondColor = config->readColorEntry("Decorative Second Color", &black);

    m_borderType->blockSignals(false);
    m_borderPercent->blockSignals(false);
    m_borderWidth->blockSignals(false);
    m_firstColorButton->blockSignals(false);
    m_secondColorButton->blockSignals(false);
    m_preserveAspectRatio->blockSignals(false);

    slotBorderTypeChanged(m_borderType->currentItem());
}

void BorderTool::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("border Tool");

    config->writeEntry("Border Type", m_borderType->currentItem());
    config->writeEntry("Border Percent", m_borderPercent->value());
    config->writeEntry("Border Width", m_borderWidth->value());
    config->writeEntry("Preserve Aspect Ratio", m_preserveAspectRatio->isChecked());

    config->writeEntry("Solid Color", m_solidColor);
    config->writeEntry("Niepce Border Color", m_niepceBorderColor);
    config->writeEntry("Niepce Line Color", m_niepceLineColor);
    config->writeEntry("Bevel Upper Left Color", m_bevelUpperLeftColor);
    config->writeEntry("Bevel Lower Right Color", m_bevelLowerRightColor);
    config->writeEntry("Decorative First Color", m_decorativeFirstColor);
    config->writeEntry("Decorative Second Color", m_decorativeSecondColor);

    m_previewWidget->writeSettings();

    config->sync();
}

void BorderTool::slotResetSettings()
{
    m_borderType->blockSignals(true);
    m_borderPercent->blockSignals(true);
    m_borderWidth->blockSignals(true);
    m_firstColorButton->blockSignals(true);
    m_secondColorButton->blockSignals(true);
    m_preserveAspectRatio->blockSignals(true);

    m_borderType->slotReset();
    m_borderPercent->slotReset();
    m_borderWidth->slotReset();
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

void BorderTool::renderingFinished()
{
    m_preserveAspectRatio->setEnabled(true);
    m_borderType->setEnabled(true);
    m_borderPercent->setEnabled(true);
    m_borderWidth->setEnabled(true);
    m_firstColorButton->setEnabled(true);
    m_secondColorButton->setEnabled(true);
    toggleBorderSlider(m_preserveAspectRatio->isChecked());
}

void BorderTool::slotColorForegroundChanged(const QColor &color)
{
    switch (m_borderType->currentItem())
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

void BorderTool::slotColorBackgroundChanged(const QColor &color)
{
    switch (m_borderType->currentItem())
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

void BorderTool::slotBorderTypeChanged(int borderType)
{
    m_labelForeground->setText(i18n("First:"));
    m_labelBackground->setText(i18n("Second:"));
    QWhatsThis::add( m_firstColorButton, i18n("<p>Set here the foreground color of the border."));
    QWhatsThis::add( m_secondColorButton, i18n("<p>Set here the Background color of the border."));
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
          QWhatsThis::add( m_firstColorButton, i18n("<p>Set here the color of the main border."));
          QWhatsThis::add( m_secondColorButton, i18n("<p>Set here the color of the line."));
          m_firstColorButton->setColor( m_niepceBorderColor );
          m_secondColorButton->setColor( m_niepceLineColor );
          break;

       case Border::BeveledBorder:
          QWhatsThis::add( m_firstColorButton, i18n("<p>Set here the color of the upper left area."));
          QWhatsThis::add( m_secondColorButton, i18n("<p>Set here the color of the lower right area."));
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
          QWhatsThis::add( m_firstColorButton, i18n("<p>Set here the color of the first line."));
          QWhatsThis::add( m_secondColorButton, i18n("<p>Set here the color of the second line."));
          m_firstColorButton->setColor( m_decorativeFirstColor );
          m_secondColorButton->setColor( m_decorativeSecondColor );
          break;
    }

    slotEffect();
}

void BorderTool::prepareEffect()
{
    m_borderType->setEnabled(false);
    m_borderPercent->setEnabled(false);
    m_borderWidth->setEnabled(false);
    m_firstColorButton->setEnabled(false);
    m_secondColorButton->setEnabled(false);
    m_preserveAspectRatio->setEnabled(false);

    ImageIface* iface = m_previewWidget->imageIface();
    int orgWidth      = iface->originalWidth();
    int orgHeight     = iface->originalHeight();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    bool sixteenBit   = iface->previewSixteenBit();
    uchar *data       = iface->getPreviewImage();
    DImg previewImage(w, h, sixteenBit,
                      iface->previewHasAlpha(), data);
    delete [] data;

    int borderType  = m_borderType->currentItem();
    float ratio     = (float)w/(float)orgWidth;
    int borderWidth = (int)((float)m_borderWidth->value()*ratio);
    QString border  = getBorderPath( m_borderType->currentItem() );

    if (m_preserveAspectRatio->isChecked())
    {
        setFilter(dynamic_cast<DImgThreadedFilter*>(
                           new Border(&previewImage, this, orgWidth, orgHeight,
                                      border, borderType, m_borderPercent->value()/100.0,
                                      DColor(m_solidColor, sixteenBit),
                                      DColor(m_niepceBorderColor, sixteenBit),
                                      DColor(m_niepceLineColor, sixteenBit),
                                      DColor(m_bevelUpperLeftColor, sixteenBit),
                                      DColor(m_bevelLowerRightColor, sixteenBit),
                                      DColor(m_decorativeFirstColor, sixteenBit),
                                      DColor(m_decorativeSecondColor, sixteenBit))));
    }
    else
    {
        setFilter(dynamic_cast<DImgThreadedFilter*>(
                           new Border(&previewImage, this, orgWidth, orgHeight,
                                      border, borderType, borderWidth,
                                      (int)(20.0*ratio), (int)(20.0*ratio), 3,
                                      DColor(m_solidColor, sixteenBit),
                                      DColor(m_niepceBorderColor, sixteenBit),
                                      DColor(m_niepceLineColor, sixteenBit),
                                      DColor(m_bevelUpperLeftColor, sixteenBit),
                                      DColor(m_bevelLowerRightColor, sixteenBit),
                                      DColor(m_decorativeFirstColor, sixteenBit),
                                      DColor(m_decorativeSecondColor, sixteenBit))));
    }
}

void BorderTool::prepareFinal()
{
    m_borderType->setEnabled(false);
    m_borderPercent->setEnabled(false);
    m_borderWidth->setEnabled(false);
    m_firstColorButton->setEnabled(false);
    m_secondColorButton->setEnabled(false);

    int borderType    = m_borderType->currentItem();
    int borderWidth   = m_borderWidth->value();
    float borderRatio = m_borderPercent->value()/100.0;
    QString border    = getBorderPath( m_borderType->currentItem() );

    ImageIface iface(0, 0);
    int orgWidth    = iface.originalWidth();
    int orgHeight   = iface.originalHeight();
    bool sixteenBit = iface.previewSixteenBit();
    uchar *data     = iface.getOriginalImage();
    DImg orgImage(orgWidth, orgHeight, sixteenBit,
                           iface.originalHasAlpha(), data);
    delete [] data;

    if (m_preserveAspectRatio->isChecked())
    {
        setFilter(dynamic_cast<DImgThreadedFilter*>(
                           new Border(&orgImage, this, orgWidth, orgHeight,
                                    border, borderType, borderRatio,
                                    DColor(m_solidColor, sixteenBit),
                                    DColor(m_niepceBorderColor, sixteenBit),
                                    DColor(m_niepceLineColor, sixteenBit),
                                    DColor(m_bevelUpperLeftColor, sixteenBit),
                                    DColor(m_bevelLowerRightColor, sixteenBit),
                                    DColor(m_decorativeFirstColor, sixteenBit),
                                    DColor(m_decorativeSecondColor, sixteenBit))));
    }
    else
    {
        setFilter(dynamic_cast<DImgThreadedFilter*>(
                           new Border(&orgImage, this, orgWidth, orgHeight,
                                    border, borderType, borderWidth, 15, 15, 10,
                                    DColor(m_solidColor, sixteenBit),
                                    DColor(m_niepceBorderColor, sixteenBit),
                                    DColor(m_niepceLineColor, sixteenBit),
                                    DColor(m_bevelUpperLeftColor, sixteenBit),
                                    DColor(m_bevelLowerRightColor, sixteenBit),
                                    DColor(m_decorativeFirstColor, sixteenBit),
                                    DColor(m_decorativeSecondColor, sixteenBit))));
    }
}

void BorderTool::putPreviewData()
{
    ImageIface* iface = m_previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();

    DImg imTemp = filter()->getTargetImage().smoothScale(w, h, QSize::ScaleMin);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(),
                       filter()->getTargetImage().hasAlpha() );

    imDest.fill( DColor(m_previewWidget->paletteBackgroundColor().rgb(),
                        filter()->getTargetImage().sixteenBit()) );

    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage(imDest.bits());
    m_previewWidget->updatePreview();
}

void BorderTool::putFinalData()
{
    ImageIface iface(0, 0);

    DImg targetImage = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Add Border"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());
}

QString BorderTool::getBorderPath(int border)
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

    KGlobal::dirs()->addResourceType(pattern.ascii(), KGlobal::dirs()->kde_default("data") +
                                     "digikam/data");
    return (KGlobal::dirs()->findResourceDir(pattern.ascii(), pattern + ".png") + pattern + ".png" );
}

void BorderTool::slotPreserveAspectRatioToggled(bool b)
{
    toggleBorderSlider(b);
    slotTimer();
}

void BorderTool::toggleBorderSlider(bool b)
{
    m_borderPercent->setEnabled(b);
    m_borderWidth->setEnabled(!b);
    m_labelBorderPercent->setEnabled(b);
    m_labelBorderWidth->setEnabled(!b);
}

}  // NameSpace DigikamBorderImagesPlugin
