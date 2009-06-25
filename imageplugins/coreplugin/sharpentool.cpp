/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to sharp an image
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "sharpentool.h"
#include "sharpentool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QStackedWidget>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kurl.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimgrefocus.h"
#include "dimgsharpen.h"
#include "dimgunsharpmask.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

class SharpenToolPriv
{
public:

    SharpenToolPriv()
    {
        stack             = 0;
        sharpMethod       = 0;
        matrixSize        = 0;
        radiusInput       = 0;
        radiusInput2      = 0;
        radius            = 0;
        gauss             = 0;
        correlation       = 0;
        noise             = 0;
        amountInput       = 0;
        thresholdInput    = 0;
        previewWidget     = 0;
        gboxSettings      = 0;
    }

    QStackedWidget*      stack;

    RComboBox*           sharpMethod;

    RIntNumInput*        matrixSize;
    RIntNumInput*        radiusInput;

    RDoubleNumInput*     radiusInput2;
    RDoubleNumInput*     radius;
    RDoubleNumInput*     gauss;
    RDoubleNumInput*     correlation;
    RDoubleNumInput*     noise;
    RDoubleNumInput*     amountInput;
    RDoubleNumInput*     thresholdInput;

    ImagePanelWidget*    previewWidget;
    EditorToolSettings*  gboxSettings;
};

