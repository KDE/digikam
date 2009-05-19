/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to sharp an image
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kurl.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes

#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "dimgsharpen.h"
#include "dimgunsharpmask.h"
#include "dimgrefocus.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

SharpenTool::SharpenTool(QObject* parent)
           : EditorToolThreaded(parent)
{
    setObjectName("sharpen");
    setToolName(i18n("Sharpen"));
    setToolIcon(SmallIcon("sharpenimage"));
    setToolHelp("blursharpentool.anchor");

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    QLabel *label1 = new QLabel(i18n("Method:"), m_gboxSettings->plainPage());

    m_sharpMethod = new RComboBox(m_gboxSettings->plainPage());
    m_sharpMethod->addItem(i18n("Simple sharp"));
    m_sharpMethod->addItem(i18n("Unsharp mask"));
    m_sharpMethod->addItem(i18n("Refocus"));
    m_sharpMethod->setDefaultIndex(SimpleSharp);
    m_sharpMethod->setWhatsThis( i18n("Select the sharpening method to apply to the image."));

    m_stack = new QStackedWidget(m_gboxSettings->plainPage());

    grid->addWidget(label1,                                      0, 0, 1, 1);
    grid->addWidget(m_sharpMethod,                               0, 1, 1, 1);
    grid->addWidget(new KSeparator(m_gboxSettings->plainPage()), 1, 0, 1, 2);
    grid->addWidget(m_stack,                                     2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget *simpleSharpSettings = new QWidget(m_stack);
    QGridLayout* grid1           = new QGridLayout(simpleSharpSettings);

    QLabel *label = new QLabel(i18n("Sharpness:"), simpleSharpSettings);
    m_radiusInput = new RIntNumInput(simpleSharpSettings);
    m_radiusInput->setRange(0, 100, 1);
    m_radiusInput->setSliderEnabled(true);
    m_radiusInput->setDefaultValue(0);
    m_radiusInput->setWhatsThis( i18n("A sharpness of 0 has no effect, "
                                      "1 and above determine the sharpen matrix radius "
                                      "that determines how much to sharpen the image."));

    grid1->addWidget(label,         0, 0, 1, 2);
    grid1->addWidget(m_radiusInput, 1, 0, 1, 2);
    grid1->setRowStretch(2, 10);
    grid1->setMargin(0);
    grid1->setSpacing(0);

    m_stack->insertWidget(SimpleSharp, simpleSharpSettings);

    // -------------------------------------------------------------

    QWidget *unsharpMaskSettings = new QWidget(m_stack);
    QGridLayout* grid2           = new QGridLayout(unsharpMaskSettings);

    QLabel *label2 = new QLabel(i18n("Radius:"), unsharpMaskSettings);
    m_radiusInput2 = new RIntNumInput(unsharpMaskSettings);
    m_radiusInput2->setRange(1, 120, 1);
    m_radiusInput2->setSliderEnabled(true);
    m_radiusInput2->setDefaultValue(1);
    m_radiusInput2->setWhatsThis( i18n("Radius value is the Gaussian blur matrix radius value "
                                       "used to determines how much to blur the image.") );

    QLabel *label3 = new QLabel(i18n("Amount:"), unsharpMaskSettings);
    m_amountInput  = new RDoubleNumInput(unsharpMaskSettings);
    m_amountInput->setDecimals(1);
    m_amountInput->input()->setRange(0.0, 5.0, 0.1, true);
    m_amountInput->setDefaultValue(1.0);
    m_amountInput->setWhatsThis( i18n("The value of the difference between the "
                                      "original and the blur image that is added back into the original.") );

    QLabel *label4   = new QLabel(i18n("Threshold:"), unsharpMaskSettings);
    m_thresholdInput = new RDoubleNumInput(unsharpMaskSettings);
    m_thresholdInput->setDecimals(2);
    m_thresholdInput->input()->setRange(0.0, 1.0, 0.01, true);
    m_thresholdInput->setDefaultValue(0.05);
    m_thresholdInput->setWhatsThis( i18n("The threshold, as a fraction of the maximum "
                                         "luminosity value, needed to apply the difference amount.") );

    grid2->addWidget(label2,           0, 0, 1, 2);
    grid2->addWidget(m_radiusInput2,   1, 0, 1, 2);
    grid2->addWidget(label3,           2, 0, 1, 2);
    grid2->addWidget(m_amountInput,    3, 0, 1, 2);
    grid2->addWidget(label4,           4, 0, 1, 2);
    grid2->addWidget(m_thresholdInput, 5, 0, 1, 2);
    grid2->setRowStretch(6, 10);
    grid2->setMargin(0);
    grid2->setSpacing(0);

    m_stack->insertWidget(UnsharpMask, unsharpMaskSettings);

    // -------------------------------------------------------------

    QWidget *refocusSettings = new QWidget(m_stack);
    QGridLayout* grid3       = new QGridLayout(refocusSettings);

    QLabel *label5 = new QLabel(i18n("Circular sharpness:"), refocusSettings);
    m_radius       = new RDoubleNumInput(refocusSettings);
    m_radius->setDecimals(2);
    m_radius->input()->setRange(0.0, 5.0, 0.01, true);
    m_radius->setDefaultValue(1.0);
    m_radius->setWhatsThis( i18n("This is the radius of the circular convolution. It is the most important "
                                 "parameter for using this plugin. For most images the default value of 1.0 "
                                 "should give good results. Select a higher value when your image is very blurred."));

    QLabel *label6 = new QLabel(i18n("Correlation:"), refocusSettings);
    m_correlation  = new RDoubleNumInput(refocusSettings);
    m_correlation->setDecimals(2);
    m_correlation->input()->setRange(0.0, 1.0, 0.01, true);
    m_correlation->setDefaultValue(0.5);
    m_correlation->setWhatsThis( i18n("Increasing the correlation may help to reduce artifacts. The correlation can "
                                      "range from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                      "Using a high value for the correlation will reduce the sharpening effect of the "
                                      "plugin."));

    QLabel *label7 = new QLabel(i18n("Noise filter:"), refocusSettings);
    m_noise        = new RDoubleNumInput(refocusSettings);
    m_noise->setDecimals(3);
    m_noise->input()->setRange(0.0, 1.0, 0.001, true);
    m_noise->setDefaultValue(0.03);
    m_noise->setWhatsThis( i18n("Increasing the noise filter parameter may help to reduce artifacts. The noise filter "
                                "can range from 0-1 but values higher than 0.1 are rarely helpful. When the noise filter "
                                "value is too low, e.g. 0.0 the image quality will be very poor. A useful value is 0.01. "
                                "Using a high value for the noise filter will reduce the sharpening "
                                "effect of the plugin."));

    QLabel *label8 = new QLabel(i18n("Gaussian sharpness:"), refocusSettings);
    m_gauss        = new RDoubleNumInput(refocusSettings);
    m_gauss->setDecimals(2);
    m_gauss->input()->setRange(0.0, 1.0, 0.01, true);
    m_gauss->setDefaultValue(0.0);
    m_gauss->setWhatsThis( i18n("This is the sharpness for the Gaussian convolution. Use this parameter when your "
                                "blurring is of a Gaussian type. In most cases you should set this parameter to 0, because "
                                "it causes nasty artifacts. When you use non-zero values, you will probably also have to "
                                "increase the correlation and/or noise filter parameters."));

    QLabel *label9 = new QLabel(i18n("Matrix size:"), refocusSettings);
    m_matrixSize   = new RIntNumInput(refocusSettings);
    m_matrixSize->setRange(0, DImgRefocus::maxMatrixSize(), 1);
    m_matrixSize->setSliderEnabled(true);
    m_matrixSize->setDefaultValue(5);
    m_matrixSize->setWhatsThis( i18n("This parameter determines the size of the transformation matrix. "
                                     "Increasing the matrix width may give better results, especially when you have "
                                     "chosen large values for circular or Gaussian sharpness."));

    grid3->addWidget(label5,        0, 0, 1, 2);
    grid3->addWidget(m_radius,      1, 0, 1, 2);
    grid3->addWidget(label6,        2, 0, 1, 2);
    grid3->addWidget(m_correlation, 3, 0, 1, 2);
    grid3->addWidget(label7,        4, 0, 1, 2);
    grid3->addWidget(m_noise,       5, 0, 1, 2);
    grid3->addWidget(label8,        6, 0, 1, 2);
    grid3->addWidget(m_gauss,       7, 0, 1, 2);
    grid3->addWidget(label9,        8, 0, 1, 2);
    grid3->addWidget(m_matrixSize,  9, 0, 1, 2);
    grid3->setRowStretch(10, 10);
    grid3->setMargin(0);
    grid3->setSpacing(0);

    m_stack->insertWidget(Refocus, refocusSettings);

    setToolSettings(m_gboxSettings);

    m_previewWidget = new ImagePanelWidget(470, 350, "sharpen Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(m_sharpMethod, SIGNAL(activated(int)),
            this, SLOT(slotSharpMethodActived(int)));

}

SharpenTool::~SharpenTool()
{
}

void SharpenTool::renderingFinished()
{
    switch (m_stack->indexOf(m_stack->currentWidget()))
    {
        case SimpleSharp:
        {
            m_radiusInput->setEnabled(true);
            m_gboxSettings->enableButton(EditorToolSettings::Load, false);
            m_gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
            break;
        }

        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(true);
            m_amountInput->setEnabled(true);
            m_thresholdInput->setEnabled(true);
            m_gboxSettings->enableButton(EditorToolSettings::Load, false);
            m_gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
            break;
        }

        case Refocus:
        {
            m_matrixSize->setEnabled(true);
            m_radius->setEnabled(true);
            m_gauss->setEnabled(true);
            m_correlation->setEnabled(true);
            m_noise->setEnabled(true);
            break;
        }
    }
}

