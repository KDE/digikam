/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * Copyright (C) 2007-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupdcraw.moc"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <KTabWidget>

// LibKDcraw includes

#include <libkdcraw/dcrawsettingswidget.h>
#include <libkdcraw/version.h>

// Local includes

#include "drawdecoding.h"

using namespace KDcrawIface;

namespace Digikam
{

class SetupDcraw::SetupDcrawPriv
{
public:


    SetupDcrawPriv() :
        tab(0),
        behaviorPanel(0),
        settingsPanel(0),
        openSimple(0),
        openDefault(0),
        openTool(0),
        dcrawSettings(0)
    {
    }

    static const QString  configGroupName;
    static const QString  configUseRawImportToolEntry;

    KTabWidget*           tab;

    QWidget*              behaviorPanel;
    QWidget*              settingsPanel;

    QRadioButton*         openSimple;
    QRadioButton*         openDefault;
    QRadioButton*         openTool;

    DcrawSettingsWidget*  dcrawSettings;
};
const QString SetupDcraw::SetupDcrawPriv::configGroupName("ImageViewer Settings");
const QString SetupDcraw::SetupDcrawPriv::configUseRawImportToolEntry("UseRawImportTool");

SetupDcraw::SetupDcraw(QWidget* parent)
    : QScrollArea(parent), d(new SetupDcrawPriv)
{
    d->tab = new KTabWidget;

    // --------------------------------------------------------

    d->behaviorPanel = new QWidget;
    QVBoxLayout* behaviorLayout = new QVBoxLayout;

    QLabel *rawExplanation = new QLabel;
    rawExplanation->setText(i18nc("@info",
                                  "A <emphasis>raw image file</emphasis> contains minimally processed data "
                                  "from the image sensor of a digital camera.<nl/>"
                                  "Opening a raw file requires extensive data interpretation and processing."));
    rawExplanation->setWordWrap(true);
    QLabel *rawIcon = new QLabel;
    rawIcon->setPixmap(SmallIcon("camera-photo", KIconLoader::SizeLarge));
    QHBoxLayout* header = new QHBoxLayout;
    header->addWidget(rawIcon);
    header->addWidget(rawExplanation);
    header->setStretchFactor(rawExplanation, 10);
    header->addStretch(1);

    QGroupBox* behaviorBox = new QGroupBox;
    QGridLayout* boxLayout = new QGridLayout;

    QLabel *openIcon = new QLabel;
    openIcon->setPixmap(SmallIcon("document-open", KIconLoader::SizeMedium));

    QLabel *openIntro = new QLabel(i18nc("@label", "Open raw files in the image editor"));

    d->openSimple  = new QRadioButton(i18nc("@option:radio Open raw files...",
                                            "Fast and simple, as 8 bit image"));
    d->openDefault = new QRadioButton(i18nc("@option:radio Open raw files...",
                                            "Using the default settings, in 16 bit"));
    d->openTool    = new QRadioButton(i18nc("@option:radio Open raw files...",
                                            "Always open the Raw Import Tool to customize settings"));

    boxLayout->addWidget(openIcon,       0, 0);
    boxLayout->addWidget(openIntro,      0, 1);
    boxLayout->addWidget(d->openSimple,  1, 0, 1, 3);
    boxLayout->addWidget(d->openDefault, 2, 0, 1, 3);
    boxLayout->addWidget(d->openTool,    3, 0, 1, 3);
    boxLayout->setColumnStretch(2, 1);
    behaviorBox->setLayout(boxLayout);

    behaviorLayout->addLayout(header);
    behaviorLayout->addWidget(behaviorBox);
    behaviorLayout->addStretch();
    d->behaviorPanel->setLayout(behaviorLayout);

    // --------------------------------------------------------

    d->settingsPanel = new QWidget;
    QVBoxLayout* settingsLayout = new QVBoxLayout;

    d->dcrawSettings    = new DcrawSettingsWidget(0, 0 /* no advanced settings shown */);
    d->dcrawSettings->setItemIcon(0, SmallIcon("kdcraw"));
    d->dcrawSettings->setItemIcon(1, SmallIcon("whitebalance"));
    d->dcrawSettings->setItemIcon(2, SmallIcon("lensdistortion"));

    settingsLayout->addWidget(d->dcrawSettings);
    d->settingsPanel->setLayout(settingsLayout);

    // --------------------------------------------------------

    d->tab->addTab(d->behaviorPanel, i18nc("@title:tab", "Behavior"));
    d->tab->addTab(d->settingsPanel, i18nc("@title:tab", "Default Settings"));

    setWidget(d->tab);
    setWidgetResizable(true);

    // --------------------------------------------------------

    connect(d->openSimple, SIGNAL(toggled(bool)),
            this, SLOT(slotBehaviorChanged()));

    connect(d->openDefault, SIGNAL(toggled(bool)),
            this, SLOT(slotBehaviorChanged()));

    connect(d->openTool, SIGNAL(toggled(bool)),
            this, SLOT(slotBehaviorChanged()));

    connect(d->dcrawSettings, SIGNAL(signalSixteenBitsImageToggled(bool)),
            this, SLOT(slotSixteenBitsImageToggled(bool)));

    readSettings();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
}

SetupDcraw::~SetupDcraw()
{
    delete d;
}

void SetupDcraw::slotSixteenBitsImageToggled(bool)
{
    // Dcraw do not provide a way to set brightness of image in 16 bits color depth.
    // We always set on this option. We drive brightness adjustment in digiKam Raw image loader.
    d->dcrawSettings->setEnabledBrightnessSettings(true);
}

void SetupDcraw::slotBehaviorChanged()
{
    RawDecodingSettings settings = d->dcrawSettings->settings();
    settings.sixteenBitsImage = !d->openSimple->isChecked();
    d->dcrawSettings->setSettings(settings);
}

void SetupDcraw::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configUseRawImportToolEntry, d->openTool->isChecked());

    d->dcrawSettings->writeSettings(group);

    config->sync();
}

void SetupDcraw::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->dcrawSettings->readSettings(group);

    bool useTool = group.readEntry(d->configUseRawImportToolEntry, false);
    if (useTool)
        d->openTool->setChecked(true);
    else
    {
        if (d->dcrawSettings->settings().sixteenBitsImage)
            d->openDefault->setChecked(true);
        else
            d->openSimple->setChecked(true);
    }
}

}  // namespace Digikam
