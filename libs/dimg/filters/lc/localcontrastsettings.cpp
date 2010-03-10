/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : Local Contrast settings view.
 *               LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "localcontrastsettings.moc"

// Qt includes

#include <QString>
#include <QButtonGroup>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegExp>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes

#include <kdebug.h>
#include <kurl.h>
#include <kdialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kcombobox.h>
#include <kseparator.h>
#include <kiconloader.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "rexpanderbox.h"

using namespace KDcrawIface;

namespace Digikam
{

class LocalContrastSettingsPriv
{

public:

    LocalContrastSettingsPriv() :
        configLowSaturationEntry("LowSaturation"),
        configHighSaturationEntry("HighSaturation"),
        configPower1Entry("Power1"),
        configBlur1Entry("Blur1"),
        configPower2Entry("Power2"),
        configBlur2Entry("Blur2"),
        configPower3Entry("Power3"),
        configBlur3Entry("Blur3"),
        configPower4Entry("Power4"),
        configBlur4Entry("Blur4"),
        configStretchContrastEntry("StretchContrast"),
        configStageOneEntry("StageOne"),
        configStageTwoEntry("StageTwo"),
        configStageThreeEntry("StageThree"),
        configStageFourEntry("StageFour"),
        configFunctionInputEntry("FunctionInput"),
        stretchContrastCheck(0),
        stageOne(0),
        stageTwo(0),
        stageThree(0),
        stageFour(0),
        label4(0),
        label5(0),
        label6(0),
        label7(0),
        label8(0),
        label9(0),
        label10(0),
        label11(0),
        lowSaturationInput(0),
        highSaturationInput(0),
        functionInput(0),
        powerInput1(0),
        blurInput1(0),
        powerInput2(0),
        blurInput2(0),
        powerInput3(0),
        blurInput3(0),
        powerInput4(0),
        blurInput4(0),
        expanderBox(0)
        {}

    const QString       configLowSaturationEntry;
    const QString       configHighSaturationEntry;
    const QString       configPower1Entry;
    const QString       configBlur1Entry;
    const QString       configPower2Entry;
    const QString       configBlur2Entry;
    const QString       configPower3Entry;
    const QString       configBlur3Entry;
    const QString       configPower4Entry;
    const QString       configBlur4Entry;
    const QString       configStretchContrastEntry;
    const QString       configFastModeEntry;
    const QString       configStageOneEntry;
    const QString       configStageTwoEntry;
    const QString       configStageThreeEntry;
    const QString       configStageFourEntry;
    const QString       configFunctionInputEntry;

    QCheckBox*          stretchContrastCheck;
    QCheckBox*          stageOne;
    QCheckBox*          stageTwo;
    QCheckBox*          stageThree;
    QCheckBox*          stageFour;

    QLabel*             label4;
    QLabel*             label5;
    QLabel*             label6;
    QLabel*             label7;
    QLabel*             label8;
    QLabel*             label9;
    QLabel*             label10;
    QLabel*             label11;

    RIntNumInput*       lowSaturationInput;
    RIntNumInput*       highSaturationInput;

    RComboBox*          functionInput;

    RDoubleNumInput*    powerInput1;
    RDoubleNumInput*    blurInput1;
    RDoubleNumInput*    powerInput2;
    RDoubleNumInput*    blurInput2;
    RDoubleNumInput*    powerInput3;
    RDoubleNumInput*    blurInput3;
    RDoubleNumInput*    powerInput4;
    RDoubleNumInput*    blurInput4;

    RExpanderBox*       expanderBox;
};

LocalContrastSettings::LocalContrastSettings(QWidget* parent)
                     : QWidget(parent),
                       d(new LocalContrastSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);

    QWidget* firstPage = new QWidget();
    QGridLayout* grid1 = new QGridLayout(firstPage);

    QLabel* label1     = new QLabel(i18n("Function:"), firstPage);
    d->functionInput   = new RComboBox(firstPage);
    d->functionInput->addItem(i18n("Power"));
    d->functionInput->addItem(i18n("Linear"));
    d->functionInput->setDefaultIndex(0);
    d->functionInput->setWhatsThis(i18n("<b>Function</b>: This function combines the original RGB "
                                        "channels with the desaturated blurred image. This function is used in each of "
                                        "the tonemapping stages. It can be linear or power. Basically, this function "
                                        "increases the values where both the original and blurred image's value are low "
                                        "and do the opposite on high values."));