SharpenTool::SharpenTool(QObject* parent)
           : EditorToolThreaded(parent),
             d(new SharpenToolPriv)
{
    setObjectName("sharpen");
    setToolName(i18n("Sharpen"));
    setToolIcon(SmallIcon("sharpenimage"));
    setToolHelp("blursharpentool.anchor");

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel|
                                             EditorToolSettings::Load|
                                             EditorToolSettings::SaveAs|
                                             EditorToolSettings::Try,
                                             EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    QLabel *label1 = new QLabel(i18n("Method:"), d->gboxSettings->plainPage());
    d->sharpMethod = new RComboBox(d->gboxSettings->plainPage());
    d->sharpMethod->addItem(i18n("Simple sharp"));
    d->sharpMethod->addItem(i18n("Unsharp mask"));
    d->sharpMethod->addItem(i18n("Refocus"));
    d->sharpMethod->setDefaultIndex(SimpleSharp);
    d->sharpMethod->setWhatsThis( i18n("Select the sharpening method to apply to the image."));

    d->stack = new QStackedWidget(d->gboxSettings->plainPage());

    grid->addWidget(label1,                                       0, 0, 1, 1);
    grid->addWidget(d->sharpMethod,                               0, 1, 1, 1);
    grid->addWidget(new KSeparator(d->gboxSettings->plainPage()), 1, 0, 1, 2);
    grid->addWidget(d->stack,                                     2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget *simpleSharpSettings = new QWidget(d->stack);
    QGridLayout* grid1           = new QGridLayout(simpleSharpSettings);

    QLabel *label = new QLabel(i18n("Sharpness:"), simpleSharpSettings);
    d->radiusInput = new RIntNumInput(simpleSharpSettings);
    d->radiusInput->setRange(0, 100, 1);
    d->radiusInput->setSliderEnabled(true);
    d->radiusInput->setDefaultValue(0);
    d->radiusInput->setWhatsThis( i18n("A sharpness of 0 has no effect, "
                                       "1 and above determine the sharpen matrix radius "
                                       "that determines how much to sharpen the image."));

    grid1->addWidget(label,          0, 0, 1, 2);
    grid1->addWidget(d->radiusInput, 1, 0, 1, 2);
    grid1->setRowStretch(2, 10);
    grid1->setMargin(0);
    grid1->setSpacing(0);

    d->stack->insertWidget(SimpleSharp, simpleSharpSettings);

    // -------------------------------------------------------------

    QWidget *unsharpMaskSettings = new QWidget(d->stack);
    QGridLayout* grid2           = new QGridLayout(unsharpMaskSettings);

    QLabel *label2  = new QLabel(i18n("Radius:"), unsharpMaskSettings);
    d->radiusInput2 = new RDoubleNumInput(unsharpMaskSettings);
    d->radiusInput2->setRange(0.0, 120.0, 0.1, true);
    d->radiusInput2->setDefaultValue(1.0);
    d->radiusInput2->setWhatsThis( i18n("Radius value is the Gaussian blur matrix radius value "
                                        "used to determines how much to blur the image.") );

    QLabel *label3  = new QLabel(i18n("Amount:"), unsharpMaskSettings);
    d->amountInput  = new RDoubleNumInput(unsharpMaskSettings);
    d->amountInput->setDecimals(1);
    d->amountInput->input()->setRange(0.0, 5.0, 0.1, true);
    d->amountInput->setDefaultValue(1.0);
    d->amountInput->setWhatsThis( i18n("The value of the difference between the "
                                       "original and the blur image that is added back into the original.") );

    QLabel *label4    = new QLabel(i18n("Threshold:"), unsharpMaskSettings);
    d->thresholdInput = new RDoubleNumInput(unsharpMaskSettings);
    d->thresholdInput->setDecimals(2);
    d->thresholdInput->input()->setRange(0.0, 1.0, 0.01, true);
    d->thresholdInput->setDefaultValue(0.05);
    d->thresholdInput->setWhatsThis( i18n("The threshold, as a fraction of the maximum "
                                          "luminosity value, needed to apply the difference amount.") );

    grid2->addWidget(label2,            0, 0, 1, 2);
    grid2->addWidget(d->radiusInput2,   1, 0, 1, 2);
    grid2->addWidget(label3,            2, 0, 1, 2);
    grid2->addWidget(d->amountInput,    3, 0, 1, 2);
    grid2->addWidget(label4,            4, 0, 1, 2);
    grid2->addWidget(d->thresholdInput, 5, 0, 1, 2);
    grid2->setRowStretch(6, 10);
    grid2->setMargin(0);
    grid2->setSpacing(0);

    d->stack->insertWidget(UnsharpMask, unsharpMaskSettings);

    // -------------------------------------------------------------

    QWidget *refocusSettings = new QWidget(d->stack);
    QGridLayout* grid3       = new QGridLayout(refocusSettings);

    QLabel *label5  = new QLabel(i18n("Circular sharpness:"), refocusSettings);
    d->radius       = new RDoubleNumInput(refocusSettings);
    d->radius->setDecimals(2);
    d->radius->input()->setRange(0.0, 5.0, 0.01, true);
    d->radius->setDefaultValue(1.0);
    d->radius->setWhatsThis( i18n("This is the radius of the circular convolution. It is the most important "
                                  "parameter for using this plugin. For most images the default value of 1.0 "
                                  "should give good results. Select a higher value when your image is very blurred."));

    QLabel *label6  = new QLabel(i18n("Correlation:"), refocusSettings);
    d->correlation  = new RDoubleNumInput(refocusSettings);
    d->correlation->setDecimals(2);
    d->correlation->input()->setRange(0.0, 1.0, 0.01, true);
    d->correlation->setDefaultValue(0.5);
    d->correlation->setWhatsThis( i18n("Increasing the correlation may help to reduce artifacts. The correlation can "
                                       "range from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                       "Using a high value for the correlation will reduce the sharpening effect of the "
                                       "plugin."));

    QLabel *label7  = new QLabel(i18n("Noise filter:"), refocusSettings);
    d->noise        = new RDoubleNumInput(refocusSettings);
    d->noise->setDecimals(3);
    d->noise->input()->setRange(0.0, 1.0, 0.001, true);
    d->noise->setDefaultValue(0.03);
    d->noise->setWhatsThis( i18n("Increasing the noise filter parameter may help to reduce artifacts. The noise filter "
                                 "can range from 0-1 but values higher than 0.1 are rarely helpful. When the noise filter "
                                 "value is too low, e.g. 0.0 the image quality will be very poor. A useful value is 0.01. "
                                 "Using a high value for the noise filter will reduce the sharpening "
                                 "effect of the plugin."));

    QLabel *label8  = new QLabel(i18n("Gaussian sharpness:"), refocusSettings);
    d->gauss        = new RDoubleNumInput(refocusSettings);
    d->gauss->setDecimals(2);
    d->gauss->input()->setRange(0.0, 1.0, 0.01, true);
    d->gauss->setDefaultValue(0.0);
    d->gauss->setWhatsThis( i18n("This is the sharpness for the Gaussian convolution. Use this parameter when your "
                                 "blurring is of a Gaussian type. In most cases you should set this parameter to 0, because "
                                 "it causes nasty artifacts. When you use non-zero values, you will probably also have to "
                                 "increase the correlation and/or noise filter parameters."));

    QLabel *label9  = new QLabel(i18n("Matrix size:"), refocusSettings);
    d->matrixSize   = new RIntNumInput(refocusSettings);
    d->matrixSize->setRange(0, DImgRefocus::maxMatrixSize(), 1);
    d->matrixSize->setSliderEnabled(true);
    d->matrixSize->setDefaultValue(5);
    d->matrixSize->setWhatsThis( i18n("This parameter determines the size of the transformation matrix. "
                                      "Increasing the matrix width may give better results, especially when you have "
                                      "chosen large values for circular or Gaussian sharpness."));

    grid3->addWidget(label5,         0, 0, 1, 2);
    grid3->addWidget(d->radius,      1, 0, 1, 2);
    grid3->addWidget(label6,         2, 0, 1, 2);
    grid3->addWidget(d->correlation, 3, 0, 1, 2);
    grid3->addWidget(label7,         4, 0, 1, 2);
    grid3->addWidget(d->noise,       5, 0, 1, 2);
    grid3->addWidget(label8,         6, 0, 1, 2);
    grid3->addWidget(d->gauss,       7, 0, 1, 2);
    grid3->addWidget(label9,         8, 0, 1, 2);
    grid3->addWidget(d->matrixSize,  9, 0, 1, 2);
    grid3->setRowStretch(10, 10);
    grid3->setMargin(0);
    grid3->setSpacing(0);

    d->stack->insertWidget(Refocus, refocusSettings);

    setToolSettings(d->gboxSettings);

    d->previewWidget = new ImagePanelWidget(470, 350, "sharpen Tool", d->gboxSettings->panIconView());

    setToolView(d->previewWidget);
    init();

    // -------------------------------------------------------------

    connect(d->sharpMethod, SIGNAL(activated(int)),
            this, SLOT(slotSharpMethodActived(int)));

}

SharpenTool::~SharpenTool()
{
    delete d;
}

void SharpenTool::renderingFinished()
{
    switch (d->stack->indexOf(d->stack->currentWidget()))
    {
        case SimpleSharp:
        {
            d->radiusInput->setEnabled(true);
            d->gboxSettings->enableButton(EditorToolSettings::Load, false);
            d->gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
            break;
        }

        case UnsharpMask:
        {
            d->radiusInput2->setEnabled(true);
            d->amountInput->setEnabled(true);
            d->thresholdInput->setEnabled(true);
            d->gboxSettings->enableButton(EditorToolSettings::Load, false);
            d->gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
            break;
        }

        case Refocus:
        {
            d->matrixSize->setEnabled(true);
            d->radius->setEnabled(true);
            d->gauss->setEnabled(true);
            d->correlation->setEnabled(true);
            d->noise->setEnabled(true);
            break;
        }
    }
}

void SharpenTool::slotSharpMethodActived(int w)
{
    d->stack->setCurrentWidget(d->stack->widget(w));
    if (w == Refocus)
    {
        d->gboxSettings->enableButton(EditorToolSettings::Load, true);
        d->gboxSettings->enableButton(EditorToolSettings::SaveAs, true);
    }
    else
    {
        d->gboxSettings->enableButton(EditorToolSettings::Load, false);
        d->gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
    }
}

void SharpenTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("sharpen Tool");

    blockWidgetSignals(true);

    d->amountInput->setValue(group.readEntry("UnsharpMaskAmountAdjustment", d->amountInput->defaultValue()));
    d->correlation->setValue(group.readEntry("RefocusCorrelationAdjustment", d->correlation->defaultValue()));
    d->gauss->setValue(group.readEntry("RefocusGaussAdjustment", d->gauss->defaultValue()));
    d->matrixSize->setValue(group.readEntry("RefocusMatrixSize", d->matrixSize->defaultValue()));
    d->noise->setValue(group.readEntry("RefocusNoiseAdjustment", d->noise->defaultValue()));
    d->radius->setValue(group.readEntry("RefocusRadiusAdjustment", d->radius->defaultValue()));
    d->radiusInput->setValue(group.readEntry("SimpleSharpRadiusAdjustment", d->radiusInput->defaultValue()));
    d->radiusInput2->setValue(group.readEntry("UnsharpMaskRadiusAdjustment", d->radiusInput2->defaultValue()));
    d->sharpMethod->setCurrentIndex(group.readEntry("SharpenMethod", d->sharpMethod->defaultIndex()));
    d->thresholdInput->setValue(group.readEntry("UnsharpMaskThresholdAdjustment", d->thresholdInput->defaultValue()));

    blockWidgetSignals(false);

    slotSharpMethodActived(d->sharpMethod->currentIndex());
}

void SharpenTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("sharpen Tool");
    group.writeEntry("SimpleSharpRadiusAdjustment", d->radiusInput->value());
    group.writeEntry("UnsharpMaskRadiusAdjustment", d->radiusInput2->value());
    group.writeEntry("UnsharpMaskAmountAdjustment", d->amountInput->value());
    group.writeEntry("UnsharpMaskThresholdAdjustment", d->thresholdInput->value());
    group.writeEntry("RefocusMatrixSize", d->matrixSize->value());
    group.writeEntry("RefocusRadiusAdjustment", d->radius->value());
    group.writeEntry("RefocusGaussAdjustment", d->gauss->value());
    group.writeEntry("RefocusCorrelationAdjustment", d->correlation->value());
    group.writeEntry("RefocusNoiseAdjustment", d->noise->value());
    group.writeEntry("SharpenMethod", d->sharpMethod->currentIndex());
    d->previewWidget->writeSettings();
    config->sync();
}

void SharpenTool::slotResetSettings()
{
    blockWidgetSignals(true);

    switch (d->stack->indexOf(d->stack->currentWidget()))
    {
        case SimpleSharp:
            d->radiusInput->slotReset();
            break;
        case UnsharpMask:
            d->radiusInput2->slotReset();
            d->amountInput->slotReset();
            d->thresholdInput->slotReset();
            break;
        case Refocus:
            d->matrixSize->slotReset();
            d->radius->slotReset();
            d->gauss->slotReset();
            d->correlation->slotReset();
            d->noise->slotReset();
            break;
    }

    blockWidgetSignals(false);
}

