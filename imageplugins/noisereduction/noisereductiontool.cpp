/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
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

// Qt includes.

#include <qcheckbox.h>
#include <qfile.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "noisereduction.h"
#include "noisereductiontool.h"
#include "noisereductiontool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamNoiseReductionImagesPlugin
{

NoiseReductionTool::NoiseReductionTool(QObject* parent)
                  : EditorToolThreaded(parent)
{
    setName("noisereduction");
    setToolName(i18n("Noise Reduction"));
    setToolIcon(SmallIcon("noisereduction"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);

    QGridLayout* grid = new QGridLayout( m_gboxSettings->plainPage(), 1, 1);

    QTabWidget *mainTab = new QTabWidget(m_gboxSettings->plainPage());
    QWidget* firstPage  = new QWidget( mainTab );
    QGridLayout* grid1  = new QGridLayout(firstPage, 6, 1);

    QLabel *label1 = new QLabel(i18n("Radius:"), firstPage);

    m_radiusInput = new RDoubleNumInput(firstPage);
    m_radiusInput->setPrecision(1);
    m_radiusInput->setRange(0.0, 10.0, 0.1);
    m_radiusInput->setDefaultValue(1.0);
    QWhatsThis::add( m_radiusInput, i18n("<p><b>Radius</b>: this control selects the "
                                         "gliding window size used for the filter. Larger values do not increase "
                                         "the amount of time needed to filter each pixel in the image but "
                                         "can cause blurring. This window moves across the image, and the "
                                         "color in it is smoothed to remove imperfections. "
                                         "In any case it must be about the same size as the noise granularity "
                                         "or somewhat more. If it is set higher than necessary, then it "
                                         "can cause unwanted blur."));

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Threshold:"), firstPage);

    m_thresholdInput = new RDoubleNumInput(firstPage);
    m_thresholdInput->setPrecision(2);
    m_thresholdInput->setRange(0.0, 1.0, 0.01);
    m_thresholdInput->setDefaultValue(0.08);
    QWhatsThis::add( m_thresholdInput, i18n("<p><b>Threshold</b>: use the slider for coarse adjustment, "
                                            "and the spin control for fine adjustment to control edge detection sensitivity. "
                                            "This value should be set so that edges and details are clearly visible "
                                            "and noise is smoothed out. "
                                            "Adjustment must be made carefully, because the gap between \"noisy\", "
                                            "\"smooth\", and \"blur\" is very small. Adjust it as carefully as you would adjust "
                                            "the focus of a camera."));

    // -------------------------------------------------------------

    QLabel *label4 = new QLabel(i18n("Texture:"), firstPage);

    m_textureInput = new RDoubleNumInput(firstPage);
    m_textureInput->setPrecision(2);
    m_textureInput->setRange(-0.99, 0.99, 0.01);
    m_textureInput->setDefaultValue(0.0);
    QWhatsThis::add( m_textureInput, i18n("<p><b>Texture</b>: this control sets the texture accuracy. "
                                          "This value can be used, to get more or less texture accuracy. When decreased, "
                                          "then noise and texture are blurred out, when increased then texture is "
                                          "amplified, but also noise will increase. It has almost no effect on image edges."));

    // -------------------------------------------------------------

    QLabel *label7 = new QLabel(i18n("Sharpness:"), firstPage);  // Filter setting "Lookahead".

    m_sharpnessInput = new RDoubleNumInput(firstPage);
    m_sharpnessInput->setPrecision(2);
    m_sharpnessInput->setRange(0.0, 1.0, 0.1);
    m_sharpnessInput->setDefaultValue(0.25);
    QWhatsThis::add( m_sharpnessInput, i18n("<p><b>Sharpness</b>: "
                                            "This value improves the frequency response for the filter. "
                                            "When it is too strong then not all noise can be removed, or spike noise may appear. "
                                            "Set it near to maximum, if you want to remove very weak noise or JPEG-artifacts, "
                                            "without losing detail."));

    // -------------------------------------------------------------

    QLabel *label5 = new QLabel(i18n("Edge Lookahead:"), firstPage);     // Filter setting "Sharp".

    m_lookaheadInput = new RDoubleNumInput(firstPage);
    m_lookaheadInput->setPrecision(2);
    m_lookaheadInput->setRange(0.01, 20.0, 0.01);
    m_lookaheadInput->setDefaultValue(2.0);
    QWhatsThis::add( m_lookaheadInput, i18n("<p><b>Edge</b>: "
                                            "This value defines the pixel distance to which the filter looks ahead for edges. "
                                            "When this value is increased, then spike noise is erased. "
                                            "You can eventually re-adjust the <b>Edge</b> filter, when you have changed this setting. "
                                            "When this value is too high, the adaptive filter can no longer accurately track "
                                            "image details, and noise or blurring can occur."));

    // -------------------------------------------------------------

    QLabel *label10 = new QLabel(i18n("Erosion:"), firstPage);

    m_phaseInput = new RDoubleNumInput(firstPage);
    m_phaseInput->setPrecision(1);
    m_phaseInput->setRange(0.5, 20.0, 0.5);
    m_phaseInput->setDefaultValue(1.0);
    QWhatsThis::add( m_phaseInput, i18n("<p><b>Erosion</b>: "
                                        "Use this to increase edge noise erosion and spike noise erosion "
                                        "(noise is removed by erosion)."));

    grid1->addMultiCellWidget(label1,           0, 0, 0, 0);
    grid1->addMultiCellWidget(m_radiusInput,    0, 0, 1, 1);
    grid1->addMultiCellWidget(label3,           1, 1, 0, 0);
    grid1->addMultiCellWidget(m_thresholdInput, 1, 1, 1, 1);
    grid1->addMultiCellWidget(label4,           2, 2, 0, 0);
    grid1->addMultiCellWidget(m_textureInput,   2, 2, 1, 1);
    grid1->addMultiCellWidget(label7,           3, 3, 0, 0);
    grid1->addMultiCellWidget(m_sharpnessInput, 3, 3, 1, 1);
    grid1->addMultiCellWidget(label5,           4, 4, 0, 0);
    grid1->addMultiCellWidget(m_lookaheadInput, 4, 4, 1, 1);
    grid1->addMultiCellWidget(label10,          5, 5, 0, 0);
    grid1->addMultiCellWidget(m_phaseInput,     5, 5, 1, 1);
    grid1->setMargin(m_gboxSettings->spacingHint());
    grid1->setSpacing(m_gboxSettings->spacingHint());
    grid1->setColStretch(1, 10);
    grid1->setRowStretch(6, 10);

    mainTab->addTab( firstPage, i18n("Details") );

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget( mainTab );
    QGridLayout* grid2  = new QGridLayout( secondPage, 4, 1);

    QLabel *label2 = new QLabel(i18n("Luminance:"), secondPage);

    m_lumToleranceInput = new RDoubleNumInput(secondPage);
    m_lumToleranceInput->setPrecision(1);
    m_lumToleranceInput->setRange(0.0, 1.0, 0.1);
    m_lumToleranceInput->setDefaultValue(1.0);
    QWhatsThis::add( m_lumToleranceInput, i18n("<p><b>Luminance</b>: this control sets the luminance tolerance of the image."
                     "We recommend using either the <b>Color</b> or the <b>Luminance</b> tolerance settings "
                     "to make an image correction, not both at the same time. These settings "
                     "do not influence the main smoothing process controlled by the <b>Details</b> "
                     "settings."));

    // -------------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Color:"), secondPage);

    m_csmoothInput = new RDoubleNumInput(secondPage);
    m_csmoothInput->setPrecision(1);
    m_csmoothInput->setRange(0.0, 1.0, 0.1);
    m_csmoothInput->setDefaultValue(1.0);
    QWhatsThis::add( m_csmoothInput, i18n("<p><b>Color</b>: this control sets the color tolerance of the image. It is "
                                          "recommended using either the <b>Color</b> or the <b>Luminance</b> tolerance "
                                          "to make image correction, not both at the same time. These settings "
                                          "do not influence the main smoothing process controlled by the <b>Details</b> "
                                          "settings."));

    // -------------------------------------------------------------

    QLabel *label8 = new QLabel(i18n("Gamma:"), secondPage);

    m_gammaInput = new RDoubleNumInput(secondPage);
    m_gammaInput->setPrecision(1);
    m_gammaInput->setRange(0.3, 3.0, 0.1);
    m_gammaInput->setDefaultValue(1.4);
    QWhatsThis::add( m_gammaInput, i18n("<p><b>Gamma</b>: this control sets the gamma tolerance of the image. This value "
                                        "can be used to increase the tolerance values for darker areas (which commonly "
                                        "are noisier). This results in more blur for shadow areas."));

    // -------------------------------------------------------------

    QLabel *label9 = new QLabel(i18n("Damping:"), secondPage);

    m_dampingInput = new RDoubleNumInput(secondPage);
    m_dampingInput->setPrecision(1);
    m_dampingInput->setRange(0.5, 20.0, 0.5);
    m_dampingInput->setDefaultValue(5.0);
    QWhatsThis::add( m_dampingInput, i18n("<p><b>Damping</b>: this control sets the phase-jitter damping adjustment. "
                                          "This value defines how fast the adaptive filter-radius reacts to luminance "
                                          "variations. If increased, then edges appear smoother; if too high, then blur "
                                          "may occur. If at minimum, then noise and phase jitter at the edges can occur. It "
                                          "can suppress spike noise when increased, and this is the preferred method to "
                                          "remove it."));

    grid2->addMultiCellWidget(label2,              0, 0, 0, 0);
    grid2->addMultiCellWidget(m_lumToleranceInput, 0, 0, 1, 1);
    grid2->addMultiCellWidget(label6,              1, 1, 0, 0);
    grid2->addMultiCellWidget(m_csmoothInput,      1, 1, 1, 1);
    grid2->addMultiCellWidget(label8,              2, 2, 0, 0);
    grid2->addMultiCellWidget(m_gammaInput,        2, 2, 1, 1);
    grid2->addMultiCellWidget(label9,              3, 3, 0, 0);
    grid2->addMultiCellWidget(m_dampingInput,      3, 3, 1, 1);
    grid2->setMargin(m_gboxSettings->spacingHint());
    grid2->setSpacing(m_gboxSettings->spacingHint());
    grid2->setColStretch(1, 10);
    grid2->setRowStretch(4, 10);

    mainTab->addTab( secondPage, i18n("Advanced") );

    grid->addMultiCellWidget(mainTab, 0, 0, 0, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    m_previewWidget = new ImagePanelWidget(470, 350, "noisereduction Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();
}

NoiseReductionTool::~NoiseReductionTool()
{
}

void NoiseReductionTool::renderingFinished()
{
    m_radiusInput->setEnabled(true);
    m_lumToleranceInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_textureInput->setEnabled(true);
    m_sharpnessInput->setEnabled(true);
    m_csmoothInput->setEnabled(true);
    m_lookaheadInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
    m_dampingInput->setEnabled(true);
    m_phaseInput->setEnabled(true);
}

void NoiseReductionTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("noisereduction Tool");

    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);
    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);

    m_radiusInput->setValue(config->readDoubleNumEntry("RadiusAjustment", m_radiusInput->defaultValue()));
    m_lumToleranceInput->setValue(config->readDoubleNumEntry("LumToleranceAjustment", m_lumToleranceInput->defaultValue()));
    m_thresholdInput->setValue(config->readDoubleNumEntry("ThresholdAjustment", m_thresholdInput->defaultValue()));
    m_textureInput->setValue(config->readDoubleNumEntry("TextureAjustment", m_textureInput->defaultValue()));
    m_sharpnessInput->setValue(config->readDoubleNumEntry("SharpnessAjustment", m_sharpnessInput->defaultValue()));
    m_csmoothInput->setValue(config->readDoubleNumEntry("CsmoothAjustment", m_csmoothInput->defaultValue()));
    m_lookaheadInput->setValue(config->readDoubleNumEntry("LookAheadAjustment", m_lookaheadInput->defaultValue()));
    m_gammaInput->setValue(config->readDoubleNumEntry("GammaAjustment", m_gammaInput->defaultValue()));
    m_dampingInput->setValue(config->readDoubleNumEntry("DampingAjustment", m_dampingInput->defaultValue()));
    m_phaseInput->setValue(config->readDoubleNumEntry("PhaseAjustment", m_phaseInput->defaultValue()));

    m_radiusInput->setEnabled(true);
    m_lumToleranceInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_textureInput->setEnabled(true);
    m_sharpnessInput->setEnabled(true);
    m_csmoothInput->setEnabled(true);
    m_lookaheadInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
    m_dampingInput->setEnabled(true);
    m_phaseInput->setEnabled(true);
}

void NoiseReductionTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("noisereduction Tool");
    config->writeEntry("RadiusAjustment", m_radiusInput->value());
    config->writeEntry("LumToleranceAjustment", m_lumToleranceInput->value());
    config->writeEntry("ThresholdAjustment", m_thresholdInput->value());
    config->writeEntry("TextureAjustment", m_textureInput->value());
    config->writeEntry("SharpnessAjustment", m_sharpnessInput->value());
    config->writeEntry("CsmoothAjustment", m_csmoothInput->value());
    config->writeEntry("LookAheadAjustment", m_lookaheadInput->value());
    config->writeEntry("GammaAjustment", m_gammaInput->value());
    config->writeEntry("DampingAjustment", m_dampingInput->value());
    config->writeEntry("PhaseAjustment", m_phaseInput->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void NoiseReductionTool::slotResetSettings()
{
    m_radiusInput->setEnabled(true);
    m_lumToleranceInput->setEnabled(true);
    m_thresholdInput->setEnabled(true);
    m_textureInput->setEnabled(true);
    m_sharpnessInput->setEnabled(true);
    m_csmoothInput->setEnabled(true);
    m_lookaheadInput->setEnabled(true);
    m_gammaInput->setEnabled(true);
    m_dampingInput->setEnabled(true);
    m_phaseInput->setEnabled(true);

    m_radiusInput->slotReset();
    m_lumToleranceInput->slotReset();
    m_thresholdInput->slotReset();
    m_textureInput->slotReset();
    m_sharpnessInput->slotReset();
    m_csmoothInput->slotReset();
    m_lookaheadInput->slotReset();
    m_gammaInput->slotReset();
    m_dampingInput->slotReset();
    m_phaseInput->slotReset();

    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);
    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);
}

