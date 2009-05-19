/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image plugin to add a border
 *               around an image.
 *
 * Copyright 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


#include "bordertool.h"
#include "bordertool.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes

#include "border.h"
#include "daboutdata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamBorderImagesPlugin
{

class BorderToolPriv
{
public:

    BorderToolPriv()
    {
         preserveAspectRatio  = 0;
         labelBackground      = 0;
         labelBorderPercent   = 0;
         labelBorderWidth     = 0;
         labelForeground      = 0;
         firstColorButton     = 0;
         secondColorButton    = 0;
         gboxSettings         = 0;
         previewWidget        = 0;
         borderType           = 0;
         borderPercent        = 0;
         borderWidth          = 0;
    }

    QCheckBox*          preserveAspectRatio;

    QColor              bevelLowerRightColor;
    QColor              bevelUpperLeftColor;
    QColor              decorativeFirstColor;
    QColor              decorativeSecondColor;
    QColor              niepceBorderColor;
    QColor              niepceLineColor;
    QColor              solidColor;

    QLabel*             labelBackground;
    QLabel*             labelBorderPercent;
    QLabel*             labelBorderWidth;
    QLabel*             labelForeground;

    KColorButton*       firstColorButton;
    KColorButton*       secondColorButton;

    EditorToolSettings* gboxSettings;
    ImageWidget*        previewWidget;

    RComboBox*          borderType;
    RIntNumInput*       borderPercent;
    RIntNumInput*       borderWidth;
};

BorderTool::BorderTool(QObject* parent)
          : EditorToolThreaded(parent),
            d(new BorderToolPriv)
{
    setObjectName("border");
    setToolName(i18n("Add Border"));
    setToolIcon(SmallIcon("bordertool"));

    d->previewWidget = new ImageWidget("bordertool Tool", 0, QString(),
                                       false, ImageGuideWidget::HVGuideMode, false);

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel);


    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());

    QLabel *label1 = new QLabel(i18n("Type:"), d->gboxSettings->plainPage());

    d->borderType  = new RComboBox( d->gboxSettings->plainPage() );
    d->borderType->addItem(i18nc("solid border type", "Solid"));
    // NOTE: Niepce is a real name. This is the first guy in the world to have built a camera.
    d->borderType->addItem("Niepce");
    d->borderType->addItem(i18nc("beveled border type", "Beveled"));
    d->borderType->addItem(i18n("Decorative Pine"));
    d->borderType->addItem(i18n("Decorative Wood"));
    d->borderType->addItem(i18n("Decorative Paper"));
    d->borderType->addItem(i18n("Decorative Parquet"));
    d->borderType->addItem(i18n("Decorative Ice"));
    d->borderType->addItem(i18n("Decorative Leaf"));
    d->borderType->addItem(i18n("Decorative Marble"));
    d->borderType->addItem(i18n("Decorative Rain"));
    d->borderType->addItem(i18n("Decorative Craters"));
    d->borderType->addItem(i18n("Decorative Dried"));
    d->borderType->addItem(i18n("Decorative Pink"));
    d->borderType->addItem(i18n("Decorative Stone"));
    d->borderType->addItem(i18n("Decorative Chalk"));
    d->borderType->addItem(i18n("Decorative Granite"));
    d->borderType->addItem(i18n("Decorative Rock"));
    d->borderType->addItem(i18n("Decorative Wall"));
    d->borderType->setDefaultIndex(Border::SolidBorder);
    d->borderType->setWhatsThis( i18n("Select the border type to add around the image here."));

    KSeparator *line1 = new KSeparator(Qt::Horizontal, d->gboxSettings->plainPage());

    // -------------------------------------------------------------------

    d->preserveAspectRatio = new QCheckBox(d->gboxSettings->plainPage());
    d->preserveAspectRatio->setText(i18n("Preserve Aspect Ratio"));
    d->preserveAspectRatio->setWhatsThis(i18n("Enable this option if you want to preserve the aspect "
                                              "ratio of image. If enabled, the border width will be "
                                              "a percentage of the image size, else the border width will be "
                                              "in pixels."));

    d->labelBorderPercent  = new QLabel(i18n("Width (%):"), d->gboxSettings->plainPage());
    d->borderPercent       = new RIntNumInput(d->gboxSettings->plainPage());
    d->borderPercent->setRange(1, 50, 1);
    d->borderPercent->setSliderEnabled(true);
    d->borderPercent->setDefaultValue(10);
    d->borderPercent->setWhatsThis( i18n("Set here the border width as a percentage of the image size."));

    d->labelBorderWidth = new QLabel(i18n("Width (pixels):"), d->gboxSettings->plainPage());
    d->borderWidth      = new RIntNumInput(d->gboxSettings->plainPage());
    d->borderWidth->setRange(1, 1000, 1);
    d->borderWidth->setSliderEnabled(true);
    d->borderWidth->setDefaultValue(100);
    d->borderWidth->setWhatsThis(i18n("Set here the border width in pixels to add around the image."));

    ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();

    if (w > h)
        d->borderWidth->setRange(1, h/2, 1);
    else
        d->borderWidth->setRange(1, w/2, 1);

    KSeparator *line2 = new KSeparator(Qt::Horizontal, d->gboxSettings->plainPage());

    // -------------------------------------------------------------------

    d->labelForeground   = new QLabel(d->gboxSettings->plainPage());
    d->firstColorButton  = new KColorButton( QColor::QColor( 192, 192, 192 ), d->gboxSettings->plainPage() );
    d->labelBackground   = new QLabel(d->gboxSettings->plainPage());
    d->secondColorButton = new KColorButton( QColor::QColor( 128, 128, 128 ), d->gboxSettings->plainPage() );

    // -------------------------------------------------------------------

    gridSettings->addWidget(label1,                  0, 0, 1, 3);
    gridSettings->addWidget(d->borderType,           1, 0, 1, 3);
    gridSettings->addWidget(line1,                   2, 0, 1, 3);
    gridSettings->addWidget(d->preserveAspectRatio,  3, 0, 1, 3);
    gridSettings->addWidget(d->labelBorderPercent,   4, 0, 1, 3);
    gridSettings->addWidget(d->borderPercent,        5, 0, 1, 3);
    gridSettings->addWidget(d->labelBorderWidth,     6, 0, 1, 3);
    gridSettings->addWidget(d->borderWidth,          7, 0, 1, 3);
    gridSettings->addWidget(line2,                   8, 0, 1, 3);
    gridSettings->addWidget(d->labelForeground,      9, 0, 1, 1);
    gridSettings->addWidget(d->firstColorButton,     9, 1, 1, 2);
    gridSettings->addWidget(d->labelBackground,     10, 0, 1, 1);
    gridSettings->addWidget(d->secondColorButton,   10, 1, 1, 2);
    gridSettings->setRowStretch(11, 10);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->preserveAspectRatio, SIGNAL(toggled(bool)),
            this, SLOT(slotPreserveAspectRatioToggled(bool)));

    connect(d->borderType, SIGNAL(activated(int)),
            this, SLOT(slotBorderTypeChanged(int)));

    connect(d->borderPercent, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->borderWidth, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->firstColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorForegroundChanged(const QColor &)));

    connect(d->secondColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotColorBackgroundChanged(const QColor &)));
}