    // -------------------------------------------------------------

    d->stretchContrastCheck = new QCheckBox(i18n("Stretch contrast"), firstPage);
    d->stretchContrastCheck->setWhatsThis(i18n("<b>Stretch contrast</b>: This stretches the contrast of the original image. "
                                               "It is applied before the tonemapping process."));
    d->stretchContrastCheck->setChecked(false);

    // -------------------------------------------------------------

    QLabel* label2         = new QLabel(i18n("Highlights saturation:"), firstPage);
    d->highSaturationInput = new RIntNumInput(firstPage);
    d->highSaturationInput->setRange(0, 100, 1);
    d->highSaturationInput->setDefaultValue(50);
    d->highSaturationInput->setSliderEnabled(true);
    d->highSaturationInput->setObjectName("highSaturationInput");
    d->highSaturationInput->setWhatsThis(i18n("<b>Highlights saturation</b>: Usually the (perceived) saturation is "
                                              "increased. The user can choose to lower the saturation on original highlight "
                                              "and shadows from the image with these parameters."));

    // -------------------------------------------------------------

    QLabel* label3        = new QLabel(i18n("Shadow saturation:"), firstPage);
    d->lowSaturationInput = new RIntNumInput(firstPage);
    d->lowSaturationInput->setRange(0, 100, 1);
    d->lowSaturationInput->setDefaultValue(50);
    d->lowSaturationInput->setSliderEnabled(true);
    d->lowSaturationInput->setObjectName("lowSaturationInput");
    d->lowSaturationInput->setWhatsThis(i18n("<b>Shadow saturation</b>: Usually the (perceived) saturation is "
                                             "increased. The user can choose to lower the saturation on original highlight "
                                             "and shadows from the image with these parameters."));

    // -------------------------------------------------------------

    grid1->addWidget(label1,                  0, 0, 1, 1);
    grid1->addWidget(d->functionInput,        0, 1, 1, 1);
    grid1->addWidget(d->stretchContrastCheck, 1, 0, 1, 1);
    grid1->addWidget(label2,                  2, 0, 1, 1);
    grid1->addWidget(d->highSaturationInput,  2, 1, 1, 1);
    grid1->addWidget(label3,                  3, 0, 1, 1);
    grid1->addWidget(d->lowSaturationInput,   3, 1, 1, 1);
    grid1->setMargin(KDialog::spacingHint());
    grid1->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget();
    QGridLayout* grid2  = new QGridLayout( secondPage );

    d->stageOne         = new QCheckBox(i18n("Enabled"), secondPage);
    d->stageOne->setWhatsThis(i18n("Check to enable this stage."));
    d->stageOne->setChecked(false);
    d->stageOne->setObjectName("stageOne");

    // -------------------------------------------------------------

    d->label4      = new QLabel(i18n("Power:"), secondPage);
    d->powerInput1 = new RDoubleNumInput(firstPage);
    d->powerInput1->input()->setRange(0.0, 100.0, 1.0, true);
    d->powerInput1->setDefaultValue(50.0);
    d->powerInput1->setObjectName("powerInput1");
    d->powerInput1->setWhatsThis(i18n("<b>Power</b>: How strong the effect is applied."));

    // -------------------------------------------------------------

    d->label5      = new QLabel(i18n("Blur:"), secondPage);
    d->blurInput1  = new RDoubleNumInput(firstPage);
    d->blurInput1->input()->setRange(0.0, 1000.0, 1.0, true);
    d->blurInput1->setDefaultValue(500.0);
    d->blurInput1->setObjectName("blurInput1");
    d->blurInput1->setWhatsThis(i18n("<b>Blur</b>: How strong the image is blurred before combining with the original "
                                     "image and with the tonemapping function."));

    grid2->addWidget(d->stageOne,    0, 0, 1, 2);
    grid2->addWidget(d->label4,      1, 0, 1, 1);
    grid2->addWidget(d->powerInput1, 1, 1, 1, 1);
    grid2->addWidget(d->label5,      2, 0, 1, 1);
    grid2->addWidget(d->blurInput1,  2, 1, 1, 1);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QWidget* thirdPage = new QWidget();
    QGridLayout* grid3 = new QGridLayout( thirdPage );

    d->stageTwo = new QCheckBox(i18n("Enabled"), thirdPage);
    d->stageTwo->setWhatsThis(i18n("Check to enable this stage."));
    d->stageTwo->setChecked(false);
    d->stageTwo->setObjectName("stageTwo");