void NoiseReductionTool::prepareEffect()
{
    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);
    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);

    double r  = m_radiusInput->value();
    double l  = m_lumToleranceInput->value();
    double th = m_thresholdInput->value();
    double tx = m_textureInput->value();
    double s  = m_sharpnessInput->value();
    double c  = m_csmoothInput->value();
    double a  = m_lookaheadInput->value();
    double g  = m_gammaInput->value();
    double d  = m_dampingInput->value();
    double p  = m_phaseInput->value();

    DImg image = m_previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new NoiseReduction(&image, this, r, l, th, tx, s, c, a, g, d, p)));
}

void NoiseReductionTool::prepareFinal()
{
    m_radiusInput->setEnabled(false);
    m_lumToleranceInput->setEnabled(false);
    m_thresholdInput->setEnabled(false);
    m_textureInput->setEnabled(false);
    m_sharpnessInput->setEnabled(false);
    m_csmoothInput->setEnabled(false);
    m_lookaheadInput->setEnabled(false);
    m_gammaInput->setEnabled(false);
    m_dampingInput->setEnabled(false);
    m_phaseInput->setEnabled(false);

    double r  = m_radiusInput->value();
    double l  = m_lumToleranceInput->value();
    double th = m_thresholdInput->value();
    double tx = m_textureInput->value();
    double s  = m_sharpnessInput->value();
    double c  = m_csmoothInput->value();
    double a  = m_lookaheadInput->value();
    double g  = m_gammaInput->value();
    double d  = m_dampingInput->value();
    double p  = m_phaseInput->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new NoiseReduction(iface.getOriginalImg(), this, r, l, th, tx, s, c, a, g, d, p)));
}