void SharpenTool::slotSharpMethodActived(int w)
{
    m_stack->setCurrentWidget(m_stack->widget(w));
    if (w == Refocus)
    {
        m_gboxSettings->enableButton(EditorToolSettings::Load, true);
        m_gboxSettings->enableButton(EditorToolSettings::SaveAs, true);
    }
    else
    {
        m_gboxSettings->enableButton(EditorToolSettings::Load, false);
        m_gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
    }
}

void SharpenTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("sharpen Tool");

    blockWidgetSignals(true);

    m_amountInput->setValue(group.readEntry("UnsharpMaskAmountAdjustment", m_amountInput->defaultValue()));
    m_correlation->setValue(group.readEntry("RefocusCorrelationAdjustment", m_correlation->defaultValue()));
    m_gauss->setValue(group.readEntry("RefocusGaussAdjustment", m_gauss->defaultValue()));
    m_matrixSize->setValue(group.readEntry("RefocusMatrixSize", m_matrixSize->defaultValue()));
    m_noise->setValue(group.readEntry("RefocusNoiseAdjustment", m_noise->defaultValue()));
    m_radius->setValue(group.readEntry("RefocusRadiusAdjustment", m_radius->defaultValue()));
    m_radiusInput->setValue(group.readEntry("SimpleSharpRadiusAdjustment", m_radiusInput->defaultValue()));
    m_radiusInput2->setValue(group.readEntry("UnsharpMaskRadiusAdjustment", m_radiusInput2->defaultValue()));
    m_sharpMethod->setCurrentIndex(group.readEntry("SharpenMethod", m_sharpMethod->defaultIndex()));
    m_thresholdInput->setValue(group.readEntry("UnsharpMaskThresholdAdjustment", m_thresholdInput->defaultValue()));

    blockWidgetSignals(false);

    slotSharpMethodActived(m_sharpMethod->currentIndex());
}

void SharpenTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("sharpen Tool");
    group.writeEntry("SimpleSharpRadiusAdjustment", m_radiusInput->value());
    group.writeEntry("UnsharpMaskRadiusAdjustment", m_radiusInput2->value());
    group.writeEntry("UnsharpMaskAmountAdjustment", m_amountInput->value());
    group.writeEntry("UnsharpMaskThresholdAdjustment", m_thresholdInput->value());
    group.writeEntry("RefocusMatrixSize", m_matrixSize->value());
    group.writeEntry("RefocusRadiusAdjustment", m_radius->value());
    group.writeEntry("RefocusGaussAdjustment", m_gauss->value());
    group.writeEntry("RefocusCorrelationAdjustment", m_correlation->value());
    group.writeEntry("RefocusNoiseAdjustment", m_noise->value());
    group.writeEntry("SharpenMethod", m_sharpMethod->currentIndex());
    m_previewWidget->writeSettings();
    config->sync();
}

void SharpenTool::slotResetSettings()
{
    blockWidgetSignals(true);

    switch (m_stack->indexOf(m_stack->currentWidget()))
    {
        case SimpleSharp:
            m_radiusInput->slotReset();
            break;
        case UnsharpMask:
            m_radiusInput2->slotReset();
            m_amountInput->slotReset();
            m_thresholdInput->slotReset();
            break;
        case Refocus:
            m_matrixSize->slotReset();
            m_radius->slotReset();
            m_gauss->slotReset();
            m_correlation->slotReset();
            m_noise->slotReset();
            break;
    }

    blockWidgetSignals(false);
}

