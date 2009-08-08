/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
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

#include "localcontrasttool.h"
#include "localcontrasttool.moc"

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
#include <libkdcraw/rcombobox.h>

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "localcontrast.h"
#include "rexpanderbox.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamLocalContrastImagesPlugin
{

class LocalContrastToolPriv
{
public:

    LocalContrastToolPriv()
    {
        lowSaturationInput  = 0;
        highSaturationInput = 0;
        functionInput       = 0;
        fastModeCheck       = 0;
        stageOne            = 0;
        stageTwo            = 0;
        stageThree          = 0;
        stageFour           = 0;
        powerInput1         = 0;
        blurInput1          = 0;
        powerInput2         = 0;
        blurInput2          = 0;
        powerInput3         = 0;
        blurInput3          = 0;
        powerInput4         = 0;
        blurInput4          = 0;
        previewWidget       = 0;
        expanderBox         = 0;
        gboxSettings        = 0;
    }

    RIntNumInput*       lowSaturationInput;
    RIntNumInput*       highSaturationInput;

    RComboBox*          functionInput;

    QCheckBox*          stretchContrastCheck;
    QCheckBox*          fastModeCheck;
    QCheckBox*          stageOne;
    QCheckBox*          stageTwo;
    QCheckBox*          stageThree;
    QCheckBox*          stageFour;

    RDoubleNumInput*    powerInput1;
    RDoubleNumInput*    blurInput1;
    RDoubleNumInput*    powerInput2;
    RDoubleNumInput*    blurInput2;
    RDoubleNumInput*    powerInput3;
    RDoubleNumInput*    blurInput3;
    RDoubleNumInput*    powerInput4;
    RDoubleNumInput*    blurInput4;

    ImagePanelWidget*   previewWidget;
    RExpanderBox*       expanderBox;
    EditorToolSettings* gboxSettings;
};

LocalContrastTool::LocalContrastTool(QObject* parent)
                  : EditorToolThreaded(parent),
                    d(new LocalContrastToolPriv)
{
    setObjectName("localcontrast");
    setToolName(i18n("Local Contrast"));
    setToolIcon(SmallIcon("contrast"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
//                              EditorToolSettings::Load|
//                              EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);


    d->gboxSettings->setTools(EditorToolSettings::PanIcon);

    // -------------------------------------------------------------

    QGridLayout* grid  = new QGridLayout( d->gboxSettings->plainPage() );
    QWidget* firstPage = new QWidget();
    QGridLayout* grid1 = new QGridLayout(firstPage);

    QLabel *label1   = new QLabel(i18n("Function:"), firstPage);
    d->functionInput = new RComboBox(firstPage);
    d->functionInput->addItem(i18n("Power"));
    d->functionInput->addItem(i18n("Linear"));
    d->functionInput->setDefaultIndex(0);
    d->functionInput->setWhatsThis(i18n("<b>Function</b>: This is the function which combines the original RGB channels "
                                        "with the desaturated blurred image. This function used on each of the tonemapping "
                                        "stage. It can linear or power. Basically, these function increase the value where "
                                        "both the original and blurred image's value are low and do the opposite on high "
                                        "values."));

    // -------------------------------------------------------------

    d->stretchContrastCheck = new QCheckBox(i18n("Stretch contrast"), firstPage);
    d->stretchContrastCheck->setWhatsThis(i18n("<b>Stretch contrast</b>: This stretches the contrast of the original image. "
                                               "It is applied before the tonemapping process."));
    d->stretchContrastCheck->setChecked(false);

    // -------------------------------------------------------------

    QLabel *label2          = new QLabel(i18n("High saturation:"), firstPage);
    d->highSaturationInput  = new RIntNumInput(firstPage);
    d->highSaturationInput->setRange(0, 100, 1);
    d->highSaturationInput->setDefaultValue(50);
    d->highSaturationInput->setSliderEnabled(true);
    d->highSaturationInput->setObjectName("highSaturationInput");
    d->highSaturationInput->setWhatsThis(i18n("<b>High and low saturation</b>: Usually the (perceived) saturation is "
                                             "increased. The user can choose to lower the saturation on original highlight "
                                             "and shadows from the image with these parameters."));

    // -------------------------------------------------------------

    QLabel *label3         = new QLabel(i18n("Low saturation:"), firstPage);
    d->lowSaturationInput  = new RIntNumInput(firstPage);
    d->lowSaturationInput->setRange(0, 100, 1);
    d->lowSaturationInput->setDefaultValue(50);
    d->lowSaturationInput->setSliderEnabled(true);
    d->lowSaturationInput->setObjectName("lowSaturationInput");
    d->lowSaturationInput->setWhatsThis(i18n("<b>High and low saturation</b>: Usually the (perceived) saturation is "
                                             "increased. The user can choose to lower the saturation on original highlight "
                                             "and shadows from the image with these parameters."));

    // -------------------------------------------------------------

    d->fastModeCheck = new QCheckBox(i18n("Fast mode"), firstPage);
    d->fastModeCheck->setWhatsThis(i18n("<b>Fast mode</b>: There are available two ways to do the tonemapping: using a "
                                        "very fast algorithm, which might produce artifacts on the image or a more precise "
                                        "algorithm, which is slower."));
    d->fastModeCheck->setChecked(false);

    grid1->addWidget(label1,                    0, 0, 1, 1);
    grid1->addWidget(d->functionInput,          0, 1, 1, 1);
    grid1->addWidget(d->stretchContrastCheck,   1, 0, 1, 1);
    grid1->addWidget(label2,                    2, 0, 1, 1);
    grid1->addWidget(d->highSaturationInput,    2, 1, 1, 1);
    grid1->addWidget(label3,                    3, 0, 1, 1);
    grid1->addWidget(d->lowSaturationInput,     3, 1, 1, 1);
    grid1->addWidget(d->fastModeCheck,          4, 0, 1, 1);
    grid1->setMargin(d->gboxSettings->spacingHint());
    grid1->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget();
    QGridLayout* grid2  = new QGridLayout( secondPage );

    d->stageOne = new QCheckBox(i18n("Enabled"), secondPage);
    d->stageOne->setWhatsThis(i18n("Check to enable this stage."));
    d->stageOne->setChecked(false);

    // -------------------------------------------------------------

    QLabel* label4 = new QLabel(i18n("Power:"), secondPage);
    d->powerInput1 = new RDoubleNumInput(firstPage);
    d->powerInput1->input()->setRange(0.0, 100.0, 1.0, true);
    d->powerInput1->setDefaultValue(50.0);
    d->powerInput1->setObjectName("powerInput1");
    d->powerInput1->setWhatsThis(i18n("<b>Power</b>: How strong the effect is applied."));

    // -------------------------------------------------------------

    QLabel* label5 = new QLabel(i18n("Blur:"), secondPage);
    d->blurInput1  = new RDoubleNumInput(firstPage);
    d->blurInput1->input()->setRange(0.0, 1000.0, 1.0, true);
    d->blurInput1->setDefaultValue(500.0);
    d->blurInput1->setObjectName("blurInput1");
    d->blurInput1->setWhatsThis(i18n("<b>Blur</b>: How strong the image is blured before combining with the original "
                                    "image with the tonemapping function."));

    grid2->addWidget(d->stageOne,           0, 0, 1, 1);
    grid2->addWidget(label4,                1, 0, 1, 1);
    grid2->addWidget(d->powerInput1,        1, 1, 1, 1);
    grid2->addWidget(label5,                2, 0, 1, 1);
    grid2->addWidget(d->blurInput1,         2, 1, 1, 1);
    grid2->setMargin(d->gboxSettings->spacingHint());
    grid2->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget* thirdPage = new QWidget();
    QGridLayout* grid3  = new QGridLayout( thirdPage );

    d->stageTwo = new QCheckBox(i18n("Enabled"), thirdPage);
    d->stageTwo->setWhatsThis(i18n("Check to enable this stage."));
    d->stageTwo->setChecked(false);

    // -------------------------------------------------------------

    QLabel* label6 = new QLabel(i18n("Power:"), thirdPage);
    d->powerInput2 = new RDoubleNumInput(thirdPage);
    d->powerInput2->input()->setRange(0.0, 100.0, 1.0, true);
    d->powerInput2->setDefaultValue(50.0);
    d->powerInput2->setObjectName("powerInput2");
    d->powerInput2->setWhatsThis(i18n("<b>Power</b>: How strong the effect is applied."));

    // -------------------------------------------------------------

    QLabel* label7 = new QLabel(i18n("Blur:"), thirdPage);
    d->blurInput2  = new RDoubleNumInput(thirdPage);
    d->blurInput2->input()->setRange(0.0, 1000.0, 1.0, true);
    d->blurInput2->setDefaultValue(500.0);
    d->blurInput2->setObjectName("blurInput2");
    d->blurInput2->setWhatsThis(i18n("<b>Blur</b>: How strong the image is blured before combining with the original "
                                    "image with the tonemapping function."));

    grid3->addWidget(d->stageTwo,           0, 0, 1, 1);
    grid3->addWidget(label6,                1, 0, 1, 1);
    grid3->addWidget(d->powerInput2,        1, 1, 1, 1);
    grid3->addWidget(label7,                2, 0, 1, 1);
    grid3->addWidget(d->blurInput2,         2, 1, 1, 1);
    grid3->setMargin(d->gboxSettings->spacingHint());
    grid3->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget* fourthPage = new QWidget();
    QGridLayout* grid4  = new QGridLayout( fourthPage );

    d->stageThree = new QCheckBox(i18n("Enabled"), fourthPage);
    d->stageThree->setWhatsThis(i18n("Check to enable this stage."));
    d->stageThree->setChecked(false);

    // -------------------------------------------------------------

    QLabel* label8 = new QLabel(i18n("Power:"), fourthPage);
    d->powerInput3 = new RDoubleNumInput(fourthPage);
    d->powerInput3->input()->setRange(0.0, 100.0, 1.0, true);
    d->powerInput3->setDefaultValue(50.0);
    d->powerInput3->setObjectName("powerInput3");
    d->powerInput3->setWhatsThis(i18n("<b>Power</b>: How strong the effect is applied."));

    // -------------------------------------------------------------

    QLabel* label9 = new QLabel(i18n("Blur:"), fourthPage);
    d->blurInput3  = new RDoubleNumInput(fourthPage);
    d->blurInput3->input()->setRange(0.0, 1000.0, 1.0, true);
    d->blurInput3->setDefaultValue(500.0);
    d->blurInput3->setObjectName("blurInput3");
    d->blurInput3->setWhatsThis(i18n("<b>Blur</b>: How strong the image is blured before combining with the original "
                                    "image with the tonemapping function."));

    grid4->addWidget(d->stageThree,         0, 0, 1, 1);
    grid4->addWidget(label8,                1, 0, 1, 1);
    grid4->addWidget(d->powerInput3,        1, 1, 1, 1);
    grid4->addWidget(label9,                2, 0, 1, 1);
    grid4->addWidget(d->blurInput3,         2, 1, 1, 1);
    grid4->setMargin(d->gboxSettings->spacingHint());
    grid4->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget* fifthPage = new QWidget();
    QGridLayout* grid5  = new QGridLayout( fifthPage );

    d->stageFour = new QCheckBox(i18n("Enabled"), fifthPage);
    d->stageFour->setWhatsThis(i18n("Check to enable this stage."));
    d->stageFour->setChecked(false);

    // -------------------------------------------------------------

    QLabel* label10 = new QLabel(i18n("Power:"), fifthPage);
    d->powerInput4  = new RDoubleNumInput(fifthPage);
    d->powerInput4->input()->setRange(0.0, 100.0, 1.0, true);
    d->powerInput4->setDefaultValue(50.0);
    d->powerInput4->setObjectName("powerInput4");
    d->powerInput4->setWhatsThis(i18n("<b>Power</b>: How strong the effect is applied."));

    // -------------------------------------------------------------

    QLabel* label11 = new QLabel(i18n("Blur:"), fifthPage);
    d->blurInput4   = new RDoubleNumInput(fifthPage);
    d->blurInput4->input()->setRange(0.0, 1000.0, 1.0, true);
    d->blurInput4->setDefaultValue(500.0);
    d->blurInput4->setObjectName("blurInput4");
    d->blurInput4->setWhatsThis(i18n("<b>Blur</b>: How strong the image is blured before combining with the original "
                                    "image with the tonemapping function."));

    grid5->addWidget(d->stageFour,          0, 0, 1, 1);
    grid5->addWidget(label10,               1, 0, 1, 1);
    grid5->addWidget(d->powerInput4,        1, 1, 1, 1);
    grid5->addWidget(label11,               2, 0, 1, 1);
    grid5->addWidget(d->blurInput4,         2, 1, 1, 1);
    grid5->setMargin(d->gboxSettings->spacingHint());
    grid5->setSpacing(d->gboxSettings->spacingHint());

    d->expanderBox = new RExpanderBox;
    d->expanderBox->addItem(firstPage, SmallIcon("contrast"), i18n("General settings"),
                            QString("GeneralSettingsContainer"), true);
    d->expanderBox->addItem(secondPage, SmallIcon("contrast"), i18n("Stage 1"),
                            QString("Stage1SettingsContainer"), true);
    d->expanderBox->addItem(thirdPage, SmallIcon("contrast"), i18n("Stage 2"),
                            QString("Stage2SettingsContainer"), true);
    d->expanderBox->addItem(fourthPage, SmallIcon("contrast"), i18n("Stage 3"),
                            QString("Stage3SettingsContainer"), true);
    d->expanderBox->addItem(fifthPage, SmallIcon("contrast"), i18n("Stage 4"),
                            QString("Stage4SettingsContainer"), true);
    d->expanderBox->addStretch();

    grid->addWidget(d->expanderBox, 0, 0, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);

    d->previewWidget = new ImagePanelWidget(470, 350, "localcontrast Tool", d->gboxSettings->panIconView());
    setToolView(d->previewWidget);

    init();
}

LocalContrastTool::~LocalContrastTool()
{
    delete d;
}

void LocalContrastTool::renderingFinished()
{
    d->expanderBox->setEnabled(true);
}

void LocalContrastTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("localcontrast Tool");

    d->expanderBox->setEnabled(false);

    d->lowSaturationInput->setValue(group.readEntry("LowSaturation",        d->lowSaturationInput->defaultValue()));
    d->highSaturationInput->setValue(group.readEntry("HighSaturation",      d->highSaturationInput->defaultValue()));
    d->powerInput1->setValue(group.readEntry("Power1",                      d->powerInput1->defaultValue()));
    d->blurInput1->setValue(group.readEntry("Blur1",                        d->blurInput1->defaultValue()));
    d->powerInput2->setValue(group.readEntry("Power2",                      d->powerInput2->defaultValue()));
    d->blurInput2->setValue(group.readEntry("Blur2",                        d->blurInput2->defaultValue()));
    d->powerInput3->setValue(group.readEntry("Power3",                      d->powerInput3->defaultValue()));
    d->blurInput3->setValue(group.readEntry("Blur3",                        d->blurInput3->defaultValue()));
    d->powerInput4->setValue(group.readEntry("Power4",                      d->powerInput4->defaultValue()));
    d->blurInput4->setValue(group.readEntry("Blur4",                        d->blurInput4->defaultValue()));
    d->stretchContrastCheck->setChecked(group.readEntry("StretchContrast",  false));
    d->fastModeCheck->setChecked(group.readEntry("FastMode",                false));
    d->stageOne->setChecked(group.readEntry("StageOne",                     false));
    d->stageTwo->setChecked(group.readEntry("StageTwo",                     false));
    d->stageThree->setChecked(group.readEntry("StageThree",                 false));
    d->stageFour->setChecked(group.readEntry("StageFour",                   false));
    d->functionInput->setCurrentIndex(group.readEntry("FunctionInput",      d->functionInput->defaultIndex()));
    d->expanderBox->readSettings();

    d->expanderBox->setEnabled(true);
}

void LocalContrastTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("localcontrast Tool");

    group.writeEntry("LowSaturation",       d->lowSaturationInput->value());
    group.writeEntry("HighSaturation",      d->highSaturationInput->value());
    group.writeEntry("Power1",              d->powerInput1->value());
    group.writeEntry("Blur1",               d->blurInput1->value());
    group.writeEntry("Power2",              d->powerInput2->value());
    group.writeEntry("Blur2",               d->blurInput2->value());
    group.writeEntry("Power3",              d->powerInput3->value());
    group.writeEntry("Blur3",               d->blurInput3->value());
    group.writeEntry("Power4",              d->powerInput4->value());
    group.writeEntry("Blur4",               d->blurInput4->value());
    group.writeEntry("StretchContrast",     d->stretchContrastCheck->isChecked());
    group.writeEntry("FastMode",            d->fastModeCheck->isChecked());
    group.writeEntry("StageOne",            d->stageOne->isChecked());
    group.writeEntry("StageTwo",            d->stageTwo->isChecked());
    group.writeEntry("StageThree",          d->stageThree->isChecked());
    group.writeEntry("StageFour",           d->stageFour->isChecked());
    group.writeEntry("FunctionInput",       d->functionInput->currentIndex());
    d->expanderBox->writeSettings();

    d->previewWidget->writeSettings();
    group.sync();
}

void LocalContrastTool::slotResetSettings()
{
    d->expanderBox->setEnabled(false);

    d->lowSaturationInput->slotReset();
    d->highSaturationInput->slotReset();
    d->powerInput1->slotReset();
    d->blurInput1->slotReset();
    d->powerInput2->slotReset();
    d->blurInput2->slotReset();
    d->powerInput3->slotReset();
    d->blurInput3->slotReset();
    d->powerInput4->slotReset();
    d->blurInput4->slotReset();
    d->stretchContrastCheck->setChecked(false);
    d->fastModeCheck->setChecked(false);
    d->stageOne->setChecked(false);
    d->stageTwo->setChecked(false);
    d->stageThree->setChecked(false);
    d->stageFour->setChecked(false);

    d->expanderBox->setEnabled(true);
}

ToneMappingParameters *LocalContrastTool::createParams()
{
    ToneMappingParameters *par = new ToneMappingParameters();

    // Setting general parameters
    par->info_fast_mode   = d->fastModeCheck->isChecked();
    par->low_saturation   = d->lowSaturationInput->value();
    par->high_saturation  = d->highSaturationInput->value();
    par->stretch_contrast = d->stretchContrastCheck->isChecked();
    par->function_id      = d->functionInput->currentIndex();

    // Setting stages parameters
    par->stage[0].enabled = d->stageOne->isChecked();
    par->stage[1].enabled = d->stageTwo->isChecked();
    par->stage[2].enabled = d->stageThree->isChecked();
    par->stage[3].enabled = d->stageFour->isChecked();

    if(par->stage[0].enabled)
    {
        par->stage[0].power = d->powerInput1->value();
        par->stage[0].blur = d->blurInput1->value();
    }

    if(par->stage[1].enabled)
    {
        par->stage[1].power = d->powerInput2->value();
        par->stage[1].blur = d->blurInput2->value();
    }

    if(par->stage[2].enabled)
    {
        par->stage[2].power = d->powerInput3->value();
        par->stage[2].blur = d->blurInput3->value();
    }

    if(par->stage[3].enabled)
    {
        par->stage[3].power = d->powerInput4->value();
        par->stage[3].blur = d->blurInput4->value();
    }

    return par;
}

void LocalContrastTool::prepareEffect()
{
    d->expanderBox->setEnabled(false);

    ToneMappingParameters *par = createParams();

    DImg image = d->previewWidget->getOriginalRegionImage();
    DImg *image2 = new DImg(image.width(), image.height(), image.sixteenBit(), image.hasAlpha(), image.bits());

    setFilter(dynamic_cast<DImgThreadedFilter*>(new LocalContrast(image2, par, this)));

//     delete image2;
}

void LocalContrastTool::prepareFinal()
{
    d->expanderBox->setEnabled(false);

    ToneMappingParameters *par = createParams();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter *>(new LocalContrast(iface.getOriginalImg(), par, this)));
}

void LocalContrastTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void LocalContrastTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Local Contrast"), filter()->getTargetImage().bits());
}

// void LocalContrastTool::slotLoadSettings()
// {
//     KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
//                                QString( "*" ), kapp->activeWindow(),
//                                QString( i18n("Photograph Noise Reduction Settings File to Load")) );
//     if ( loadRestorationFile.isEmpty() )
//         return;
//
//     QFile file(loadRestorationFile.path());
//
//     if ( file.open(QIODevice::ReadOnly) )
//     {
//         QTextStream stream( &file );
//         if ( stream.readLine() != "# Photograph Noise Reduction Configuration File" )
//         {
//             KMessageBox::error(kapp->activeWindow(),
//                                i18n("\"%1\" is not a Photograph Noise Reduction settings text file.",
//                                     loadRestorationFile.fileName()));
//             file.close();
//             return;
//         }
//
//         blockSignals(true);
//         d->radiusInput->setValue( stream.readLine().toDouble() );
//         d->lumToleranceInput->setValue( stream.readLine().toDouble() );
//         d->thresholdInput->setValue( stream.readLine().toDouble() );
//         d->textureInput->setValue( stream.readLine().toDouble() );
//         d->sharpnessInput->setValue( stream.readLine().toDouble() );
//         d->csmoothInput->setValue( stream.readLine().toDouble() );
//         d->lookaheadInput->setValue( stream.readLine().toDouble() );
//         d->gammaInput->setValue( stream.readLine().toDouble() );
//         d->dampingInput->setValue( stream.readLine().toDouble() );
//         d->phaseInput->setValue( stream.readLine().toDouble() );
//         blockSignals(false);
// //         slotEffect();
//     }
//     else
//     {
//         KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Noise Reduction text file."));
//     }
//
//     file.close();
// }

// void LocalContrastTool::slotSaveAsSettings()
// {
//     KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
//                                QString( "*" ), kapp->activeWindow(),
//                                QString( i18n("Photograph Noise Reduction Settings File to Save")) );
//     if ( saveRestorationFile.isEmpty() )
//         return;
//
//     QFile file(saveRestorationFile.path());
//
//     if ( file.open(QIODevice::WriteOnly) )
//     {
//         QTextStream stream( &file );
//         stream << "# Photograph Noise Reduction Configuration File\n";
//         stream << d->radiusInput->value() << "\n";
//         stream << d->lumToleranceInput->value() << "\n";
//         stream << d->thresholdInput->value() << "\n";
//         stream << d->textureInput->value() << "\n";
//         stream << d->sharpnessInput->value() << "\n";
//         stream << d->csmoothInput->value() << "\n";
//         stream << d->lookaheadInput->value() << "\n";
//         stream << d->gammaInput->value() << "\n";
//         stream << d->dampingInput->value() << "\n";
//         stream << d->phaseInput->value() << "\n";
//
//     }
//     else
//     {
//         KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Noise Reduction text file."));
//     }
//
//     file.close();
// }

}  // namespace DigikamNoiseReductionImagesPlugin