void SharpenTool::prepareEffect()
{
    switch (d->stack->indexOf(d->stack->currentWidget()))
    {
        case SimpleSharp:
        {
            d->radiusInput->setEnabled(false);

            DImg img      = d->previewWidget->getOriginalRegionImage();
            double radius = d->radiusInput->value()/10.0;
            double sigma;

            if (radius < 1.0) sigma = radius;
            else sigma = sqrt(radius);

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgSharpen(&img, this, radius, sigma)));
            break;
        }

        case UnsharpMask:
        {
            d->radiusInput2->setEnabled(false);
            d->amountInput->setEnabled(false);
            d->thresholdInput->setEnabled(false);

            DImg img  = d->previewWidget->getOriginalRegionImage();
            int    r  = d->radiusInput2->value();
            double a  = d->amountInput->value();
            double th = d->thresholdInput->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgUnsharpMask(&img, this, r, a, th)));
            break;
        }

        case Refocus:
        {
            d->matrixSize->setEnabled(false);
            d->radius->setEnabled(false);
            d->gauss->setEnabled(false);
            d->correlation->setEnabled(false);
            d->noise->setEnabled(false);

            DImg   img    = d->previewWidget->getOriginalRegionImage();
            int    ms     = d->matrixSize->value();
            double r      = d->radius->value();
            double g      = d->gauss->value();
            double c      = d->correlation->value();
            double n      = d->noise->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgRefocus(&img, this, ms, r, g, c, n)));
            break;
        }
    }
}