BorderTool::~BorderTool()
{
}

void BorderTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("border Tool");

    blockWidgetSignals(true);

    d->borderType->setCurrentIndex(group.readEntry("Border Type", d->borderType->defaultIndex()));
    d->borderPercent->setValue(group.readEntry("Border Percent", d->borderPercent->defaultValue()));
    d->borderWidth->setValue(group.readEntry("Border Width", d->borderWidth->defaultValue()));
    d->preserveAspectRatio->setChecked(group.readEntry("Preserve Aspect Ratio", true));

    QColor black(0, 0, 0);
    QColor white(255, 255, 255);
    QColor gray1(192, 192, 192);
    QColor gray2(128, 128, 128);

    d->solidColor = group.readEntry("Solid Color", black);
    d->niepceBorderColor = group.readEntry("Niepce Border Color", white);
    d->niepceLineColor = group.readEntry("Niepce Line Color", black);
    d->bevelUpperLeftColor = group.readEntry("Bevel Upper Left Color", gray1);
    d->bevelLowerRightColor = group.readEntry("Bevel Lower Right Color", gray2);
    d->decorativeFirstColor = group.readEntry("Decorative First Color", black);
    d->decorativeSecondColor = group.readEntry("Decorative Second Color", black);

    blockWidgetSignals(false);

    slotBorderTypeChanged(d->borderType->currentIndex());
}

void BorderTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("border Tool");

    group.writeEntry("Border Type", d->borderType->currentIndex());
    group.writeEntry("Border Percent", d->borderPercent->value());
    group.writeEntry("Border Width", d->borderWidth->value());
    group.writeEntry("Preserve Aspect Ratio", d->preserveAspectRatio->isChecked());

    group.writeEntry("Solid Color", d->solidColor);
    group.writeEntry("Niepce Border Color", d->niepceBorderColor);
    group.writeEntry("Niepce Line Color", d->niepceLineColor);
    group.writeEntry("Bevel Upper Left Color", d->bevelUpperLeftColor);
    group.writeEntry("Bevel Lower Right Color", d->bevelLowerRightColor);
    group.writeEntry("Decorative First Color", d->decorativeFirstColor);
    group.writeEntry("Decorative Second Color", d->decorativeSecondColor);

    d->previewWidget->writeSettings();

    group.sync();
}

void BorderTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->borderType->slotReset();
    d->borderPercent->slotReset();
    d->borderWidth->slotReset();
    d->preserveAspectRatio->setChecked(true);

    d->solidColor            = QColor(0, 0, 0);
    d->niepceBorderColor     = QColor(255, 255, 255);
    d->niepceLineColor       = QColor(0, 0, 0);
    d->bevelUpperLeftColor   = QColor(192, 192, 192);
    d->bevelLowerRightColor  = QColor(128, 128, 128);
    d->decorativeFirstColor  = QColor(0, 0, 0);
    d->decorativeSecondColor = QColor(0, 0, 0);

    blockWidgetSignals(false);

    slotBorderTypeChanged(Border::SolidBorder);
}

void BorderTool::renderingFinished()
{
    d->preserveAspectRatio->setEnabled(true);
    d->borderType->setEnabled(true);
    d->borderPercent->setEnabled(true);
    d->borderWidth->setEnabled(true);
    d->firstColorButton->setEnabled(true);
    d->secondColorButton->setEnabled(true);
    toggleBorderSlider(d->preserveAspectRatio->isChecked());
}

void BorderTool::slotColorForegroundChanged(const QColor& color)
{
    switch (d->borderType->currentIndex())
    {
        case Border::SolidBorder:
            d->solidColor = color;
            break;

        case Border::NiepceBorder:
            d->niepceBorderColor = color;
            break;

        case Border::BeveledBorder:
            d->bevelUpperLeftColor = color;
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
            d->decorativeFirstColor = color;
            break;
    }

    slotEffect();
}

void BorderTool::slotColorBackgroundChanged(const QColor& color)
{
    switch (d->borderType->currentIndex())
    {
        case Border::SolidBorder:
            d->solidColor = color;
            break;

        case Border::NiepceBorder:
            d->niepceLineColor = color;
            break;

        case Border::BeveledBorder:
            d->bevelLowerRightColor = color;
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
            d->decorativeSecondColor = color;
            break;
    }

    slotEffect();
}

void BorderTool::slotBorderTypeChanged(int borderType)
{
    d->labelForeground->setText(i18nc("first color for border effect", "First:"));
    d->labelBackground->setText(i18nc("second color for border effect", "Second:"));
    d->firstColorButton->setWhatsThis(i18n("Set here the foreground color of the border."));
    d->secondColorButton->setWhatsThis(i18n("Set here the Background color of the border."));
    d->firstColorButton->setEnabled(true);
    d->secondColorButton->setEnabled(true);
    d->labelForeground->setEnabled(true);
    d->labelBackground->setEnabled(true);
    d->borderPercent->setEnabled(true);

    switch (borderType)
    {
        case Border::SolidBorder:
            d->firstColorButton->setColor(d->solidColor);
            d->secondColorButton->setEnabled(false);
            d->labelBackground->setEnabled(false);
            break;

        case Border::NiepceBorder:
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the main border."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the line."));
            d->firstColorButton->setColor(d->niepceBorderColor);
            d->secondColorButton->setColor(d->niepceLineColor);
            break;

        case Border::BeveledBorder:
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the upper left area."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the lower right area."));
            d->firstColorButton->setColor(d->bevelUpperLeftColor);
            d->secondColorButton->setColor(d->bevelLowerRightColor);
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
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the first line."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the second line."));
            d->firstColorButton->setColor(d->decorativeFirstColor);
            d->secondColorButton->setColor(d->decorativeSecondColor);
            break;
    }

    slotEffect();
}

void BorderTool::prepareEffect()
{
    d->borderType->setEnabled(false);
    d->borderPercent->setEnabled(false);
    d->borderWidth->setEnabled(false);
    d->firstColorButton->setEnabled(false);
    d->secondColorButton->setEnabled(false);
    d->preserveAspectRatio->setEnabled(false);

    ImageIface* iface = d->previewWidget->imageIface();
    int orgWidth      = iface->originalWidth();
    int orgHeight     = iface->originalHeight();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    bool sixteenBit   = iface->previewSixteenBit();
    uchar *data       = iface->getPreviewImage();
    DImg previewImage(w, h, sixteenBit,
                      iface->previewHasAlpha(), data);
    delete [] data;

    int borderType  = d->borderType->currentIndex();
    float ratio     = (float)w/(float)orgWidth;
    int borderWidth = (int)((float)d->borderWidth->value()*ratio);
    QString border  = getBorderPath( d->borderType->currentIndex() );

    if (d->preserveAspectRatio->isChecked())
    {
        setFilter(dynamic_cast<DImgThreadedFilter *>(
                  new Border(&previewImage, this, orgWidth, orgHeight,
                             border, borderType, d->borderPercent->value()/100.0,
                             DColor(d->solidColor, sixteenBit),
                             DColor(d->niepceBorderColor, sixteenBit),
                             DColor(d->niepceLineColor, sixteenBit),
                             DColor(d->bevelUpperLeftColor, sixteenBit),
                             DColor(d->bevelLowerRightColor, sixteenBit),
                             DColor(d->decorativeFirstColor, sixteenBit),
                             DColor(d->decorativeSecondColor, sixteenBit))));
    }
    else
    {
        setFilter(dynamic_cast<DImgThreadedFilter *>(
                  new Border(&previewImage, this, orgWidth, orgHeight,
                             border, borderType, borderWidth,
                             (int)(20.0*ratio), (int)(20.0*ratio), 3,
                             DColor(d->solidColor, sixteenBit),
                             DColor(d->niepceBorderColor, sixteenBit),
                             DColor(d->niepceLineColor, sixteenBit),
                             DColor(d->bevelUpperLeftColor, sixteenBit),
                             DColor(d->bevelLowerRightColor, sixteenBit),
                             DColor(d->decorativeFirstColor, sixteenBit),
                             DColor(d->decorativeSecondColor, sixteenBit))));
    }
}

