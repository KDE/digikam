/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-10
 * Description : setup tab for face tags
 *
 * Copyright (C) 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupfacetags.moc"

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

class SetupFaceTags::Private
{
public:

    Private():
        configGroupName("Face Tags Settings"),
        configFaceDetectionEntry("FaceDetection"),
        configFaceSuggestionEntry("FaceSuggestion"),
        configDetectionAccuracyEntry("DetectionAccuracy"),
        configSuggestionThresholdEntry("SuggestionThreshold")
    {
        enableFaceDetection       = 0;
        enableFaceSuggestions     = 0;
        detectionAccuracySlider   = 0;
        suggestionThresholdSlider = 0;
        detectionCBLabel          = 0;
        suggestionCBLabel         = 0;
        detectionSliderLabel      = 0;;
        suggestionSliderLabel     = 0;;
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

    QLabel*       detectionCBLabel;
    QLabel*       suggestionCBLabel;
    QLabel*       detectionSliderLabel;
    QLabel*       suggestionSliderLabel;
};

SetupFaceTags::SetupFaceTags(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* layout = new QVBoxLayout(panel);

    d->detectionSliderLabel = new QLabel;
    d->detectionSliderLabel->setTextFormat(Qt::PlainText);
    d->detectionSliderLabel->setText(i18n("The accuracy of face detection.\n"
                                          "If you have a slow computer, it is a good idea to choose a lower value.\n"
                                          "Choosing a higher value will increase the accuracy of face detection,\n"
                                          "but will be slow.\n"));

    d->detectionAccuracySlider = new QSlider(Qt::Horizontal, panel);
    d->detectionAccuracySlider->setRange(1, 5);
    d->detectionAccuracySlider->setTickInterval(1);
    d->detectionAccuracySlider->setPageStep(1);
    d->detectionAccuracySlider->setTickPosition(QSlider::TicksBelow);

    d->suggestionSliderLabel = new QLabel;
    d->suggestionSliderLabel->setTextFormat(Qt::PlainText);
    d->suggestionSliderLabel->setText(i18n("The threshold of face suggestions.\n"
                                           "A larger suggestion threshold means that fewer suggestions will be presented,\n"
                                           "however these will be more accurate.\n"));

    d->suggestionThresholdSlider = new QSlider(Qt::Horizontal, panel);
    d->suggestionThresholdSlider->setRange(1, 10);
    d->suggestionThresholdSlider->setTickInterval(1);
    d->suggestionThresholdSlider->setPageStep(1);
    d->suggestionThresholdSlider->setTickPosition(QSlider::TicksBelow);

    d->detectionCBLabel    = new QLabel;
    d->detectionCBLabel->setTextFormat(Qt::PlainText);
    d->detectionCBLabel->setText(i18n("If this option is enabled, digiKam will search for faces in your images,\n"
                                      "thus making it easier to tag people in your photographs.\n"));
    d->enableFaceDetection = new QCheckBox(i18n("Enable face detection"), panel);

    d->suggestionCBLabel   = new QLabel;
    d->suggestionCBLabel->setTextFormat(Qt::PlainText);
    d->suggestionCBLabel->setText(i18n("If this option is enabled, digiKam will try to identify detected faces,\n"
                                       "and present you with suggestions of similar faces,\n"
                                       "thus making person tagging even faster.\n"));
    d->enableFaceSuggestions = new QCheckBox(i18n("Enable face suggestion"), panel);
    d->enableFaceSuggestions->setCheckState(Qt::Unchecked);

    layout->addWidget(d->detectionCBLabel);
    layout->addWidget(d->enableFaceDetection);
    layout->addSpacing(20);
    layout->addWidget(d->detectionSliderLabel);
    layout->addWidget(d->detectionAccuracySlider);
    layout->addSpacing(20);
    layout->addWidget(d->suggestionCBLabel);
    layout->addWidget(d->enableFaceSuggestions);
    layout->addSpacing(20);
    layout->addWidget(d->suggestionSliderLabel);
    layout->addWidget(d->suggestionThresholdSlider);
    layout->addStretch();
    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());

    connect(d->enableFaceDetection, SIGNAL(stateChanged(int)),
            this, SLOT(updateDetection(int)));

    connect(d->enableFaceSuggestions, SIGNAL(stateChanged(int)),
            this, SLOT(updateSuggestion(int)));

    readSettings();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupFaceTags::~SetupFaceTags()
{
    delete d;
}

void SetupFaceTags::updateDetection(int /*value*/)
{
    if (!d->enableFaceDetection->isChecked())
    {
        d->detectionAccuracySlider->setEnabled(false);
        d->enableFaceSuggestions->setCheckState(Qt::Unchecked);
        d->enableFaceSuggestions->setEnabled(false);
        d->suggestionThresholdSlider->setEnabled(false);

        d->detectionSliderLabel->setEnabled(false);
        d->suggestionCBLabel->setEnabled(false);
        d->suggestionSliderLabel->setEnabled(false);
    }
    else
    {
        d->detectionAccuracySlider->setEnabled(true);
        d->enableFaceSuggestions->setCheckState(Qt::Checked);
        d->enableFaceSuggestions->setEnabled(true);
        d->suggestionThresholdSlider->setEnabled(true);

        d->detectionSliderLabel->setEnabled(true);
        d->suggestionCBLabel->setEnabled(true);
        d->suggestionSliderLabel->setEnabled(true);
    }
}

void SetupFaceTags::updateSuggestion(int /*value*/)
{
    if (!d->enableFaceSuggestions->isChecked())
    {
        d->suggestionThresholdSlider->setEnabled(false);
        d->suggestionSliderLabel->setEnabled(false);
    }
    else
    {
        d->suggestionThresholdSlider->setEnabled(true);
        d->suggestionSliderLabel->setEnabled(true);
    }
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

    d->enableFaceDetection->setChecked(group.readEntry(d->configFaceDetectionEntry,           true));
    d->enableFaceSuggestions->setChecked(group.readEntry(d->configFaceSuggestionEntry,        false));
    d->detectionAccuracySlider->setValue(group.readEntry(d->configDetectionAccuracyEntry,     3));
    d->suggestionThresholdSlider->setValue(group.readEntry(d->configSuggestionThresholdEntry, 0.8));
}

}   // namespace Digikam