void SharpenTool::prepareFinal()
{
    ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    DImg orgImage   = DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;

    switch (d->stack->indexOf(d->stack->currentWidget()))
    {
        case SimpleSharp:
        {
            d->radiusInput->setEnabled(false);

            double radius = d->radiusInput->value()/10.0;
            double sigma;

            if (radius < 1.0) sigma = radius;
            else sigma = sqrt(radius);

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgSharpen(&orgImage, this, radius, sigma)));
            break;
        }

        case UnsharpMask:
        {
            d->radiusInput2->setEnabled(false);
            d->amountInput->setEnabled(false);
            d->thresholdInput->setEnabled(false);

            int    r  = d->radiusInput2->value();
            double a  = d->amountInput->value();
            double th = d->thresholdInput->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgUnsharpMask(&orgImage, this, r, a, th)));
            break;
        }

        case Refocus:
        {
            d->matrixSize->setEnabled(false);
            d->radius->setEnabled(false);
            d->gauss->setEnabled(false);
            d->correlation->setEnabled(false);
            d->noise->setEnabled(false);

            int    ms   = d->matrixSize->value();
            double r    = d->radius->value();
            double g    = d->gauss->value();
            double c    = d->correlation->value();
            double n    = d->noise->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgRefocus(&orgImage, this, ms, r, g, c, n)));
            break;
        }
    }
}

void SharpenTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(imDest);
}

void SharpenTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg imDest = filter()->getTargetImage();

    switch (d->stack->indexOf(d->stack->currentWidget()))
    {
        case SimpleSharp:
        {
            iface.putOriginalImage(i18n("Sharpen"), imDest.bits());
            break;
        }

        case UnsharpMask:
        {
            iface.putOriginalImage(i18n("Unsharp Mask"), imDest.bits());
            break;
        }

        case Refocus:
        {
            iface.putOriginalImage(i18n("Refocus"), imDest.bits());
            break;
        }
    }
}

void SharpenTool::slotLoadSettings()
{
    KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Refocus Settings File to Load")) );
    if ( loadRestorationFile.isEmpty() )
        return;

    QFile file(loadRestorationFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Refocus Configuration File" )
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph Refocus settings text file.",
                                    loadRestorationFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);
        d->matrixSize->setValue( stream.readLine().toInt() );
        d->radius->setValue( stream.readLine().toDouble() );
        d->gauss->setValue( stream.readLine().toDouble() );
        d->correlation->setValue( stream.readLine().toDouble() );
        d->noise->setValue( stream.readLine().toDouble() );
        blockSignals(false);
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Refocus text file."));

    file.close();
}

void SharpenTool::slotSaveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Refocus Settings File to Save")) );
    if ( saveRestorationFile.isEmpty() )
        return;

    QFile file(saveRestorationFile.path());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Photograph Refocus Configuration File\n";
        stream << d->matrixSize->value() << "\n";
        stream << d->radius->value() << "\n";
        stream << d->gauss->value() << "\n";
        stream << d->correlation->value() << "\n";
        stream << d->noise->value() << "\n";
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Refocus text file."));

    file.close();
}

void SharpenTool::blockWidgetSignals(bool b)
{
    d->amountInput->blockSignals(b);
    d->correlation->blockSignals(b);
    d->gauss->blockSignals(b);
    d->matrixSize->blockSignals(b);
    d->noise->blockSignals(b);
    d->radius->blockSignals(b);
    d->radiusInput->blockSignals(b);
    d->radiusInput2->blockSignals(b);
    d->sharpMethod->blockSignals(b);
    d->thresholdInput->blockSignals(b);
}

}  // namespace DigikamImagesPluginCore