void SharpenTool::prepareEffect()
{
    switch (m_stack->indexOf(m_stack->currentWidget()))
    {
        case SimpleSharp:
        {
            m_radiusInput->setEnabled(false);

            DImg img      = m_previewWidget->getOriginalRegionImage();
            double radius = m_radiusInput->value()/10.0;
            double sigma;

            if (radius < 1.0) sigma = radius;
            else sigma = sqrt(radius);

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgSharpen(&img, this, radius, sigma)));
            break;
        }

        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(false);
            m_amountInput->setEnabled(false);
            m_thresholdInput->setEnabled(false);

            DImg img  = m_previewWidget->getOriginalRegionImage();
            int    r  = m_radiusInput2->value();
            double a  = m_amountInput->value();
            double th = m_thresholdInput->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgUnsharpMask(&img, this, r, a, th)));
            break;
        }

        case Refocus:
        {
            m_matrixSize->setEnabled(false);
            m_radius->setEnabled(false);
            m_gauss->setEnabled(false);
            m_correlation->setEnabled(false);
            m_noise->setEnabled(false);

            DImg   img    = m_previewWidget->getOriginalRegionImage();
            int    ms     = m_matrixSize->value();
            double r      = m_radius->value();
            double g      = m_gauss->value();
            double c      = m_correlation->value();
            double n      = m_noise->value();

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

    switch (m_stack->indexOf(m_stack->currentWidget()))
    {
        case SimpleSharp:
        {
            m_radiusInput->setEnabled(false);

            double radius = m_radiusInput->value()/10.0;
            double sigma;

            if (radius < 1.0) sigma = radius;
            else sigma = sqrt(radius);

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgSharpen(&orgImage, this, radius, sigma)));
            break;
        }

        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(false);
            m_amountInput->setEnabled(false);
            m_thresholdInput->setEnabled(false);

            int    r  = m_radiusInput2->value();
            double a  = m_amountInput->value();
            double th = m_thresholdInput->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgUnsharpMask(&orgImage, this, r, a, th)));
            break;
        }

        case Refocus:
        {
            m_matrixSize->setEnabled(false);
            m_radius->setEnabled(false);
            m_gauss->setEnabled(false);
            m_correlation->setEnabled(false);
            m_noise->setEnabled(false);

            int    ms   = m_matrixSize->value();
            double r    = m_radius->value();
            double g    = m_gauss->value();
            double c    = m_correlation->value();
            double n    = m_noise->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgRefocus(&orgImage, this, ms, r, g, c, n)));
            break;
        }
    }
}

void SharpenTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    m_previewWidget->setPreviewImage(imDest);
}

void SharpenTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg imDest = filter()->getTargetImage();

    switch (m_stack->indexOf(m_stack->currentWidget()))
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
        m_matrixSize->setValue( stream.readLine().toInt() );
        m_radius->setValue( stream.readLine().toDouble() );
        m_gauss->setValue( stream.readLine().toDouble() );
        m_correlation->setValue( stream.readLine().toDouble() );
        m_noise->setValue( stream.readLine().toDouble() );
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
        stream << m_matrixSize->value() << "\n";
        stream << m_radius->value() << "\n";
        stream << m_gauss->value() << "\n";
        stream << m_correlation->value() << "\n";
        stream << m_noise->value() << "\n";
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Refocus text file."));

    file.close();
}

void SharpenTool::blockWidgetSignals(bool b)
{
    m_amountInput->blockSignals(b);
    m_correlation->blockSignals(b);
    m_gauss->blockSignals(b);
    m_matrixSize->blockSignals(b);
    m_noise->blockSignals(b);
    m_radius->blockSignals(b);
    m_radiusInput->blockSignals(b);
    m_radiusInput2->blockSignals(b);
    m_sharpMethod->blockSignals(b);
    m_thresholdInput->blockSignals(b);
}

}  // namespace DigikamImagesPluginCore
