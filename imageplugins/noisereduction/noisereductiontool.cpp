/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
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

#include "noisereductiontool.h"
#include "noisereductiontool.moc"

// Qt includes

#include <QCheckBox>
#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QString>
#include <QTextStream>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "daboutdata.h"
#include "rexpanderbox.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "noisereduction.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamNoiseReductionImagesPlugin
{

NoiseReductionTool::NoiseReductionTool(QObject* parent)
                  : EditorToolThreaded(parent)
{
    setObjectName("noisereduction");
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

    QGridLayout* grid  = new QGridLayout( m_gboxSettings->plainPage() );
    QWidget* firstPage = new QWidget();
    QGridLayout* grid1 = new QGridLayout(firstPage);
    QLabel *label1     = new QLabel(i18n("Radius:"), firstPage);

    m_radiusInput  = new RDoubleNumInput(firstPage);
    m_radiusInput->setDecimals(1);
    m_radiusInput->input()->setRange(0.0, 10.0, 0.1, true);
    m_radiusInput->setDefaultValue(1.0);
    m_radiusInput->setWhatsThis( i18n("<b>Radius</b>: this control selects the "
                                      "gliding window size used for the filter. Larger values do not increase "
                                      "the amount of time needed to filter each pixel in the image but "
                                      "can cause blurring. This window moves across the image, and the "
                                      "color in it is smoothed to remove imperfections. "
                                      "In any case it must be about the same size as the noise granularity "
                                      "or somewhat more. If it is set higher than necessary, then it "
                                      "can cause unwanted blur."));

    // -------------------------------------------------------------

    QLabel *label3   = new QLabel(i18n("Threshold:"), firstPage);

    m_thresholdInput = new RDoubleNumInput(firstPage);
    m_thresholdInput->setDecimals(2);
    m_thresholdInput->input()->setRange(0.0, 1.0, 0.01, true);
    m_thresholdInput->setDefaultValue(0.08);
    m_thresholdInput->setWhatsThis( i18n("<b>Threshold</b>: use the slider for coarse adjustment, "
                                         "and the spin control for fine adjustment to control edge detection sensitivity. "
                                         "This value should be set so that edges and details are clearly visible "
                                         "and noise is smoothed out. "
                                         "Adjustment must be made carefully, because the gap between \"noisy\", "
                                         "\"smooth\", and \"blur\" is very small. Adjust it as carefully as you would adjust "
                                         "the focus of a camera."));

    // -------------------------------------------------------------

    QLabel *label4 = new QLabel(i18n("Texture:"), firstPage);

    m_textureInput = new RDoubleNumInput(firstPage);
    m_textureInput->setDecimals(2);
    m_textureInput->input()->setRange(-0.99, 0.99, 0.01, true);
    m_textureInput->setDefaultValue(0.0);
    m_textureInput->setWhatsThis( i18n("<b>Texture</b>: this control sets the texture accuracy. "
                                       "This value can be used, to get more or less texture accuracy. When decreased, "
                                       "then noise and texture are blurred out, when increased then texture is "
                                       "amplified, but also noise will increase. It has almost no effect on image edges."));

    // -------------------------------------------------------------

    QLabel *label7   = new QLabel(i18n("Sharpness:"), firstPage);  // Filter setting "Lookahead".

    m_sharpnessInput = new RDoubleNumInput(firstPage);
    m_sharpnessInput->setDecimals(2);
    m_sharpnessInput->input()->setRange(0.0, 1.0, 0.1, true);
    m_sharpnessInput->setDefaultValue(0.25);
    m_sharpnessInput->setWhatsThis( i18n("<b>Sharpness</b>: "
                                         "This value improves the frequency response for the filter. "
                                         "When it is too strong then not all noise can be removed, or spike noise may appear. "
                                         "Set it near to maximum, if you want to remove very weak noise or JPEG-artifacts, "
                                         "without losing detail."));

    // -------------------------------------------------------------

    QLabel *label5   = new QLabel(i18n("Edge Lookahead:"), firstPage);     // Filter setting "Sharp".

    m_lookaheadInput = new RDoubleNumInput(firstPage);
    m_lookaheadInput->setDecimals(2);
    m_lookaheadInput->input()->setRange(0.01, 20.0, 0.01, true);
    m_lookaheadInput->setDefaultValue(2.0);
    m_lookaheadInput->setWhatsThis( i18n("<b>Edge</b>: "
                                         "This value defines the pixel distance to which the filter looks ahead for edges. "
                                         "When this value is increased, then spike noise is erased. "
                                         "You can eventually re-adjust the <b>Edge</b> filter, when you have changed this setting. "
                                         "When this value is too high, the adaptive filter can no longer accurately track "
                                         "image details, and noise or blurring can occur."));

    // -------------------------------------------------------------

    QLabel *label10 = new QLabel(i18n("Erosion:"), firstPage);

    m_phaseInput    = new RDoubleNumInput(firstPage);
    m_phaseInput->setDecimals(1);
    m_phaseInput->input()->setRange(0.5, 20.0, 0.5, true);
    m_phaseInput->setDefaultValue(1.0);
    m_phaseInput->setWhatsThis( i18n("<b>Erosion</b>: "
                                     "Use this to increase edge noise erosion and spike noise erosion "
                                     "(noise is removed by erosion)."));

    grid1->addWidget(label1,           0, 0, 1, 1);
    grid1->addWidget(m_radiusInput,    0, 1, 1, 1);
    grid1->addWidget(label3,           1, 0, 1, 1);
    grid1->addWidget(m_thresholdInput, 1, 1, 1, 1);
    grid1->addWidget(label4,           2, 0, 1, 1);
    grid1->addWidget(m_textureInput,   2, 1, 1, 1);
    grid1->addWidget(label7,           3, 0, 1, 1);
    grid1->addWidget(m_sharpnessInput, 3, 1, 1, 1);
    grid1->addWidget(label5,           4, 0, 1, 1);
    grid1->addWidget(m_lookaheadInput, 4, 1, 1, 1);
    grid1->addWidget(label10,          5, 0, 1, 1);
    grid1->addWidget(m_phaseInput,     5, 1, 1, 1);
    grid1->setMargin(m_gboxSettings->spacingHint());
    grid1->setSpacing(m_gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget();
    QGridLayout* grid2  = new QGridLayout( secondPage );

    QLabel *label2      = new QLabel(i18n("Luminance:"), secondPage);

    m_lumToleranceInput = new RDoubleNumInput(secondPage);
    m_lumToleranceInput->setDecimals(1);
    m_lumToleranceInput->input()->setRange(0.0, 1.0, 0.1, true);
    m_lumToleranceInput->setDefaultValue(1.0);
    m_lumToleranceInput->setWhatsThis( i18n("<b>Luminance</b>: this control sets the luminance tolerance of the image. "
                                            "Using either the <b>Color</b> or the <b>Luminance</b> tolerance settings "
                                            "to make an image correction is recommended, but not both at the same time. These settings "
                                            "do not influence the main smoothing process controlled by the <b>Details</b> "
                                            "settings."));

    // -------------------------------------------------------------

    QLabel *label6 = new QLabel(i18nc("color tolerance", "Color:"), secondPage);

    m_csmoothInput = new RDoubleNumInput(secondPage);
    m_csmoothInput->setDecimals(1);
    m_csmoothInput->input()->setRange(0.0, 1.0, 0.1, true);
    m_csmoothInput->setDefaultValue(1.0);
    m_csmoothInput->setWhatsThis( i18n("<b>Color</b>: this control sets the color tolerance of the image. It is "
                                       "recommended using either the <b>Color</b> or the <b>Luminance</b> tolerance "
                                       "to make image correction, not both at the same time. These settings "
                                       "do not influence the main smoothing process controlled by the <b>Details</b> "
                                       "settings."));

    // -------------------------------------------------------------

    QLabel *label8 = new QLabel(i18nc("gamma tolerance", "Gamma:"), secondPage);

    m_gammaInput   = new RDoubleNumInput(secondPage);
    m_gammaInput->setDecimals(1);
    m_gammaInput->input()->setRange(0.3, 3.0, 0.1, true);
    m_gammaInput->setDefaultValue(1.4);
    m_gammaInput->setWhatsThis( i18n("<b>Gamma</b>: this control sets the gamma tolerance of the image. This value "
                                     "can be used to increase the tolerance values for darker areas (which commonly "
                                     "are noisier). This results in more blur for shadow areas."));

    // -------------------------------------------------------------

    QLabel *label9 = new QLabel(i18n("Damping:"), secondPage);

    m_dampingInput = new RDoubleNumInput(secondPage);
    m_dampingInput->setDecimals(1);
    m_dampingInput->input()->setRange(0.5, 20.0, 0.5, true);
    m_dampingInput->setDefaultValue(5.0);
    m_dampingInput->setWhatsThis( i18n("<b>Damping</b>: this control sets the phase-jitter damping adjustment. "
                                       "This value defines how fast the adaptive filter-radius reacts to luminance "
                                       "variations. If increased, then edges appear smoother; if too high, then blur "
                                       "may occur. If at minimum, then noise and phase jitter at the edges can occur. It "
                                       "can suppress spike noise when increased, and this is the preferred method to "
                                       "remove it."));

    grid2->addWidget(label2,              0, 0, 1, 1);
    grid2->addWidget(m_lumToleranceInput, 0, 1, 1, 1);
    grid2->addWidget(label6,              1, 0, 1, 1);
    grid2->addWidget(m_csmoothInput,      1, 1, 1, 1);
    grid2->addWidget(label8,              2, 0, 1, 1);
    grid2->addWidget(m_gammaInput,        2, 1, 1, 1);
    grid2->addWidget(label9,              3, 0, 1, 1);
    grid2->addWidget(m_dampingInput,      3, 1, 1, 1);
    grid2->setColumnStretch(1, 10);
    grid2->setRowStretch(4, 10);
    grid2->setMargin(m_gboxSettings->spacingHint());
    grid2->setSpacing(m_gboxSettings->spacingHint());

    m_expanderBox = new RExpanderBox;
    m_expanderBox->addItem(firstPage, SmallIcon("noisereduction"), i18n("Details"),
                           QString("DetailsContainer"), true);
    m_expanderBox->addItem(secondPage, SmallIcon("noisereduction"), i18n("Advanced settings"),
                           QString("AdvancedSettingsContainer"), true);
    m_expanderBox->addStretch();

    grid->addWidget(m_expanderBox, 0, 0, 1, 1);
    grid->setRowStretch(0, 10);
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
    m_expanderBox->setEnabled(true);
}

void NoiseReductionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("noisereduction Tool");

    m_expanderBox->setEnabled(false);

    m_csmoothInput->setValue(group.readEntry("CsmoothAdjustment", m_csmoothInput->defaultValue()));
    m_dampingInput->setValue(group.readEntry("DampingAdjustment", m_dampingInput->defaultValue()));
    m_gammaInput->setValue(group.readEntry("GammaAdjustment", m_gammaInput->defaultValue()));
    m_lookaheadInput->setValue(group.readEntry("LookAheadAdjustment", m_lookaheadInput->defaultValue()));
    m_lumToleranceInput->setValue(group.readEntry("LumToleranceAdjustment", m_lumToleranceInput->defaultValue()));
    m_phaseInput->setValue(group.readEntry("PhaseAdjustment", m_phaseInput->defaultValue()));
    m_radiusInput->setValue(group.readEntry("RadiusAdjustment", m_radiusInput->defaultValue()));
    m_sharpnessInput->setValue(group.readEntry("SharpnessAdjustment", m_sharpnessInput->defaultValue()));
    m_textureInput->setValue(group.readEntry("TextureAdjustment", m_textureInput->defaultValue()));
    m_thresholdInput->setValue(group.readEntry("ThresholdAdjustment", m_thresholdInput->defaultValue()));
    m_expanderBox->readSettings(group);

    m_expanderBox->setEnabled(true);
}

void NoiseReductionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("noisereduction Tool");
    group.writeEntry("RadiusAdjustment", m_radiusInput->value());
    group.writeEntry("LumToleranceAdjustment", m_lumToleranceInput->value());
    group.writeEntry("ThresholdAdjustment", m_thresholdInput->value());
    group.writeEntry("TextureAdjustment", m_textureInput->value());
    group.writeEntry("SharpnessAdjustment", m_sharpnessInput->value());
    group.writeEntry("CsmoothAdjustment", m_csmoothInput->value());
    group.writeEntry("LookAheadAdjustment", m_lookaheadInput->value());
    group.writeEntry("GammaAdjustment", m_gammaInput->value());
    group.writeEntry("DampingAdjustment", m_dampingInput->value());
    group.writeEntry("PhaseAdjustment", m_phaseInput->value());
    m_expanderBox->writeSettings(group);
    m_previewWidget->writeSettings();
    group.sync();
}