void NoiseReductionTool::putPreviewData()
{
    m_previewWidget->setPreviewImage(filter()->getTargetImage());
}

void NoiseReductionTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Noise Reduction"), filter()->getTargetImage().bits());
}

void NoiseReductionTool::slotLoadSettings()
{
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Noise Reduction Settings File to Load")) );
    if ( loadRestorationFile.isEmpty() )
        return;

    QFile file(loadRestorationFile.path());

    if ( file.open(IO_ReadOnly) )
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Noise Reduction Configuration File" )
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph Noise Reduction settings text file.")
                               .arg(loadRestorationFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);
        m_radiusInput->setValue( stream.readLine().toDouble() );
        m_lumToleranceInput->setValue( stream.readLine().toDouble() );
        m_thresholdInput->setValue( stream.readLine().toDouble() );
        m_textureInput->setValue( stream.readLine().toDouble() );
        m_sharpnessInput->setValue( stream.readLine().toDouble() );
        m_csmoothInput->setValue( stream.readLine().toDouble() );
        m_lookaheadInput->setValue( stream.readLine().toDouble() );
        m_gammaInput->setValue( stream.readLine().toDouble() );
        m_dampingInput->setValue( stream.readLine().toDouble() );
        m_phaseInput->setValue( stream.readLine().toDouble() );
        blockSignals(false);
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Noise Reduction text file."));

    file.close();
}

void NoiseReductionTool::slotSaveAsSettings()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Noise Reduction Settings File to Save")) );
    if ( saveRestorationFile.isEmpty() )
        return;

    QFile file(saveRestorationFile.path());

    if ( file.open(IO_WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Photograph Noise Reduction Configuration File\n";
        stream << m_radiusInput->value() << "\n";
        stream << m_lumToleranceInput->value() << "\n";
        stream << m_thresholdInput->value() << "\n";
        stream << m_textureInput->value() << "\n";
        stream << m_sharpnessInput->value() << "\n";
        stream << m_csmoothInput->value() << "\n";
        stream << m_lookaheadInput->value() << "\n";
        stream << m_gammaInput->value() << "\n";
        stream << m_dampingInput->value() << "\n";
        stream << m_phaseInput->value() << "\n";

    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Noise Reduction text file."));

    file.close();
}

}  // NameSpace DigikamNoiseReductionImagesPlugin
