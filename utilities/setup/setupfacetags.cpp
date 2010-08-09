/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-10
 * Description : setup tab for face tags
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include "setupfacetags.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QSlider>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>

namespace Digikam
{

class SetupFaceTagsPriv
{
public:

    SetupFaceTagsPriv():
        configGroupName("Face Tags Settings"),
        configFaceDetectionEntry("FaceDetection"),
        configFaceSuggestionEntry("FaceSuggestion"),
        configDetectionAccuracyEntry("DetectionAccuracy"),
        configSuggestionThresholdEntry("SuggestionThreshold")
    {
        enableFaceDetection = 0;
        enableFaceSuggestions = 0;
        detectionAccuracySlider = 0;
        suggestionThresholdSlider = 0;
    }

    const QString configGroupName;
    const QString configFaceDetectionEntry;
    const QString configFaceSuggestionEntry;
    const QString configDetectionAccuracyEntry;
    const QString configSuggestionThresholdEntry;
    
    QCheckBox*    enableFaceDetection;
    QCheckBox*    enableFaceSuggestions;
    QSlider*      detectionAccuracySlider;
    QSlider*      suggestionThresholdSlider;
};

SetupFaceTags::SetupFaceTags(QWidget* parent): QScrollArea(parent), d(new SetupFaceTagsPriv)
{
    QWidget *panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout *layout = new QVBoxLayout(panel);

    d->detectionAccuracySlider = new QSlider(Qt::Horizontal, panel);
    d->suggestionThresholdSlider = new QSlider(Qt::Horizontal, panel);
    d->detectionAccuracySlider->setWhatsThis( i18n("The accuracy of face detection."
                                             "If you have a weak computer, it is a good idea to choose a lower value."
                                             "Choosing a higher value will increase the accuracy of face detection,"
                                             "but will be slow."));
    
    d->enableFaceDetection = new QCheckBox(i18n("Enable face detection"), panel);
    d->enableFaceDetection->setWhatsThis( i18n("If this option is enabled, digiKam will search for faces in your images,"
                                               "thus making it easier to tag people in your photographs."));

    d->enableFaceSuggestions = new QCheckBox(i18n("Enable face suggestion"), panel);
    d->enableFaceSuggestions->setWhatsThis( i18n("If this option is enabled, digiKam will try to identify detected faces,"
                                                 "and present you with suggestions of similar faces,"
                                                 "thus making person tagging even faster."));

    
    layout->addWidget(d->enableFaceDetection);
    layout->addWidget(d->detectionAccuracySlider);
    layout->addWidget(d->enableFaceSuggestions);
    layout->addWidget(d->suggestionThresholdSlider);
    
    layout->addStretch();
    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());

    readSettings();
    
    connect(d->enableFaceDetection, SIGNAL(stateChanged(int)),
            this, SLOT(updateDetection(int)) );
    
    connect(d->enableFaceSuggestions, SIGNAL(stateChanged(int)),
            this, SLOT(updateSuggestion(int)) );
    
    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupFaceTags::~SetupFaceTags()
{
    delete d;
}

void SetupFaceTags::updateDetection(int value)
{
    if(!d->enableFaceDetection->isChecked())
    {
        d->detectionAccuracySlider->setEnabled(false);
        d->enableFaceSuggestions->setCheckState(Qt::Unchecked);
        d->enableFaceSuggestions->setEnabled(false);
        d->suggestionThresholdSlider->setEnabled(false);
    }
    
    else
    {
        d->detectionAccuracySlider->setEnabled(true);
        d->enableFaceSuggestions->setCheckState(Qt::Checked);
        d->enableFaceSuggestions->setEnabled(true);
        d->suggestionThresholdSlider->setEnabled(true);
    }
}

void SetupFaceTags::updateSuggestion(int value)
{
    if(!d->enableFaceSuggestions->isChecked())
        d->suggestionThresholdSlider->setEnabled(false);
    else
        d->suggestionThresholdSlider->setEnabled(true);
}



void SetupFaceTags::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configFaceDetectionEntry,       d->enableFaceDetection->isChecked());
    group.writeEntry(d->configFaceSuggestionEntry,      d->enableFaceSuggestions->isChecked());
    group.writeEntry(d->configDetectionAccuracyEntry,   d->detectionAccuracySlider->value());
    group.writeEntry(d->configSuggestionThresholdEntry, d->suggestionThresholdSlider->value());

    config->sync();
}

void SetupFaceTags::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    
    d->enableFaceDetection->setChecked(group.readEntry(d->configFaceDetectionEntry,                 true));
    d->enableFaceSuggestions->setChecked(group.readEntry(d->configFaceSuggestionEntry,              true));
    d->detectionAccuracySlider->setValue(group.readEntry(d->configDetectionAccuracyEntry,           4));
    d->suggestionThresholdSlider->setValue(group.readEntry(d->configSuggestionThresholdEntry,       0.8));
}

}   // namespace Digikam