void BorderTool::prepareFinal()
{
    d->borderType->setEnabled(false);
    d->borderPercent->setEnabled(false);
    d->borderWidth->setEnabled(false);
    d->firstColorButton->setEnabled(false);
    d->secondColorButton->setEnabled(false);

    int borderType    = d->borderType->currentIndex();
    int borderWidth   = d->borderWidth->value();
    float borderRatio = d->borderPercent->value()/100.0f;
    QString border    = getBorderPath( d->borderType->currentIndex() );

    ImageIface iface(0, 0);
    int orgWidth    = iface.originalWidth();
    int orgHeight   = iface.originalHeight();
    bool sixteenBit = iface.previewSixteenBit();
    uchar *data     = iface.getOriginalImage();
    DImg orgImage(orgWidth, orgHeight, sixteenBit,
                           iface.originalHasAlpha(), data);
    delete [] data;

    if (d->preserveAspectRatio->isChecked())
    {
        setFilter(dynamic_cast<DImgThreadedFilter *>(
                  new Border(&orgImage, this, orgWidth, orgHeight,
                             border, borderType, borderRatio,
                             DColor(d->solidColor, sixteenBit),
                             DColor(d->niepceBorderColor, sixteenBit),
                             DColor(d->niepceLineColor, sixteenBit),
                             DColor(d->bevelUpperLeftColor, sixteenBit),
                             DColor(d->bevelLowerRightColor, sixteenBit),
                             DColor(d->decorativeFirstColor, sixteenBit),
                             DColor(d->decorativeSecondColor, sixteenBit))));
    }
    else
    {
        setFilter(dynamic_cast<DImgThreadedFilter *>(
                  new Border(&orgImage, this, orgWidth, orgHeight,
                             border, borderType, borderWidth, 15, 15, 10,
                             DColor(d->solidColor, sixteenBit),
                             DColor(d->niepceBorderColor, sixteenBit),
                             DColor(d->niepceLineColor, sixteenBit),
                             DColor(d->bevelUpperLeftColor, sixteenBit),
                             DColor(d->bevelLowerRightColor, sixteenBit),
                             DColor(d->decorativeFirstColor, sixteenBit),
                             DColor(d->decorativeSecondColor, sixteenBit))));
    }
}

void BorderTool::putPreviewData(void)
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w = iface->previewWidth();
    int h = iface->previewHeight();

    DImg imTemp = filter()->getTargetImage().smoothScale(w, h, Qt::ScaleMin);
    DImg imDest( w, h, filter()->getTargetImage().sixteenBit(),
                       filter()->getTargetImage().hasAlpha() );

    imDest.fill(DColor(d->previewWidget->palette().color(QPalette::Background).rgb(),
                filter()->getTargetImage().sixteenBit()) );

    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage(imDest.bits());
    d->previewWidget->updatePreview();
}

void BorderTool::putFinalData(void)
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

    return (KStandardDirs::locate("data", QString("digikam/data/") + pattern + QString(".png")));
}

void BorderTool::slotPreserveAspectRatioToggled(bool b)
{
    toggleBorderSlider(b);
    slotTimer();
}

void BorderTool::toggleBorderSlider(bool b)
{
    d->borderPercent->setEnabled(b);
    d->borderWidth->setEnabled(!b);
    d->labelBorderPercent->setEnabled(b);
    d->labelBorderWidth->setEnabled(!b);
}

void BorderTool::blockWidgetSignals(bool b)
{
    d->borderType->blockSignals(b);
    d->borderPercent->blockSignals(b);
    d->borderWidth->blockSignals(b);
    d->firstColorButton->blockSignals(b);
    d->secondColorButton->blockSignals(b);
    d->preserveAspectRatio->blockSignals(b);
}

}  // namespace DigikamBorderImagesPlugin
