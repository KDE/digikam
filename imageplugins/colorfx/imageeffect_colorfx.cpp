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
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QGroupBox>
#include <QButtonGroup>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>

// KDE includes.

#include <kconfig.h>
#include <knuminput.h>
#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kvbox.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "imageeffect_colorfx.h"
#include "imageeffect_colorfx.moc"

namespace DigikamColorFXImagesPlugin
{

ImageEffect_ColorFX::ImageEffect_ColorFX(QWidget* parent)
                    : Digikam::ImageDlgBase(parent,
                      i18n("Apply Color Special Effects to Photograph"),
                      "coloreffect", false, false)
{
    m_destinationPreviewData = 0;

    // About data and help button.

    KAboutData *about = new KAboutData("digikam", 0,
                            ki18n("Color Effects"),
                            digiKamVersion().toAscii(),
                            ki18n("A digiKam plugin to apply special color effects to an image."),
                            KAboutData::License_GPL,
                            ki18n("(c) 2004-2005, Renchi Raju\n(c) 2006-2007, Gilles Caulier"),
                            KLocalizedString(),
                            Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Renchi Raju"), ki18n("Original Author"),
                     "renchi@pooh.tam.uiuc.edu");

    about->addAuthor(ki18n("Caulier Gilles"), ki18n("Maintainer"),
                     "caulier dot gilles at gmail dot com");

    setAboutData(about);

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget("coloreffect Tool Dialog", mainWidget(),
                          i18n("<p>This is the color effect preview"));

    setPreviewAreaWidget(m_previewWidget);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(mainWidget());
    QGridLayout* gridSettings = new QGridLayout(gboxSettings);

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( gboxSettings );
    m_channelCB->addItem( i18n("Luminosity") );
    m_channelCB->addItem( i18n("Red") );
    m_channelCB->addItem( i18n("Green") );
    m_channelCB->addItem( i18n("Blue") );
    m_channelCB->setWhatsThis( i18n("<p>Select the histogram channel to display here:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"));

    QWidget *scaleBox = new QWidget(gboxSettings);
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    m_scaleBG         = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis( i18n("<p>Select the histogram scale here.<p>"
                                 "If the image's maximal counts are small, you can use the linear scale.<p>"
                                 "Logarithmic scale can be used when the maximal counts are big; "
                                 "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    m_scaleBG->addButton(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);

    QPushButton *logHistoButton = new QPushButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    m_scaleBG->addButton(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(linHistoButton);
    hlay->addWidget(logHistoButton);

    m_scaleBG->setExclusive(true);
    logHistoButton->setChecked(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(scaleBox);

    // -------------------------------------------------------------

    KVBox *histoBox   = new KVBox(gboxSettings);
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, histoBox, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // -------------------------------------------------------------

    m_effectTypeLabel = new QLabel(i18n("Type:"), gboxSettings);

    m_effectType = new QComboBox( gboxSettings );
    m_effectType->addItem( i18n("Solarize") );
    m_effectType->addItem( i18n("Vivid") );
    m_effectType->addItem( i18n("Neon") );
    m_effectType->addItem( i18n("Find Edges") );
    m_effectType->setWhatsThis( i18n("<p>Select the effect type to apply to the image here.<p>"
                                     "<b>Solarize</b>: simulates solarization of photograph.<p>"
                                     "<b>Vivid</b>: simulates the Velvia(tm) slide film colors.<p>"
                                     "<b>Neon</b>: coloring the edges in a photograph to "
                                     "reproduce a fluorescent light effect.<p>"
                                     "<b>Find Edges</b>: detects the edges in a photograph "
                                     "and their strength."));

    m_levelLabel = new QLabel(i18n("Level:"), gboxSettings);
    m_levelInput = new KIntNumInput(gboxSettings);
    m_levelInput->setRange(0, 100, 1);
    m_levelInput->setSliderEnabled(true);
    m_levelInput->setWhatsThis( i18n("<p>Set here the level of the effect."));

    m_iterationLabel = new QLabel(i18n("Iteration:"), gboxSettings);
    m_iterationInput = new KIntNumInput(gboxSettings);
    m_iterationInput->setRange(0, 100, 1);
    m_iterationInput->setSliderEnabled(true);
    m_iterationInput->setWhatsThis( i18n("<p>This value controls the number of iterations "
                                         "to use with the Neon and Find Edges effects."));

    gridSettings->addLayout(l1, 0, 0, 1, 5 );
    gridSettings->addWidget(histoBox, 1, 0, 2, 5 );
    gridSettings->addWidget(m_effectTypeLabel, 3, 0, 1, 5 );
    gridSettings->addWidget(m_effectType, 4, 0, 1, 5 );
    gridSettings->addWidget(m_levelLabel, 5, 0, 1, 5 );
    gridSettings->addWidget(m_levelInput, 6, 0, 1, 5 );
    gridSettings->addWidget(m_iterationLabel, 7, 0, 1, 5 );
    gridSettings->addWidget(m_iterationInput, 8, 0, 1, 5 );
    gridSettings->setRowStretch(9, 10);
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_levelInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_iterationInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(m_effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));
}

ImageEffect_ColorFX::~ImageEffect_ColorFX()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    delete m_previewWidget;
}

void ImageEffect_ColorFX::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("coloreffect Tool Dialog");
    m_effectType->setCurrentIndex(group.readEntry("EffectType", (int)ColorFX));
    m_levelInput->setValue(group.readEntry("LevelAjustment", 0));
    m_iterationInput->setValue(group.readEntry("IterationAjustment", 3));
    slotEffectTypeChanged(m_effectType->currentIndex());  //check for enable/disable of iteration
}

void ImageEffect_ColorFX::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("coloreffect Tool Dialog");
    group.writeEntry("EffectType", m_effectType->currentIndex());
    group.writeEntry("LevelAjustment", m_levelInput->value());
    group.writeEntry("IterationAjustment", m_iterationInput->value());
    group.sync();
}

void ImageEffect_ColorFX::resetValues()
{
    m_levelInput->setValue(0);
}

void ImageEffect_ColorFX::slotChannelChanged(int channel)
{
    switch(channel)
    {
        case LuminosityChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            break;

        case RedChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            break;

        case GreenChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            break;

        case BlueChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            break;
    }

    m_histogramWidget->repaint();
}

void ImageEffect_ColorFX::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void ImageEffect_ColorFX::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_ColorFX::slotEffectTypeChanged(int type)
{
    m_levelInput->setEnabled(true);
    m_levelLabel->setEnabled(true);

    m_levelInput->blockSignals(true);
    m_iterationInput->blockSignals(true);
    m_levelInput->setRange(0, 100, 1);
    m_levelInput->setSliderEnabled(true);
    m_levelInput->setValue(25);

    switch (type)
       {
       case ColorFX:
          m_levelInput->setRange(0, 100, 1);
          m_levelInput->setSliderEnabled(true);
          m_levelInput->setValue(0);
          m_iterationInput->setEnabled(false);
          m_iterationLabel->setEnabled(false);
          break;

       case Vivid:
          m_levelInput->setRange(0, 50, 1);
          m_levelInput->setSliderEnabled(true);
          m_levelInput->setValue(5);
          m_iterationInput->setEnabled(false);
          m_iterationLabel->setEnabled(false);
          break;

       case Neon:
       case FindEdges:
          m_levelInput->setRange(0, 5, 1);
          m_levelInput->setSliderEnabled(true);
          m_levelInput->setValue(3);
          m_iterationInput->setEnabled(true);
          m_iterationLabel->setEnabled(true);
          m_iterationInput->setRange(0, 5, 1);
          m_iterationInput->setSliderEnabled(true);
          m_iterationInput->setValue(2);
          break;
       }

    m_levelInput->blockSignals(false);
    m_iterationInput->blockSignals(false);

    slotEffect();
}

void ImageEffect_ColorFX::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    Digikam::ImageIface* iface      = m_previewWidget->imageIface();
    uchar *m_destinationPreviewData = iface->getPreviewImage();
    int w                           = iface->previewWidth();
    int h                           = iface->previewHeight();
    bool sb                         = iface->previewSixteenBit();

