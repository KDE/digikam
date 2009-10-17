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
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "noisereduction.h"
#include "rexpanderbox.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamNoiseReductionImagesPlugin
{

class NoiseReductionToolPriv
{
public:

    NoiseReductionToolPriv() :
        configGroupName("noisereduction Tool"),
        configCsmoothAdjustmentEntry("CsmoothAdjustment"),
        configDampingAdjustmentEntry("DampingAdjustment"),
        configGammaAdjustmentEntry("GammaAdjustment"),
        configLookAheadAdjustmentEntry("LookAheadAdjustment"),
        configLumToleranceAdjustmentEntry("LumToleranceAdjustment"),
        configPhaseAdjustmentEntry("PhaseAdjustment"),
        configRadiusAdjustmentEntry("RadiusAdjustment"),
        configSharpnessAdjustmentEntry("SharpnessAdjustment"),
        configTextureAdjustmentEntry("TextureAdjustment"),
        configThresholdAdjustmentEntry("ThresholdAdjustment"),

        radiusInput(0),
        lumToleranceInput(0),
        thresholdInput(0),
        textureInput(0),
        sharpnessInput(0),
        csmoothInput(0),
        lookaheadInput(0),
        gammaInput(0),
        dampingInput(0),
        phaseInput(0),
        previewWidget(0),
        expanderBox(0),
        gboxSettings(0)
        {}

    const QString        configGroupName;
    const QString        configCsmoothAdjustmentEntry;
    const QString        configDampingAdjustmentEntry;
    const QString        configGammaAdjustmentEntry;
    const QString        configLookAheadAdjustmentEntry;
    const QString        configLumToleranceAdjustmentEntry;
    const QString        configPhaseAdjustmentEntry;
    const QString        configRadiusAdjustmentEntry;
    const QString        configSharpnessAdjustmentEntry;
    const QString        configTextureAdjustmentEntry;
    const QString        configThresholdAdjustmentEntry;

    RDoubleNumInput*     radiusInput;
    RDoubleNumInput*     lumToleranceInput;
    RDoubleNumInput*     thresholdInput;
    RDoubleNumInput*     textureInput;
    RDoubleNumInput*     sharpnessInput;

    RDoubleNumInput*     csmoothInput;
    RDoubleNumInput*     lookaheadInput;
    RDoubleNumInput*     gammaInput;
    RDoubleNumInput*     dampingInput;
    RDoubleNumInput*     phaseInput;

    ImagePanelWidget*    previewWidget;
    RExpanderBox*        expanderBox;
    EditorToolSettings*  gboxSettings;
};

NoiseReductionTool::NoiseReductionTool(QObject* parent)
                  : EditorToolThreaded(parent),
                    d(new NoiseReductionToolPriv)
{
    setObjectName("noisereduction");
    setToolName(i18n("Noise Reduction"));
    setToolIcon(SmallIcon("noisereduction"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    d->gboxSettings->setTools(EditorToolSettings::PanIcon);

    QGridLayout* grid  = new QGridLayout( d->gboxSettings->plainPage() );
    QWidget* firstPage = new QWidget();
    QGridLayout* grid1 = new QGridLayout(firstPage);

    QLabel *label1  = new QLabel(i18n("Radius:"), firstPage);
    d->radiusInput  = new RDoubleNumInput(firstPage);
    d->radiusInput->setDecimals(1);
    d->radiusInput->input()->setRange(0.0, 10.0, 0.1, true);
    d->radiusInput->setDefaultValue(1.0);
    d->radiusInput->setWhatsThis(i18n("<b>Radius</b>: this control selects the "
                                      "gliding window size used for the filter. Larger values do not increase "
                                      "the amount of time needed to filter each pixel in the image but "
                                      "can cause blurring. This window moves across the image, and the "
                                      "color in it is smoothed to remove imperfections. "
                                      "In any case it must be about the same size as the noise granularity "
                                      "or somewhat more. If it is set higher than necessary, then it "
                                      "can cause unwanted blur."));

    // -------------------------------------------------------------

    QLabel *label3    = new QLabel(i18n("Threshold:"), firstPage);
    d->thresholdInput = new RDoubleNumInput(firstPage);
    d->thresholdInput->setDecimals(2);
    d->thresholdInput->input()->setRange(0.01, 1.0, 0.01, true);
    d->thresholdInput->setDefaultValue(0.08);
    d->thresholdInput->setWhatsThis(i18n("<b>Threshold</b>: use the slider for coarse adjustment, "
                                         "and the spin control for fine adjustment to control edge detection sensitivity. "
                                         "This value should be set so that edges and details are clearly visible "
                                         "and noise is smoothed out. "
                                         "Adjustment must be made carefully, because the gap between \"noisy\", "
                                         "\"smooth\", and \"blur\" is very small. Adjust it as carefully as you would adjust "
                                         "the focus of a camera."));

    // -------------------------------------------------------------

    QLabel *label4  = new QLabel(i18n("Texture:"), firstPage);
    d->textureInput = new RDoubleNumInput(firstPage);
    d->textureInput->setDecimals(2);
    d->textureInput->input()->setRange(-0.99, 0.99, 0.01, true);
    d->textureInput->setDefaultValue(0.0);
    d->textureInput->setWhatsThis(i18n("<b>Texture</b>: this control sets the texture accuracy. "
                                       "This value can be used, to get more or less texture accuracy. When decreased, "
                                       "then noise and texture are blurred out, when increased then texture is "
                                       "amplified, but also noise will increase. It has almost no effect on image edges."));

    // -------------------------------------------------------------

    QLabel *label7    = new QLabel(i18n("Sharpness:"), firstPage);  // Filter setting "Lookahead".
    d->sharpnessInput = new RDoubleNumInput(firstPage);
    d->sharpnessInput->setDecimals(2);
    d->sharpnessInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->sharpnessInput->setDefaultValue(0.25);
    d->sharpnessInput->setWhatsThis(i18n("<b>Sharpness</b>: "
                                         "This value improves the frequency response for the filter. "
                                         "When it is too strong then not all noise can be removed, or spike noise may appear. "
                                         "Set it near to maximum, if you want to remove very weak noise or JPEG-artifacts, "
                                         "without losing detail."));

    // -------------------------------------------------------------

    QLabel *label5    = new QLabel(i18n("Edge Lookahead:"), firstPage);     // Filter setting "Sharp".
    d->lookaheadInput = new RDoubleNumInput(firstPage);
    d->lookaheadInput->setDecimals(2);
    d->lookaheadInput->input()->setRange(0.01, 20.0, 0.01, true);
    d->lookaheadInput->setDefaultValue(2.0);
    d->lookaheadInput->setWhatsThis(i18n("<b>Edge</b>: "
                                         "This value defines the pixel distance to which the filter looks ahead for edges. "
                                         "When this value is increased, then spike noise is erased. "
                                         "You can eventually re-adjust the <b>Edge</b> filter, when you have changed this setting. "
                                         "When this value is too high, the adaptive filter can no longer accurately track "
                                         "image details, and noise or blurring can occur."));

    // -------------------------------------------------------------

    QLabel *label10  = new QLabel(i18n("Erosion:"), firstPage);
    d->phaseInput    = new RDoubleNumInput(firstPage);
    d->phaseInput->setDecimals(1);
    d->phaseInput->input()->setRange(0.5, 20.0, 0.5, true);
    d->phaseInput->setDefaultValue(1.0);
    d->phaseInput->setWhatsThis(i18n("<b>Erosion</b>: "
                                     "Use this to increase edge noise erosion and spike noise erosion "
                                     "(noise is removed by erosion)."));

    grid1->addWidget(label1,            0, 0, 1, 1);
    grid1->addWidget(d->radiusInput,    0, 1, 1, 1);
    grid1->addWidget(label3,            1, 0, 1, 1);
    grid1->addWidget(d->thresholdInput, 1, 1, 1, 1);
    grid1->addWidget(label4,            2, 0, 1, 1);
    grid1->addWidget(d->textureInput,   2, 1, 1, 1);
    grid1->addWidget(label7,            3, 0, 1, 1);
    grid1->addWidget(d->sharpnessInput, 3, 1, 1, 1);
    grid1->addWidget(label5,            4, 0, 1, 1);
    grid1->addWidget(d->lookaheadInput, 4, 1, 1, 1);
    grid1->addWidget(label10,           5, 0, 1, 1);
    grid1->addWidget(d->phaseInput,     5, 1, 1, 1);
    grid1->setMargin(d->gboxSettings->spacingHint());
    grid1->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget();
    QGridLayout* grid2  = new QGridLayout( secondPage );

    QLabel *label2       = new QLabel(i18n("Luminance:"), secondPage);
    d->lumToleranceInput = new RDoubleNumInput(secondPage);
    d->lumToleranceInput->setDecimals(1);
    d->lumToleranceInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->lumToleranceInput->setDefaultValue(1.0);
    d->lumToleranceInput->setWhatsThis(i18n("<b>Luminance</b>: this control sets the luminance tolerance of the image. "
                                            "Using either the <b>Color</b> or the <b>Luminance</b> tolerance settings "
                                            "to make an image correction is recommended, but not both at the same time. These settings "
                                            "do not influence the main smoothing process controlled by the <b>Details</b> "
                                            "settings."));

    // -------------------------------------------------------------

    QLabel *label6  = new QLabel(i18nc("color tolerance", "Color:"), secondPage);
    d->csmoothInput = new RDoubleNumInput(secondPage);
    d->csmoothInput->setDecimals(1);
    d->csmoothInput->input()->setRange(0.0, 1.0, 0.1, true);
    d->csmoothInput->setDefaultValue(1.0);
    d->csmoothInput->setWhatsThis(i18n("<b>Color</b>: this control sets the color tolerance of the image. It is "
                                       "recommended using either the <b>Color</b> or the <b>Luminance</b> tolerance "
                                       "to make image correction, not both at the same time. These settings "
                                       "do not influence the main smoothing process controlled by the <b>Details</b> "
                                       "settings."));

    // -------------------------------------------------------------

    QLabel *label8  = new QLabel(i18nc("gamma tolerance", "Gamma:"), secondPage);
    d->gammaInput   = new RDoubleNumInput(secondPage);
    d->gammaInput->setDecimals(1);
    d->gammaInput->input()->setRange(0.3, 3.0, 0.1, true);
    d->gammaInput->setDefaultValue(1.4);
    d->gammaInput->setWhatsThis(i18n("<b>Gamma</b>: this control sets the gamma tolerance of the image. This value "
                                     "can be used to increase the tolerance values for darker areas (which commonly "
                                     "are noisier). This results in more blur for shadow areas."));

    // -------------------------------------------------------------

    QLabel *label9  = new QLabel(i18n("Damping:"), secondPage);
    d->dampingInput = new RDoubleNumInput(secondPage);
    d->dampingInput->setDecimals(1);
    d->dampingInput->input()->setRange(0.5, 20.0, 0.5, true);
    d->dampingInput->setDefaultValue(5.0);
    d->dampingInput->setWhatsThis(i18n("<b>Damping</b>: this control sets the phase-jitter damping adjustment. "
                                       "This value defines how fast the adaptive filter-radius reacts to luminance "
                                       "variations. If increased, then edges appear smoother; if too high, then blur "
                                       "may occur. If at minimum, then noise and phase jitter at the edges can occur. It "
                                       "can suppress spike noise when increased, and this is the preferred method to "
                                       "remove it."));

    grid2->addWidget(label2,               0, 0, 1, 1);
    grid2->addWidget(d->lumToleranceInput, 0, 1, 1, 1);
    grid2->addWidget(label6,               1, 0, 1, 1);
    grid2->addWidget(d->csmoothInput,      1, 1, 1, 1);
    grid2->addWidget(label8,               2, 0, 1, 1);
    grid2->addWidget(d->gammaInput,        2, 1, 1, 1);
    grid2->addWidget(label9,               3, 0, 1, 1);
    grid2->addWidget(d->dampingInput,      3, 1, 1, 1);
    grid2->setColumnStretch(1, 10);
    grid2->setRowStretch(4, 10);
    grid2->setMargin(d->gboxSettings->spacingHint());
    grid2->setSpacing(d->gboxSettings->spacingHint());

    d->expanderBox = new RExpanderBox;
    d->expanderBox->setObjectName("NoiseReductionTool Expander");
    d->expanderBox->addItem(firstPage, SmallIcon("noisereduction"), i18n("Details"),
                            QString("DetailsContainer"), true);
    d->expanderBox->addItem(secondPage, SmallIcon("noisereduction"), i18n("Advanced settings"),
                            QString("AdvancedSettingsContainer"), true);
    d->expanderBox->addStretch();

    grid->addWidget(d->expanderBox, 0, 0, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);

    d->previewWidget = new ImagePanelWidget(470, 350, "noisereduction Tool", d->gboxSettings->panIconView());
    setToolView(d->previewWidget);

    init();
}

NoiseReductionTool::~NoiseReductionTool()
{
    delete d;
}

void NoiseReductionTool::renderingFinished()
{
    d->expanderBox->setEnabled(true);
}

void NoiseReductionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->expanderBox->setEnabled(false);

    d->csmoothInput->setValue(group.readEntry(d->configCsmoothAdjustmentEntry,           d->csmoothInput->defaultValue()));
    d->dampingInput->setValue(group.readEntry(d->configDampingAdjustmentEntry,           d->dampingInput->defaultValue()));
    d->gammaInput->setValue(group.readEntry(d->configGammaAdjustmentEntry,               d->gammaInput->defaultValue()));
    d->lookaheadInput->setValue(group.readEntry(d->configLookAheadAdjustmentEntry,       d->lookaheadInput->defaultValue()));
    d->lumToleranceInput->setValue(group.readEntry(d->configLumToleranceAdjustmentEntry, d->lumToleranceInput->defaultValue()));
    d->phaseInput->setValue(group.readEntry(d->configPhaseAdjustmentEntry,               d->phaseInput->defaultValue()));
    d->radiusInput->setValue(group.readEntry(d->configRadiusAdjustmentEntry,             d->radiusInput->defaultValue()));
    d->sharpnessInput->setValue(group.readEntry(d->configSharpnessAdjustmentEntry,       d->sharpnessInput->defaultValue()));
    d->textureInput->setValue(group.readEntry(d->configTextureAdjustmentEntry,           d->textureInput->defaultValue()));
    d->thresholdInput->setValue(group.readEntry(d->configThresholdAdjustmentEntry,       d->thresholdInput->defaultValue()));
    d->expanderBox->readSettings();

    d->expanderBox->setEnabled(true);
}

void NoiseReductionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configRadiusAdjustmentEntry,       d->radiusInput->value());
    group.writeEntry(d->configLumToleranceAdjustmentEntry, d->lumToleranceInput->value());
    group.writeEntry(d->configThresholdAdjustmentEntry,    d->thresholdInput->value());
    group.writeEntry(d->configTextureAdjustmentEntry,      d->textureInput->value());
    group.writeEntry(d->configSharpnessAdjustmentEntry,    d->sharpnessInput->value());
    group.writeEntry(d->configCsmoothAdjustmentEntry,      d->csmoothInput->value());
    group.writeEntry(d->configLookAheadAdjustmentEntry,    d->lookaheadInput->value());
    group.writeEntry(d->configGammaAdjustmentEntry,        d->gammaInput->value());
    group.writeEntry(d->configDampingAdjustmentEntry,      d->dampingInput->value());
    group.writeEntry(d->configPhaseAdjustmentEntry,        d->phaseInput->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void NoiseReductionTool::slotResetSettings()
{
    d->expanderBox->setEnabled(false);

    d->csmoothInput->slotReset();
    d->dampingInput->slotReset();
    d->gammaInput->slotReset();
    d->lookaheadInput->slotReset();
    d->lumToleranceInput->slotReset();
    d->phaseInput->slotReset();
    d->radiusInput->slotReset();
    d->sharpnessInput->slotReset();
    d->textureInput->slotReset();
    d->thresholdInput->slotReset();

    d->expanderBox->setEnabled(true);
}

void NoiseReductionTool::prepareEffect()
{
    d->expanderBox->setEnabled(false);

    double r   = d->radiusInput->value();
    double l   = d->lumToleranceInput->value();
    double th  = d->thresholdInput->value();
    double tx  = d->textureInput->value();
    double s   = d->sharpnessInput->value();
    double c   = d->csmoothInput->value();
    double a   = d->lookaheadInput->value();
    double g   = d->gammaInput->value();
    double da  = d->dampingInput->value();
    double p   = d->phaseInput->value();
    DImg image = d->previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new NoiseReduction(&image, this, r, l, th, tx, s, c, a, g, da, p)));
}