    // -------------------------------------------------------------

    d->label6      = new QLabel(i18n("Power:"), thirdPage);
    d->powerInput2 = new RDoubleNumInput(thirdPage);
    d->powerInput2->input()->setRange(0.0, 100.0, 1.0, true);
    d->powerInput2->setDefaultValue(50.0);
    d->powerInput2->setObjectName("powerInput2");
    d->powerInput2->setWhatsThis(i18n("<b>Power</b>: How strong the effect is applied."));

    // -------------------------------------------------------------

    d->label7     = new QLabel(i18n("Blur:"), thirdPage);
    d->blurInput2 = new RDoubleNumInput(thirdPage);
    d->blurInput2->input()->setRange(0.0, 1000.0, 1.0, true);
    d->blurInput2->setDefaultValue(500.0);
    d->blurInput2->setObjectName("blurInput2");
    d->blurInput2->setWhatsThis(i18n("<b>Blur</b>: How strong the image is blurred before combining with the original "
                                     "image and with the tonemapping function."));

    grid3->addWidget(d->stageTwo,    0, 0, 1, 2);
    grid3->addWidget(d->label6,      1, 0, 1, 1);
    grid3->addWidget(d->powerInput2, 1, 1, 1, 1);
    grid3->addWidget(d->label7,      2, 0, 1, 1);
    grid3->addWidget(d->blurInput2,  2, 1, 1, 1);
    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QWidget* fourthPage = new QWidget();
    QGridLayout* grid4  = new QGridLayout( fourthPage );

    d->stageThree = new QCheckBox(i18n("Enabled"), fourthPage);
    d->stageThree->setWhatsThis(i18n("Check to enable this stage."));
    d->stageThree->setChecked(false);
    d->stageThree->setObjectName("stageThree");

    // -------------------------------------------------------------

    d->label8      = new QLabel(i18n("Power:"), fourthPage);
    d->powerInput3 = new RDoubleNumInput(fourthPage);
    d->powerInput3->input()->setRange(0.0, 100.0, 1.0, true);
    d->powerInput3->setDefaultValue(50.0);
    d->powerInput3->setObjectName("powerInput3");
    d->powerInput3->setWhatsThis(i18n("<b>Power</b>: How strong the effect is applied."));

    // -------------------------------------------------------------

    d->label9     = new QLabel(i18n("Blur:"), fourthPage);
    d->blurInput3 = new RDoubleNumInput(fourthPage);
    d->blurInput3->input()->setRange(0.0, 1000.0, 1.0, true);
    d->blurInput3->setDefaultValue(500.0);
    d->blurInput3->setObjectName("blurInput3");
    d->blurInput3->setWhatsThis(i18n("<b>Blur</b>: How strong the image is blurred before combining with the original "
                                     "image and with the tonemapping function."));

    grid4->addWidget(d->stageThree,  0, 0, 1, 2);
    grid4->addWidget(d->label8,      1, 0, 1, 1);
    grid4->addWidget(d->powerInput3, 1, 1, 1, 1);
    grid4->addWidget(d->label9,      2, 0, 1, 1);
    grid4->addWidget(d->blurInput3,  2, 1, 1, 1);
    grid4->setMargin(KDialog::spacingHint());
    grid4->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    QWidget* fifthPage = new QWidget();
    QGridLayout* grid5 = new QGridLayout( fifthPage );

    d->stageFour       = new QCheckBox(i18n("Enabled"), fifthPage);
    d->stageFour->setWhatsThis(i18n("Check to enable this stage."));
    d->stageFour->setChecked(false);
    d->stageFour->setObjectName("stageFour");

    // -------------------------------------------------------------

    d->label10     = new QLabel(i18n("Power:"), fifthPage);
    d->powerInput4 = new RDoubleNumInput(fifthPage);
    d->powerInput4->input()->setRange(0.0, 100.0, 1.0, true);
    d->powerInput4->setDefaultValue(50.0);
    d->powerInput4->setObjectName("powerInput4");
    d->powerInput4->setWhatsThis(i18n("<b>Power</b>: How strong the effect is applied."));

    // -------------------------------------------------------------