    colorEffect(m_destinationPreviewData, w, h, sb);

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.

    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void ImageEffect_ColorFX::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    if (data)
    {
        colorEffect(data, w, h, sb);
        QString name;

        switch (m_effectType->currentIndex())
        {
            case ColorFX:
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
    accept();
}

void ImageEffect_ColorFX::colorEffect(uchar *data, int w, int h, bool sb)
{
    switch (m_effectType->currentIndex())
    {
        case ColorFX:
            solarize(m_levelInput->value(), data, w, h, sb);
            break;

        case Vivid:
            vivid(m_levelInput->value(), data, w, h, sb);
            break;

        case Neon:
            neon(data, w, h, sb, m_levelInput->value(), m_iterationInput->value());
            break;

        case FindEdges:
            findEdges(data, w, h, sb, m_levelInput->value(), m_iterationInput->value());
            break;
    }
}

void ImageEffect_ColorFX::solarize(int factor, uchar *data, int w, int h, bool sb)
{
    bool stretch = true;

    if (!sb)        // 8 bits image.
    {
        uint threshold = (uint)((100-factor)*(255+1)/100);
        threshold      = qMax((uint)1, threshold);
        uchar *ptr = data;
        uchar  a, r, g, b;

        for (int x=0 ; x < w*h ; x++)
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

        for (int x=0 ; x < w*h ; x++)
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

void ImageEffect_ColorFX::vivid(int factor, uchar *data, int w, int h, bool sb)
{
    float amount = factor/100.0;

    Digikam::DImgImageFilters filter;

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

    Digikam::ImageCurves Curves(sb);

    if (!sb)        // 8 bits image.
    {
        Curves.setCurvePoint(Digikam::ImageHistogram::ValueChannel, 0,  QPoint(0,   0));
        Curves.setCurvePoint(Digikam::ImageHistogram::ValueChannel, 5,  QPoint(63,  60));
        Curves.setCurvePoint(Digikam::ImageHistogram::ValueChannel, 10, QPoint(191, 194));
        Curves.setCurvePoint(Digikam::ImageHistogram::ValueChannel, 16, QPoint(255, 255));
    }
    else                    // 16 bits image.
    {
        Curves.setCurvePoint(Digikam::ImageHistogram::ValueChannel, 0,  QPoint(0,     0));
        Curves.setCurvePoint(Digikam::ImageHistogram::ValueChannel, 5,  QPoint(16128, 15360));
        Curves.setCurvePoint(Digikam::ImageHistogram::ValueChannel, 10, QPoint(48896, 49664));
        Curves.setCurvePoint(Digikam::ImageHistogram::ValueChannel, 16, QPoint(65535, 65535));
   }

    Curves.curvesCalculateCurve(Digikam::ImageHistogram::AlphaChannel);   // Calculate cure on all channels.
    Curves.curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);         // ... and apply it on all channels
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
void ImageEffect_ColorFX::neon(uchar *data, int w, int h, bool sb, int Intensity, int BW)
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
void ImageEffect_ColorFX::findEdges(uchar *data, int w, int h, bool sb, int Intensity, int BW)
{
    neonFindEdges(data, w, h, sb, false, Intensity, BW);
}

// Implementation of neon and FindEdges. They share 99% of their code.
void ImageEffect_ColorFX::neonFindEdges(uchar *data, int w, int h, bool sb, bool neon, int Intensity, int BW)
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

    for (int h = 0; h < Height; h++)
    {
        for (int w = 0; w < Width; w++)
        {
            ptr  = pResBits + getOffset(Width, w, h, bytesDepth);
            ptr1 = pResBits + getOffset(Width, w + Lim_Max (w, BW, Width), h, bytesDepth);
            ptr2 = pResBits + getOffset(Width, w, h + Lim_Max (h, BW, Height), bytesDepth);

            if (sixteenBit)
            {
                for (int k = 0; k <= 2; k++)
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
                        ((unsigned short *)ptr)[k] = CLAMP065535 ((int)( sqrt(color_1 + color_2) * intensityFactor ));
                    else
                        ((unsigned short *)ptr)[k] = 65535 - CLAMP065535 ((int)( sqrt(color_1 + color_2) * intensityFactor ));
                }
            }
            else
            {
                for (int k = 0; k <= 2; k++)
                {
                    colorPoint  = ptr[k];
                    colorOther1 = ptr1[k];
                    colorOther2 = ptr2[k];
                    color_1 = (colorPoint - colorOther1) * (colorPoint - colorOther1);
                    color_2 = (colorPoint - colorOther2) * (colorPoint - colorOther2);

                    if (neon)
                        ptr[k] = CLAMP0255 ((int)( sqrt(color_1 + color_2) * intensityFactor ));
                    else
                        ptr[k] = 255 - CLAMP0255 ((int)( sqrt(color_1 + color_2) * intensityFactor ));
                }
            }
        }
    }

    memcpy (data, pResBits, Width*Height*bytesDepth);
    delete [] pResBits;
}

int ImageEffect_ColorFX::getOffset(int Width, int X, int Y, int bytesDepth)
{
    return (Y * Width * bytesDepth) + (X * bytesDepth);
}

inline int ImageEffect_ColorFX::Lim_Max(int Now, int Up, int Max)
{
    --Max;
    while (Now > Max - Up) --Up;
    return (Up);
}

}  // NameSpace DigikamColorFXImagesPlugin