void NoiseReductionTool::prepareFinal()
{
    d->expanderBox->setEnabled(false);

    double r  = d->radiusInput->value();
    double l  = d->lumToleranceInput->value();
    double th = d->thresholdInput->value();
    double tx = d->textureInput->value();
    double s  = d->sharpnessInput->value();
    double c  = d->csmoothInput->value();
    double a  = d->lookaheadInput->value();
    double g  = d->gammaInput->value();
    double da = d->dampingInput->value();
    double p  = d->phaseInput->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter *>(new NoiseReduction(iface.getOriginalImg(), this, r, l, th, tx, s, c, a, g, da, p)));
}

void NoiseReductionTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
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

    QFile file(loadRestorationFile.toLocalFile());

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
        d->radiusInput->setValue( stream.readLine().toDouble() );
        d->lumToleranceInput->setValue( stream.readLine().toDouble() );
        d->thresholdInput->setValue( stream.readLine().toDouble() );
        d->textureInput->setValue( stream.readLine().toDouble() );
        d->sharpnessInput->setValue( stream.readLine().toDouble() );
        d->csmoothInput->setValue( stream.readLine().toDouble() );
        d->lookaheadInput->setValue( stream.readLine().toDouble() );
        d->gammaInput->setValue( stream.readLine().toDouble() );
        d->dampingInput->setValue( stream.readLine().toDouble() );
        d->phaseInput->setValue( stream.readLine().toDouble() );
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

    QFile file(saveRestorationFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Photograph Noise Reduction Configuration File\n";
        stream << d->radiusInput->value() << "\n";
        stream << d->lumToleranceInput->value() << "\n";
        stream << d->thresholdInput->value() << "\n";
        stream << d->textureInput->value() << "\n";
        stream << d->sharpnessInput->value() << "\n";
        stream << d->csmoothInput->value() << "\n";
        stream << d->lookaheadInput->value() << "\n";
        stream << d->gammaInput->value() << "\n";
        stream << d->dampingInput->value() << "\n";
        stream << d->phaseInput->value() << "\n";

    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Noise Reduction text file."));
    }

    file.close();
}

}  // namespace DigikamNoiseReductionImagesPlugin