    d->label11    = new QLabel(i18n("Blur:"), fifthPage);
    d->blurInput4 = new RDoubleNumInput(fifthPage);
    d->blurInput4->input()->setRange(0.0, 1000.0, 1.0, true);
    d->blurInput4->setDefaultValue(500.0);
    d->blurInput4->setObjectName("blurInput4");
    d->blurInput4->setWhatsThis(i18n("<b>Blur</b>: How strong the image is blurred before combining with the original "
                                     "image and with the tonemapping function."));

    grid5->addWidget(d->stageFour,   0, 0, 1, 2);
    grid5->addWidget(d->label10,     1, 0, 1, 1);
    grid5->addWidget(d->powerInput4, 1, 1, 1, 1);
    grid5->addWidget(d->label11,     2, 0, 1, 1);
    grid5->addWidget(d->blurInput4,  2, 1, 1, 1);
    grid5->setMargin(KDialog::spacingHint());
    grid5->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    d->expanderBox = new RExpanderBox;
    d->expanderBox->setObjectName("LocalContrastTool Expander");
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
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->stageOne,SIGNAL(toggled(bool)),
            this, SLOT(slotStage1Enabled(bool)));

    connect(d->stageTwo,SIGNAL(toggled(bool)),
            this, SLOT(slotStage2Enabled(bool)));

    connect(d->stageThree,SIGNAL(toggled(bool)),
            this, SLOT(slotStage3Enabled(bool)));

    connect(d->stageFour,SIGNAL(toggled(bool)),
            this, SLOT(slotStage4Enabled(bool)));
}

LocalContrastSettings::~LocalContrastSettings()
{
    delete d;
}

void LocalContrastSettings::slotStage1Enabled(bool b)
{
    d->label4->setEnabled(b);
    d->powerInput1->setEnabled(b);
    d->label5->setEnabled(b);
    d->blurInput1->setEnabled(b);
}

void LocalContrastSettings::slotStage2Enabled(bool b)
{

    d->label6->setEnabled(b);
    d->powerInput2->setEnabled(b);
    d->label7->setEnabled(b);
    d->blurInput2->setEnabled(b);
}

void LocalContrastSettings::slotStage3Enabled(bool b)
{
    d->label8->setEnabled(b);
    d->powerInput3->setEnabled(b);
    d->label9->setEnabled(b);
    d->blurInput3->setEnabled(b);
}

void LocalContrastSettings::slotStage4Enabled(bool b)
{
    d->label10->setEnabled(b);
    d->powerInput4->setEnabled(b);
    d->label11->setEnabled(b);
    d->blurInput4->setEnabled(b);
}

LocalContrastContainer LocalContrastSettings::settings() const
{
    LocalContrastContainer prm;

    prm.stretch_contrast = d->stretchContrastCheck->isChecked();
    prm.low_saturation   = d->lowSaturationInput->value();
    prm.high_saturation  = d->highSaturationInput->value();
    prm.function_id      = d->functionInput->currentIndex();

    prm.stage[0].enabled = d->stageOne->isChecked();
    prm.stage[0].power   = d->powerInput1->value();
    prm.stage[0].blur    = d->blurInput1->value();

    prm.stage[1].enabled = d->stageTwo->isChecked();
    prm.stage[1].power   = d->powerInput2->value();
    prm.stage[1].blur    = d->blurInput2->value();

    prm.stage[2].enabled = d->stageThree->isChecked();
    prm.stage[2].power   = d->powerInput3->value();
    prm.stage[2].blur    = d->blurInput3->value();

    prm.stage[3].enabled = d->stageFour->isChecked();
    prm.stage[3].power   = d->powerInput4->value();
    prm.stage[3].blur    = d->blurInput4->value();

    return prm;
}

void LocalContrastSettings::setSettings(const LocalContrastContainer& settings)
{
    blockSignals(true);
    d->expanderBox->setEnabled(false);

    d->stretchContrastCheck->setChecked(settings.stretch_contrast);
    d->lowSaturationInput->setValue(settings.low_saturation);
    d->highSaturationInput->setValue(settings.high_saturation);
    d->functionInput->setCurrentIndex(settings.function_id);

    d->stageOne->setChecked(settings.stage[0].enabled);
    d->powerInput1->setValue(settings.stage[0].power);
    d->blurInput1->setValue(settings.stage[0].blur);

    d->stageTwo->setChecked(settings.stage[1].enabled);
    d->powerInput2->setValue(settings.stage[1].power);
    d->blurInput2->setValue(settings.stage[1].blur);

    d->stageThree->setChecked(settings.stage[2].enabled);
    d->powerInput3->setValue(settings.stage[2].power);
    d->blurInput3->setValue(settings.stage[2].blur);

    d->stageFour->setChecked(settings.stage[3].enabled);
    d->powerInput4->setValue(settings.stage[3].power);
    d->blurInput4->setValue(settings.stage[3].blur);

    d->expanderBox->readSettings();
    d->expanderBox->setEnabled(true);

    slotStage1Enabled(d->stageOne->isChecked());
    slotStage2Enabled(d->stageTwo->isChecked());
    slotStage3Enabled(d->stageThree->isChecked());
    slotStage4Enabled(d->stageFour->isChecked());

    blockSignals(false);
}