void NoiseReductionTool::slotResetSettings()
{
    m_expanderBox->setEnabled(false);

    m_csmoothInput->slotReset();
    m_dampingInput->slotReset();
    m_gammaInput->slotReset();
    m_lookaheadInput->slotReset();
    m_lumToleranceInput->slotReset();
    m_phaseInput->slotReset();
    m_radiusInput->slotReset();
    m_sharpnessInput->slotReset();
    m_textureInput->slotReset();
    m_thresholdInput->slotReset();

    m_expanderBox->setEnabled(true);
}

void NoiseReductionTool::prepareEffect()
{
    m_expanderBox->setEnabled(false);

    double r   = m_radiusInput->value();
    double l   = m_lumToleranceInput->value();
    double th  = m_thresholdInput->value();
    double tx  = m_textureInput->value();
    double s   = m_sharpnessInput->value();
    double c   = m_csmoothInput->value();
    double a   = m_lookaheadInput->value();
    double g   = m_gammaInput->value();
    double d   = m_dampingInput->value();
    double p   = m_phaseInput->value();
    DImg image = m_previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new NoiseReduction(&image, this, r, l, th, tx, s, c, a, g, d, p)));
}

void NoiseReductionTool::prepareFinal()
{
    m_expanderBox->setEnabled(false);

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
    setFilter(dynamic_cast<DImgThreadedFilter *>(new NoiseReduction(iface.getOriginalImg(), this, r, l, th, tx, s, c, a, g, d, p)));
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
    KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Noise Reduction Settings File to Load")) );
    if ( loadRestorationFile.isEmpty() )
        return;

    QFile file(loadRestorationFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Noise Reduction Configuration File" )
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph Noise Reduction settings text file.",
                                    loadRestorationFile.fileName()));
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
//         slotEffect();
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Noise Reduction text file."));
    }

    file.close();
}

void NoiseReductionTool::slotSaveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Noise Reduction Settings File to Save")) );
    if ( saveRestorationFile.isEmpty() )
        return;

    QFile file(saveRestorationFile.path());

    if ( file.open(QIODevice::WriteOnly) )
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
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Noise Reduction text file."));
    }

    file.close();
}

}  // namespace DigikamNoiseReductionImagesPlugin
