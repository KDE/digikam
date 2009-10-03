/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-02
 * Description : image sharpen batch tool.
 *
 * Copyright (C) 2009 by Matthias Welwarsky <matze at welwarsky dot de>
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

#include "sharpen.h"
#include "sharpen.moc"

// Qt includes

#include <QWidget>
#include <QLabel>
#include <QStackedWidget>
#include <QScrollArea>

// KDE includes

#include <kvbox.h>
#include <klocale.h>

#include <kiconloader.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

#include "dimg.h"
#include "dimgimagefilters.h"
#include "dimgsharpen.h"
#include "dimgunsharpmask.h"
#include "dimgrefocus.h"

using namespace KDcrawIface;

namespace Digikam
{

Sharpen::Sharpen(QObject *parent)
       : BatchTool("Sharpen", BaseTool, parent)
{
    setToolTitle(i18n("Sharpen Image"));
    setToolDescription(i18n("A tool to sharpen images"));
    setToolIcon(KIcon(SmallIcon("sharpenimage")));
    
    /* Settings widget is a KVBox, containing the
     * sharpen method settings, ...
     */
    KVBox *vbox = new KVBox;
    new QLabel(i18n("Method:"), vbox);

    m_sharpMethod = new RComboBox(vbox);
    m_sharpMethod->addItem(i18n("Simple sharp"));
    m_sharpMethod->addItem(i18n("Unsharp mask"));
    m_sharpMethod->addItem(i18n("Refocus"));
    m_sharpMethod->setDefaultIndex(SimpleSharp);
    m_sharpMethod->setWhatsThis( i18n("Select the sharpening method to apply to the image."));

    connect(m_sharpMethod, SIGNAL(activated(int)),
            this, SLOT(slotSharpMethodChanged(int)));

    /* ... and then a widget stack, ... */
    m_stack = new QStackedWidget(vbox);
    
    /* ... which in turn contains the settings for SimpleSharp ... */
    KVBox *simpleSharpSettings = new KVBox(m_stack);
    new QLabel(i18n("Sharpness:"), simpleSharpSettings);
    m_radiusInput = new RIntNumInput(simpleSharpSettings);
    m_radiusInput->setRange(0, 100, 1);
    m_radiusInput->setSliderEnabled(true);
    m_radiusInput->setDefaultValue(0);
    m_radiusInput->setWhatsThis( i18n("A sharpness of 0 has no effect, "
                                      "1 and above determine the sharpen matrix radius "
                                      "that determines how much to sharpen the image."));
    QLabel *space = new QLabel(simpleSharpSettings);
    simpleSharpSettings->setStretchFactor(space, 10);

    connect(m_radiusInput, SIGNAL(valueChanged(int)), 
            this, SLOT(slotSettingsChanged()));
    
    m_stack->insertWidget(SimpleSharp, simpleSharpSettings);
    
    /* ... settings for UnsharpMask ... */
    KVBox *unsharpMaskSettings = new KVBox(m_stack);
    new QLabel(i18n("Radius:"), unsharpMaskSettings);
    m_radiusInput2 = new RIntNumInput(unsharpMaskSettings);
    m_radiusInput2->setRange(1, 120, 1);
    m_radiusInput2->setSliderEnabled(true);
    m_radiusInput2->setDefaultValue(1);
    m_radiusInput2->setWhatsThis( i18n("Radius value is the Gaussian blur matrix radius value "
                                       "used to determines how much to blur the image.") );

    connect(m_radiusInput2, SIGNAL(valueChanged(int)), 
            this, SLOT(slotSettingsChanged()));

    new QLabel(i18n("Amount:"), unsharpMaskSettings);
    m_amountInput  = new RDoubleNumInput(unsharpMaskSettings);
    m_amountInput->setDecimals(1);
    m_amountInput->input()->setRange(0.0, 5.0, 0.1, true);
    m_amountInput->setDefaultValue(1.0);
    m_amountInput->setWhatsThis( i18n("The value of the difference between the "
                                      "original and the blur image that is added back into the original.") );

    connect(m_amountInput, SIGNAL(valueChanged(double)), 
            this, SLOT(slotSettingsChanged()));

    new QLabel(i18n("Threshold:"), unsharpMaskSettings);
    m_thresholdInput = new RDoubleNumInput(unsharpMaskSettings);
    m_thresholdInput->setDecimals(2);
    m_thresholdInput->input()->setRange(0.0, 1.0, 0.01, true);
    m_thresholdInput->setDefaultValue(0.05);
    m_thresholdInput->setWhatsThis( i18n("The threshold, as a fraction of the maximum "
                                         "luminosity value, needed to apply the difference amount.") );

    connect(m_thresholdInput, SIGNAL(valueChanged(double)), 
            this, SLOT(slotSettingsChanged()));

    QLabel *space2 = new QLabel(unsharpMaskSettings);
    unsharpMaskSettings->setStretchFactor(space2, 10);
    
    m_stack->insertWidget(UnsharpMask, unsharpMaskSettings);

    /* ... Refocus settings ... */
    KVBox *refocusSettings = new KVBox(m_stack);
    new QLabel(i18n("Circular sharpness:"), refocusSettings);
    m_radius       = new RDoubleNumInput(refocusSettings);
    m_radius->setDecimals(2);
    m_radius->input()->setRange(0.0, 5.0, 0.01, true);
    m_radius->setDefaultValue(1.0);
    m_radius->setWhatsThis( i18n("This is the radius of the circular convolution. It is the most important "
                                 "parameter for using this plugin. For most images the default value of 1.0 "
                                 "should give good results. Select a higher value when your image is very blurred."));

    connect(m_radius, SIGNAL(valueChanged(double)), 
            this, SLOT(slotSettingsChanged()));

    new QLabel(i18n("Correlation:"), refocusSettings);
    m_correlation  = new RDoubleNumInput(refocusSettings);
    m_correlation->setDecimals(2);
    m_correlation->input()->setRange(0.0, 1.0, 0.01, true);
    m_correlation->setDefaultValue(0.5);
    m_correlation->setWhatsThis( i18n("Increasing the correlation may help to reduce artifacts. The correlation can "
                                      "range from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                      "Using a high value for the correlation will reduce the sharpening effect of the "
                                      "plugin."));

    connect(m_correlation, SIGNAL(valueChanged(double)), 
            this, SLOT(slotSettingsChanged()));

    new QLabel(i18n("Noise filter:"), refocusSettings);
    m_noise        = new RDoubleNumInput(refocusSettings);
    m_noise->setDecimals(3);
    m_noise->input()->setRange(0.0, 1.0, 0.001, true);
    m_noise->setDefaultValue(0.03);
    m_noise->setWhatsThis( i18n("Increasing the noise filter parameter may help to reduce artifacts. The noise filter "
                                "can range from 0-1 but values higher than 0.1 are rarely helpful. When the noise filter "
                                "value is too low, e.g. 0.0 the image quality will be very poor. A useful value is 0.01. "
                                "Using a high value for the noise filter will reduce the sharpening "
                                "effect of the plugin."));

    connect(m_noise, SIGNAL(valueChanged(double)), 
            this, SLOT(slotSettingsChanged()));

    new QLabel(i18n("Gaussian sharpness:"), refocusSettings);
    m_gauss        = new RDoubleNumInput(refocusSettings);
    m_gauss->setDecimals(2);
    m_gauss->input()->setRange(0.0, 1.0, 0.01, true);
    m_gauss->setDefaultValue(0.0);
    m_gauss->setWhatsThis( i18n("This is the sharpness for the Gaussian convolution. Use this parameter when your "
                                "blurring is of a Gaussian type. In most cases you should set this parameter to 0, because "
                                "it causes nasty artifacts. When you use non-zero values, you will probably also have to "
                                "increase the correlation and/or noise filter parameters."));

    connect(m_gauss, SIGNAL(valueChanged(double)), 
            this, SLOT(slotSettingsChanged()));

    new QLabel(i18n("Matrix size:"), refocusSettings);
    m_matrixSize   = new RIntNumInput(refocusSettings);
    m_matrixSize->setRange(0, DImgRefocus::maxMatrixSize(), 1);
    m_matrixSize->setSliderEnabled(true);
    m_matrixSize->setDefaultValue(5);
    m_matrixSize->setWhatsThis( i18n("This parameter determines the size of the transformation matrix. "
                                     "Increasing the matrix width may give better results, especially when you have "
                                     "chosen large values for circular or Gaussian sharpness."));

    connect(m_matrixSize, SIGNAL(valueChanged(int)), 
            this, SLOT(slotSettingsChanged()));

    m_stack->insertWidget(Refocus, refocusSettings);
        
    /* finally tell the batchtool about its settings widget */
    setSettingsWidget(vbox);

}

Sharpen::~Sharpen()
{
}

BatchToolSettings Sharpen::defaultSettings()
{
    BatchToolSettings settings;
    
    // sharpen method
    settings.insert("SharpenFilterType", SimpleSharp);
    
    // simple sharp
    settings.insert("SimpleSharpRadius", 2);
    
    // unsharp mask
    settings.insert("UnsharpMaskRadius",    (int)1);
    settings.insert("UnsharpMaskAmount",    (double)1.0);
    settings.insert("UnsharpMaskThreshold", (double)0.05);
    
    // refocus
    settings.insert("RefocusMatrixSize",  (int)5);
    settings.insert("RefocusRadius",      (double)0.9);
    settings.insert("RefocusGauss",       (double)0.0);
    settings.insert("RefocusCorrelation", (double)0.5);
    settings.insert("RefocusNoise",       (double)0.01);
    
    return settings;
}

void Sharpen::assignSettings2Widget()
{
    // sharpen method
    int w = settings()["SharpenFilterType"].toInt();
    m_sharpMethod->setCurrentIndex(w);
    m_stack->setCurrentWidget(m_stack->widget(w));

    // simple sharp
    m_radiusInput->setValue(settings()["SimpleSharpRadius"].toInt());
    
    // unsharp mask
    m_radiusInput2->setValue(settings()["UnsharpMaskRadius"].toInt());
    m_amountInput->setValue(settings()["UnsharpMaskAmount"].toDouble());
    m_thresholdInput->setValue(settings()["UnsharpMaskThreshold"].toDouble());
    
    // refocus
    m_radius->setValue(settings()["RefocusRadius"].toDouble());
    m_gauss->setValue(settings()["RefocusGauss"].toDouble());
    m_correlation->setValue(settings()["RefocusCorrelation"].toDouble());
    m_noise->setValue(settings()["RefocusNoise"].toDouble());
    m_matrixSize->setValue(settings()["RefocusMatrixSize"].toInt());
}

void Sharpen::slotSettingsChanged()
{
    BatchToolSettings settings;
    
    // sharpen type
    settings.insert("SharpenFilterType", (int)m_sharpMethod->currentIndex());
 
    // simple sharp
    settings.insert("SimpleSharpRadius", (int)m_radiusInput->value());
    
    // unsharp mask
    settings.insert("UnsharpMaskRadius",    (int)m_radiusInput2->value());
    settings.insert("UnsharpMaskAmount",    (double)m_amountInput->value());
    settings.insert("UnsharpMaskThreshold", (double)m_thresholdInput->value());
    
    // refocus
    settings.insert("RefocusRadius",      (double)m_radius->value());
    settings.insert("RefocusCorrelation", (double)m_correlation->value());
    settings.insert("RefocusNoise",       (double)m_noise->value());
    settings.insert("RefocusGauss",       (double)m_gauss->value());
    settings.insert("RefocusMatrixSize",  (double)m_matrixSize->value());

    setSettings(settings);
}

void Sharpen::slotSharpMethodChanged(int w)
{
    m_stack->setCurrentWidget(m_stack->widget(w));
    slotSettingsChanged();
}

bool Sharpen::toolOperations()
{
    if (!loadToDImg())
        return false;

    int filterType  = settings()["SharpenFilterType"].toInt();

    uint width      = image().width();
    uint height     = image().height();
    uchar *data     = image().bits();
    bool sixteenBit = image().sixteenBit();
    
    switch (filterType)
    {
        case SimpleSharp:
        {
            double radius = settings()["SimpleSharpRadius"].toInt() / 10.0;
            double sigma;

            if (radius < 1.0)
                sigma = radius;
            else
                sigma = sqrt(radius);

            DImg orgImage(width, height, sixteenBit, true, data);
            DImgSharpen *filter = new DImgSharpen(&orgImage, 0L, radius, sigma);
            filter->startFilterDirectly();
            DImg imDest = filter->getTargetImage();
            memcpy( data, imDest.bits(), imDest.numBytes() );
            delete filter;
            break;
        }

        case UnsharpMask:
        {
            int r = settings()["UnsharpMaskRadius"].toInt();
            double a = settings()["UnsharpMaskAmount"].toDouble();
            double th = settings()["UnsharpMaskThreshold"].toDouble();

            DImg orgImage(width, height, sixteenBit, true, data);
            DImgUnsharpMask *filter = new DImgUnsharpMask(&orgImage, 0L, r, a, th);
            filter->startFilterDirectly();
            DImg imDest = filter->getTargetImage();
            memcpy( data, imDest.bits(), imDest.numBytes());
            delete filter;
            break;
        }
        
        case Refocus:
        {
            int matrixSize     = settings()["RefocusMatrixSize"].toInt();
            double radius      = settings()["RefocusRadius"].toDouble();
            double gauss       = settings()["RefocusGauss"].toDouble(); 
            double correlation = settings()["RefocusCorrelation"].toDouble(); 
            double noise       = settings()["RefocusNoise"].toDouble();
                
            DImg orgImage(width, height, sixteenBit, true, data);
            DImgRefocus *filter = new DImgRefocus(&orgImage, 0L, matrixSize, 
                                                  radius, gauss, correlation, noise);
            filter->startFilterDirectly();
            DImg imDest = filter->getTargetImage();
            memcpy( data, imDest.bits(), imDest.numBytes());
            delete filter;
            break;
        }
    }

    return savefromDImg();	
}

} // namespace Digikam