void LocalContrastSettings::resetToDefault()
{
    blockSignals(true);

    d->stretchContrastCheck->setChecked(false);
    d->lowSaturationInput->slotReset();
    d->highSaturationInput->slotReset();
    d->functionInput->slotReset();

    d->stageOne->setChecked(false);
    d->powerInput1->slotReset();
    d->blurInput1->slotReset();

    d->stageTwo->setChecked(false);
    d->powerInput2->slotReset();
    d->blurInput2->slotReset();

    d->stageThree->setChecked(false);
    d->powerInput3->slotReset();
    d->blurInput3->slotReset();

    d->stageFour->setChecked(false);
    d->powerInput4->slotReset();
    d->blurInput4->slotReset();

    blockSignals(false);
}

LocalContrastContainer LocalContrastSettings::defaultSettings() const
{
    LocalContrastContainer prm;

    prm.stretch_contrast = false;
    prm.low_saturation   = d->lowSaturationInput->defaultValue();
    prm.high_saturation  = d->highSaturationInput->defaultValue();
    prm.function_id      = d->functionInput->defaultIndex();

    prm.stage[0].enabled = false;
    prm.stage[0].power   = d->powerInput1->defaultValue();
    prm.stage[0].blur    = d->blurInput1->defaultValue();

    prm.stage[1].enabled = false;
    prm.stage[1].power   = d->powerInput2->defaultValue();
    prm.stage[1].blur    = d->blurInput2->defaultValue();

    prm.stage[2].enabled = false;
    prm.stage[2].power   = d->powerInput3->defaultValue();
    prm.stage[2].blur    = d->blurInput3->defaultValue();

    prm.stage[3].enabled = false;
    prm.stage[3].power   = d->powerInput4->defaultValue();
    prm.stage[3].blur    = d->blurInput4->defaultValue();

    return prm;
}

void LocalContrastSettings::readSettings(KConfigGroup& group)
{
    LocalContrastContainer prm;
    LocalContrastContainer defaultPrm = defaultSettings();

    prm.stretch_contrast = group.readEntry(d->configStretchContrastEntry, false);
    prm.low_saturation   = group.readEntry(d->configLowSaturationEntry,   d->lowSaturationInput->defaultValue());
    prm.high_saturation  = group.readEntry(d->configHighSaturationEntry,  d->highSaturationInput->defaultValue());
    prm.function_id      = group.readEntry(d->configFunctionInputEntry,   d->functionInput->defaultIndex());

    prm.stage[0].enabled = group.readEntry(d->configStageOneEntry,        false);
    prm.stage[0].power   = group.readEntry(d->configPower1Entry,          d->powerInput1->defaultValue());
    prm.stage[0].blur    = group.readEntry(d->configBlur1Entry,           d->blurInput1->defaultValue());

    prm.stage[1].enabled = group.readEntry(d->configStageTwoEntry,        false);
    prm.stage[1].power   = group.readEntry(d->configPower2Entry,          d->powerInput2->defaultValue());
    prm.stage[1].blur    = group.readEntry(d->configBlur2Entry,           d->blurInput2->defaultValue());

    prm.stage[2].enabled = group.readEntry(d->configStageThreeEntry,      false);
    prm.stage[2].power   = group.readEntry(d->configPower3Entry,          d->powerInput3->defaultValue());
    prm.stage[2].blur    = group.readEntry(d->configBlur3Entry,           d->blurInput3->defaultValue());

    prm.stage[3].enabled = group.readEntry(d->configStageFourEntry,       false);
    prm.stage[3].power   = group.readEntry(d->configPower4Entry,          d->powerInput4->defaultValue());
    prm.stage[3].blur    = group.readEntry(d->configBlur4Entry,           d->blurInput4->defaultValue());

    setSettings(prm);
}

