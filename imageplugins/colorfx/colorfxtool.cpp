/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin for to apply a color
 *               effect to an image.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


// #include "colorfxtool.h"
#include "colorfxtool.moc"

// Qt includes

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "daboutdata.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamColorFXImagesPlugin
{

class ColorFXToolPriv
{
public:

    ColorFXToolPriv() :
        configGroupName("coloreffect Tool"),
        configHistogramChannelEntry("Histogram Channel"),
        configHistogramScaleEntry("Histogram Scale"),
        configEffectTypeEntry("EffectType"),
        configLevelAdjustmentEntry("LevelAdjustment"),
        configIterationAdjustmentEntry("IterationAdjustment"),

        destinationPreviewData(0),
        effectTypeLabel(0),
        levelLabel(0),
        iterationLabel(0),
        effectType(0),
        levelInput(0),
        iterationInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString        configGroupName;
    const QString        configHistogramChannelEntry;
    const QString        configHistogramScaleEntry;
    const QString        configEffectTypeEntry;
    const QString        configLevelAdjustmentEntry;
    const QString        configIterationAdjustmentEntry;

    uchar*               destinationPreviewData;

    QLabel*              effectTypeLabel;
    QLabel*              levelLabel;
    QLabel*              iterationLabel;

    RComboBox*           effectType;

    RIntNumInput*        levelInput;
    RIntNumInput*        iterationInput;

    ImageWidget*         previewWidget;
    EditorToolSettings*  gboxSettings;
};

ColorFXTool::ColorFXTool(QObject* parent)
           : EditorTool(parent),
             d(new ColorFXToolPriv)
{
    setObjectName("coloreffects");
    setToolName(i18n("Color Effects"));
    setToolIcon(SmallIcon("colorfx"));

    d->destinationPreviewData = 0;

    // -------------------------------------------------------------

    d->previewWidget = new ImageWidget("coloreffects Tool", 0,
                                      i18n("This is the color effects preview"));

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);

    // -------------------------------------------------------------

    d->effectTypeLabel = new QLabel(i18n("Type:"));
    d->effectType      = new RComboBox();
    d->effectType->addItem(i18n("Solarize"));
    d->effectType->addItem(i18n("Vivid"));
    d->effectType->addItem(i18n("Neon"));
    d->effectType->addItem(i18n("Find Edges"));
    d->effectType->setDefaultIndex(Solarize);
    d->effectType->setWhatsThis(i18n("<p>Select the effect type to apply to the image here.</p>"
                                     "<p><b>Solarize</b>: simulates solarization of photograph.</p>"
                                     "<p><b>Vivid</b>: simulates the Velvia(tm) slide film colors.</p>"
                                     "<p><b>Neon</b>: coloring the edges in a photograph to "
                                     "reproduce a fluorescent light effect.</p>"
                                     "<p><b>Find Edges</b>: detects the edges in a photograph "
                                     "and their strength.</p>"));

    d->levelLabel = new QLabel(i18nc("level of the effect", "Level:"));
    d->levelInput = new RIntNumInput();
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setDefaultValue(0);
    d->levelInput->setWhatsThis( i18n("Set here the level of the effect."));

    d->iterationLabel = new QLabel(i18n("Iteration:"));
    d->iterationInput = new RIntNumInput();
    d->iterationInput->setRange(0, 100, 1);
    d->iterationInput->setSliderEnabled(true);
    d->iterationInput->setDefaultValue(0);
    d->iterationInput->setWhatsThis( i18n("This value controls the number of iterations "
                                         "to use with the Neon and Find Edges effects."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->effectTypeLabel, 0, 0, 1, 5);
    mainLayout->addWidget(d->effectType,      1, 0, 1, 5);
    mainLayout->addWidget(d->levelLabel,      2, 0, 1, 5);
    mainLayout->addWidget(d->levelInput,      3, 0, 1, 5);
    mainLayout->addWidget(d->iterationLabel,  4, 0, 1, 5);
    mainLayout->addWidget(d->iterationInput,  5, 0, 1, 5);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(d->levelInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->iterationInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));
}

ColorFXTool::~ColorFXTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
}

void ColorFXTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel(group.readEntry(d->configHistogramChannelEntry,
                        (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
                        (int)LogScaleHistogram));

    d->effectType->setCurrentIndex(group.readEntry(d->configEffectTypeEntry,       d->effectType->defaultIndex()));
    d->levelInput->setValue(group.readEntry(d->configLevelAdjustmentEntry,         d->levelInput->defaultValue()));
    d->iterationInput->setValue(group.readEntry(d->configIterationAdjustmentEntry, d->iterationInput->defaultValue()));
    slotEffectTypeChanged(d->effectType->currentIndex());  //check for enable/disable of iteration
}

void ColorFXTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry,    d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,      (int)d->gboxSettings->histogramBox()->scale());

    group.writeEntry(d->configEffectTypeEntry,          d->effectType->currentIndex());
    group.writeEntry(d->configLevelAdjustmentEntry,     d->levelInput->value());
    group.writeEntry(d->configIterationAdjustmentEntry, d->iterationInput->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void ColorFXTool::slotResetSettings()
{
    d->effectType->blockSignals(true);
    d->levelInput->blockSignals(true);
    d->iterationInput->blockSignals(true);

    d->effectType->slotReset();
    d->levelInput->slotReset();
    d->iterationInput->slotReset();

    d->effectType->blockSignals(false);
    d->levelInput->blockSignals(false);
    d->iterationInput->blockSignals(false);

    slotEffect();
}

void ColorFXTool::slotColorSelectedFromTarget( const DColor& color )
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void ColorFXTool::slotEffectTypeChanged(int type)
{
    d->levelInput->setEnabled(true);
    d->levelLabel->setEnabled(true);

    d->levelInput->blockSignals(true);
    d->iterationInput->blockSignals(true);
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setValue(25);

    switch (type)
    {
        case Solarize:
            d->levelInput->setRange(0, 100, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(0);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case Vivid:
            d->levelInput->setRange(0, 50, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(5);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case Neon:
        case FindEdges:
            d->levelInput->setRange(0, 5, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(3);
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            d->iterationInput->setRange(0, 5, 1);
            d->iterationInput->setSliderEnabled(true);
            d->iterationInput->setValue(2);
            break;
    }

    d->levelInput->blockSignals(false);
    d->iterationInput->blockSignals(false);

    slotEffect();
}

void ColorFXTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    ImageIface* iface         = d->previewWidget->imageIface();
    d->destinationPreviewData = iface->getPreviewImage();
    int w                     = iface->previewWidth();
    int h                     = iface->previewHeight();
    bool sb                   = iface->previewSixteenBit();

    colorEffect(d->destinationPreviewData, w, h, sb);

    iface->putPreviewImage(d->destinationPreviewData);
    d->previewWidget->updatePreview();

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ColorFXTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data       = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool sb           = iface->originalSixteenBit();

    if (data)
    {
        colorEffect(data, w, h, sb);
        QString name;

        switch (d->effectType->currentIndex())
        {
            case Solarize:
                name = i18n("ColorFX");
                break;

            case Vivid:
                name = i18n("Vivid");
                break;

            case Neon:
                name = i18n("Neon");
                break;

            case FindEdges:
                name = i18n("Find Edges");
                break;
        }

        iface->putOriginalImage(name, data);
        delete [] data;
    }

    kapp->restoreOverrideCursor();
}

void ColorFXTool::colorEffect(uchar *data, int w, int h, bool sb)
{
    switch (d->effectType->currentIndex())
    {
        case Solarize:
            solarize(d->levelInput->value(), data, w, h, sb);
            break;

        case Vivid:
            vivid(d->levelInput->value(), data, w, h, sb);
            break;

        case Neon:
            neon(data, w, h, sb, d->levelInput->value(), d->iterationInput->value());
            break;

        case FindEdges:
            findEdges(data, w, h, sb, d->levelInput->value(), d->iterationInput->value());
            break;
    }
}

void ColorFXTool::solarize(int factor, uchar *data, int w, int h, bool sb)
{
    bool stretch = true;

    if (!sb)        // 8 bits image.
    {
        uint threshold = (uint)((100-factor)*(255+1)/100);
        threshold      = qMax((uint)1, threshold);
        uchar *ptr = data;
        uchar  a, r, g, b;

        for (int x=0 ; x < w*h ; ++x)
        {
            b = ptr[0];
            g = ptr[1];
            r = ptr[2];
            a = ptr[3];

            if (stretch)
            {
                r = (r > threshold) ? (255-r)*255/(255-threshold) : r*255/threshold;
                g = (g > threshold) ? (255-g)*255/(255-threshold) : g*255/threshold;
                b = (b > threshold) ? (255-b)*255/(255-threshold) : b*255/threshold;
            }
            else
            {
                if (r > threshold)
                    r = (255-r);
                if (g > threshold)
                    g = (255-g);
                if (b > threshold)
                    b = (255-b);
            }

            ptr[0] = b;
            ptr[1] = g;
            ptr[2] = r;
            ptr[3] = a;

            ptr += 4;
        }
    }
    else                            // 16 bits image.
    {
        uint threshold = (uint)((100-factor)*(65535+1)/100);
        threshold      = qMax((uint)1, threshold);
        unsigned short *ptr = (unsigned short *)data;
        unsigned short  a, r, g, b;

        for (int x=0 ; x < w*h ; ++x)
        {
            b = ptr[0];
            g = ptr[1];
            r = ptr[2];
            a = ptr[3];

            if (stretch)
            {
                r = (r > threshold) ? (65535-r)*65535/(65535-threshold) : r*65535/threshold;
                g = (g > threshold) ? (65535-g)*65535/(65535-threshold) : g*65535/threshold;
                b = (b > threshold) ? (65535-b)*65535/(65535-threshold) : b*65535/threshold;
            }
            else
            {
                if (r > threshold)
                    r = (65535-r);
                if (g > threshold)
                    g = (65535-g);
                if (b > threshold)
                    b = (65535-b);
            }

            ptr[0] = b;
            ptr[1] = g;
            ptr[2] = r;
            ptr[3] = a;

            ptr += 4;
        }
    }
}

void ColorFXTool::vivid(int factor, uchar *data, int w, int h, bool sb)
{
    float amount = factor/100.0;

    DImgImageFilters filter;

    // Apply Channel Mixer adjustments.

    filter.channelMixerImage(
                             data, w, h, sb,                       // Image data.
                             true,                                 // Preserve Luminosity
                             false,                                // Disable Black & White mode.
                             1.0 + amount + amount, (-1.0)*amount, (-1.0)*amount, // Red Gains.
                             (-1.0)*amount, 1.0 + amount + amount, (-1.0)*amount, // Green Gains.
                             (-1.0)*amount, (-1.0)*amount, 1.0 + amount + amount  // Blue Gains.
                            );

    // Allocate the destination image data.

    uchar *dest = new uchar[w*h*(sb ? 8 : 4)];

    // And now apply the curve correction.

    ImageCurves Curves(sb);

    if (!sb)        // 8 bits image.
    {
        Curves.setCurvePoint(LuminosityChannel, 0,  QPoint(0,   0));
        Curves.setCurvePoint(LuminosityChannel, 5,  QPoint(63,  60));
        Curves.setCurvePoint(LuminosityChannel, 10, QPoint(191, 194));
        Curves.setCurvePoint(LuminosityChannel, 16, QPoint(255, 255));
    }
    else                    // 16 bits image.
    {
        Curves.setCurvePoint(LuminosityChannel, 0,  QPoint(0,     0));
        Curves.setCurvePoint(LuminosityChannel, 5,  QPoint(16128, 15360));
        Curves.setCurvePoint(LuminosityChannel, 10, QPoint(48896, 49664));
        Curves.setCurvePoint(LuminosityChannel, 16, QPoint(65535, 65535));
   }

    Curves.curvesCalculateCurve(AlphaChannel);   // Calculate cure on all channels.
    Curves.curvesLutSetup(AlphaChannel);         // ... and apply it on all channels
    Curves.curvesLutProcess(data, dest, w, h);

    memcpy(data, dest, w*h*(sb ? 8 : 4));
    delete [] dest;
}

/* Function to apply the Neon effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Intensity        => Intensity value
 * BW               => Border Width
 *
 * Theory           => Wow, this is a great effect, you've never seen a Neon effect
 *                     like this on PSC. Is very similar to Growing Edges (photoshop)
 *                     Some pictures will be very interesting
 */
void ColorFXTool::neon(uchar *data, int w, int h, bool sb, int Intensity, int BW)
{
    neonFindEdges(data, w, h, sb, true, Intensity, BW);
}

/* Function to apply the Find Edges effect
 *
 * data             => The image data in RGBA mode.
 * Width            => Width of image.
 * Height           => Height of image.
 * Intensity        => Intensity value
 * BW               => Border Width
 *
 * Theory           => Wow, another Photoshop filter (FindEdges). Do you understand
 *                     Neon effect ? This is the same engine, but is inversed with
 *                     255 - color.
 */
void ColorFXTool::findEdges(uchar *data, int w, int h, bool sb, int Intensity, int BW)
{
    neonFindEdges(data, w, h, sb, false, Intensity, BW);
}

// Implementation of neon and FindEdges. They share 99% of their code.
void ColorFXTool::neonFindEdges(uchar *data, int w, int h, bool sb, bool neon, int Intensity, int BW)
{
    int Width       = w;
    int Height      = h;
    bool sixteenBit = sb;
    int bytesDepth  = sb ? 8 : 4;
    uchar* pResBits = new uchar[Width*Height*bytesDepth];

    Intensity = (Intensity < 0) ? 0 : (Intensity > 5) ? 5 : Intensity;
    BW = (BW < 1) ? 1 : (BW > 5) ? 5 : BW;

    uchar *ptr, *ptr1, *ptr2;

    // these must be uint, we need full 2^32 range for 16 bit
    uint color_1, color_2, colorPoint, colorOther1, colorOther2;

    // initial copy
    memcpy (pResBits, data, Width*Height*bytesDepth);

    double intensityFactor = sqrt( 1 << Intensity );

    for (int h = 0; h < Height; ++h)
    {
        for (int w = 0; w < Width; ++w)
        {
            ptr  = pResBits + getOffset(Width, w, h, bytesDepth);
            ptr1 = pResBits + getOffset(Width, w + Lim_Max (w, BW, Width), h, bytesDepth);
            ptr2 = pResBits + getOffset(Width, w, h + Lim_Max (h, BW, Height), bytesDepth);

            if (sixteenBit)
            {
                for (int k = 0; k <= 2; ++k)
                {
                    colorPoint  = ((unsigned short *)ptr)[k];
                    colorOther1 = ((unsigned short *)ptr1)[k];
                    colorOther2 = ((unsigned short *)ptr2)[k];
                    color_1 = (colorPoint - colorOther1) * (colorPoint - colorOther1);
                    color_2 = (colorPoint - colorOther2) * (colorPoint - colorOther2);

                    // old algorithm was
                    // sqrt ((color_1 + color_2) << Intensity)
                    // As (a << I) = a * (1 << I) = a * (2^I), and we can split the square root

                    if (neon)
                        ((unsigned short *)ptr)[k] = CLAMP065535 ((int)( sqrt((double)color_1 + color_2) * intensityFactor ));
                    else
                        ((unsigned short *)ptr)[k] = 65535 - CLAMP065535 ((int)( sqrt((double)color_1 + color_2) * intensityFactor ));
                }
            }
            else
            {
                for (int k = 0; k <= 2; ++k)
                {
                    colorPoint  = ptr[k];
                    colorOther1 = ptr1[k];
                    colorOther2 = ptr2[k];
                    color_1 = (colorPoint - colorOther1) * (colorPoint - colorOther1);
                    color_2 = (colorPoint - colorOther2) * (colorPoint - colorOther2);

                    if (neon)
                        ptr[k] = CLAMP0255 ((int)( sqrt((double)color_1 + color_2) * intensityFactor ));
                    else
                        ptr[k] = 255 - CLAMP0255 ((int)( sqrt((double)color_1 + color_2) * intensityFactor ));
                }
            }
        }
    }

    memcpy (data, pResBits, Width*Height*bytesDepth);
    delete [] pResBits;
}

int ColorFXTool::getOffset(int Width, int X, int Y, int bytesDepth)
{
    return (Y * Width * bytesDepth) + (X * bytesDepth);
}

inline int ColorFXTool::Lim_Max(int Now, int Up, int Max)
{
    --Max;
    while (Now > Max - Up) --Up;
    return (Up);
}

}  // namespace DigikamColorFXImagesPlugin