void LocalContrastSettings::writeSettings(KConfigGroup& group)
{
    LocalContrastContainer prm = settings();

    group.writeEntry(d->configStretchContrastEntry, prm.stretch_contrast);
    group.writeEntry(d->configLowSaturationEntry,   prm.low_saturation);
    group.writeEntry(d->configHighSaturationEntry,  prm.high_saturation);
    group.writeEntry(d->configFunctionInputEntry,   prm.function_id);

    group.writeEntry(d->configStageOneEntry,        prm.stage[0].enabled);
    group.writeEntry(d->configPower1Entry,          prm.stage[0].power);
    group.writeEntry(d->configBlur1Entry,           prm.stage[0].blur);

    group.writeEntry(d->configStageTwoEntry,        prm.stage[1].enabled);
    group.writeEntry(d->configPower2Entry,          prm.stage[1].power);
    group.writeEntry(d->configBlur2Entry,           prm.stage[1].blur);

    group.writeEntry(d->configStageThreeEntry,      prm.stage[2].enabled);
    group.writeEntry(d->configPower3Entry,          prm.stage[2].power);
    group.writeEntry(d->configBlur3Entry,           prm.stage[2].blur);

    group.writeEntry(d->configStageFourEntry,       prm.stage[3].enabled);
    group.writeEntry(d->configPower4Entry,          prm.stage[3].power);
    group.writeEntry(d->configBlur4Entry,           prm.stage[3].blur);
    d->expanderBox->writeSettings();
}

void LocalContrastSettings::loadSettings()
{
    KUrl loadFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                    QString( "*" ), kapp->activeWindow(),
                    QString( i18n("Photograph Local Contrast Settings File to Load")) );
    if ( loadFile.isEmpty() )
        return;

    QFile file(loadFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Local Contrast Configuration File" )
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph Local Contrast settings text file.",
                                    loadFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);
        d->stretchContrastCheck->setChecked( stream.readLine().toInt() );
        d->stageOne->setChecked( stream.readLine().toInt() );
        d->stageTwo->setChecked( stream.readLine().toInt() );
        d->stageThree->setChecked( stream.readLine().toInt() );
        d->stageFour->setChecked( stream.readLine().toInt() );
        d->lowSaturationInput->setValue( stream.readLine().toInt() );
        d->highSaturationInput->setValue( stream.readLine().toInt() );
        d->functionInput->setCurrentIndex( stream.readLine().toInt() );
        d->powerInput1->setValue( stream.readLine().toDouble() );
        d->blurInput1->setValue( stream.readLine().toDouble() );
        d->powerInput2->setValue( stream.readLine().toDouble() );
        d->blurInput2->setValue( stream.readLine().toDouble() );
        d->powerInput3->setValue( stream.readLine().toDouble() );
        d->blurInput3->setValue( stream.readLine().toDouble() );
        d->powerInput4->setValue( stream.readLine().toDouble() );
        d->blurInput4->setValue( stream.readLine().toDouble() );
        blockSignals(false);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the Photograph Local Contrast text file."));
    }

    file.close();
}

void LocalContrastSettings::saveAsSettings()
{
    KUrl saveFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                    QString( "*" ), kapp->activeWindow(),
                    QString( i18n("Photograph Local Contrast Settings File to Save")) );
    if ( saveFile.isEmpty() )
        return;

    QFile file(saveFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Photograph Local Contrast Configuration File\n";

        stream << d->stretchContrastCheck->isChecked() << "\n";
        stream << d->stageOne->isChecked() << "\n";
        stream << d->stageTwo->isChecked() << "\n";
        stream << d->stageThree->isChecked() << "\n";
        stream << d->stageFour->isChecked() << "\n";
        stream << d->lowSaturationInput->value() << "\n";
        stream << d->highSaturationInput->value() << "\n";
        stream << d->functionInput->currentIndex() << "\n";
        stream << d->powerInput1->value() << "\n";
        stream << d->blurInput1->value() << "\n";
        stream << d->powerInput2->value() << "\n";
        stream << d->blurInput2->value() << "\n";
        stream << d->powerInput3->value() << "\n";
        stream << d->blurInput3->value() << "\n";
        stream << d->powerInput4->value() << "\n";
        stream << d->blurInput4->value() << "\n";
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the Photograph Local Contrast text file."));
    }

    file.close();
}

}  // namespace Digikam
