/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust settings widget.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "timeadjustsettings.h"

// Qt includes

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolButton>
#include <QTimeEdit>
#include <QComboBox>
#include <QPointer>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "clockphotodialog.h"
#include "dmetadata.h"
#include "detbyclockphotobutton.h"

namespace Digikam
{

class TimeAdjustSettings::Private
{

public:

    Private()
    {
        useSettingsBox         = 0;
        adjustSettingsBox      = 0;
        updateSettingsBox      = 0;
        useButtonGroup         = 0;
        useApplDateBtn         = 0;
        useFileDateBtn         = 0;
        useMetaDateBtn         = 0;
        useCustomDateBtn       = 0;
        updFileModDateCheck    = 0;
        updEXIFModDateCheck    = 0;
        updEXIFOriDateCheck    = 0;
        updEXIFDigDateCheck    = 0;
        updEXIFThmDateCheck    = 0;
        updIPTCDateCheck       = 0;
        updXMPDateCheck        = 0;
        useFileDateTypeChooser = 0;
        useMetaDateTypeChooser = 0;
        adjTypeChooser         = 0;
        useApplDateLbl         = 0;
        adjDaysLabel           = 0;
        adjDaysInput           = 0;
        adjDetByClockPhotoBtn  = 0;
        useCustDateInput       = 0;
        useCustTimeInput       = 0;
        adjTimeInput           = 0;
        useCustomDateTodayBtn  = 0;
        settingsExpander       = 0;
    }

    QWidget*               useSettingsBox;
    QWidget*               adjustSettingsBox;
    QWidget*               updateSettingsBox;

    QButtonGroup*          useButtonGroup;

    QRadioButton*          useApplDateBtn;
    QRadioButton*          useFileDateBtn;
    QRadioButton*          useMetaDateBtn;
    QRadioButton*          useCustomDateBtn;

    QCheckBox*             updFileModDateCheck;
    QCheckBox*             updEXIFModDateCheck;
    QCheckBox*             updEXIFOriDateCheck;
    QCheckBox*             updEXIFDigDateCheck;
    QCheckBox*             updEXIFThmDateCheck;
    QCheckBox*             updIPTCDateCheck;
    QCheckBox*             updXMPDateCheck;

    QComboBox*             useFileDateTypeChooser;
    QComboBox*             useMetaDateTypeChooser;
    QComboBox*             adjTypeChooser;

    QLabel*                useApplDateLbl;
    QLabel*                adjDaysLabel;

    QSpinBox*              adjDaysInput;

    DetByClockPhotoButton* adjDetByClockPhotoBtn;

    QDateEdit*             useCustDateInput;

    QTimeEdit*             useCustTimeInput;
    QTimeEdit*             adjTimeInput;

    QToolButton*           useCustomDateTodayBtn;

    DExpanderBox*          settingsExpander;
};

TimeAdjustSettings::TimeAdjustSettings(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel    = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    d->settingsExpander     = new DExpanderBox(panel);
    d->settingsExpander->setObjectName(QLatin1String("Time Adjust Settings Expander"));
    QVBoxLayout* const vlay = new QVBoxLayout(panel);
    vlay->addWidget(d->settingsExpander);

    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    // -- Settings View Used Timestamps ---------------------------------------------------------

    d->useSettingsBox              = new QWidget(d->settingsExpander);
    QGridLayout* const useGBLayout = new QGridLayout(d->useSettingsBox);
    d->useButtonGroup              = new QButtonGroup(d->useSettingsBox);
    d->useButtonGroup->setExclusive(true);

    QString applDateLabelString = i18n("%1 timestamp", QApplication::applicationName());
    d->useApplDateBtn           = new QRadioButton(QString(), d->useSettingsBox);
    d->useApplDateLbl           = new QLabel(applDateLabelString);
    d->useApplDateLbl->setIndent(5);

    d->useFileDateBtn           = new QRadioButton(d->useSettingsBox);
    d->useFileDateTypeChooser   = new QComboBox(d->useSettingsBox);
    d->useFileDateTypeChooser->insertItem(TimeAdjustContainer::FILELASTMOD, i18n("File last modified"));
    /*
    // NOTE: not supported by Linux, although supported by Qt (read-only)
    d->useFileDateTypeChooser->insertItem(TimeAdjustContainer::FILECREATED, i18n("File created"));
    */

    d->useMetaDateBtn         = new QRadioButton(QString(), d->useSettingsBox);
    d->useMetaDateTypeChooser = new QComboBox(d->useSettingsBox);
    d->useMetaDateTypeChooser->insertItem(TimeAdjustContainer::EXIFIPTCXMP,   i18n("EXIF/IPTC/XMP"));
    d->useMetaDateTypeChooser->insertItem(TimeAdjustContainer::EXIFCREATED,   i18n("EXIF: created"));
    d->useMetaDateTypeChooser->insertItem(TimeAdjustContainer::EXIFORIGINAL,  i18n("EXIF: original"));
    d->useMetaDateTypeChooser->insertItem(TimeAdjustContainer::EXIFDIGITIZED, i18n("EXIF: digitized"));
    d->useMetaDateTypeChooser->insertItem(TimeAdjustContainer::IPTCCREATED,   i18n("IPTC: created"));
    d->useMetaDateTypeChooser->insertItem(TimeAdjustContainer::XMPCREATED,    i18n("XMP: created"));

    d->useCustomDateBtn       = new QRadioButton(d->useSettingsBox);
    d->useCustDateInput       = new QDateEdit(d->useSettingsBox);
    d->useCustDateInput->setDisplayFormat(QLatin1String("dd MMMM yyyy"));
    d->useCustDateInput->setCalendarPopup(true);
    d->useCustTimeInput       = new QTimeEdit(d->useSettingsBox);
    d->useCustTimeInput->setDisplayFormat(QLatin1String("hh:mm:ss"));
    d->useCustomDateTodayBtn  = new QToolButton(d->useSettingsBox);
    d->useCustomDateTodayBtn->setIcon(QIcon::fromTheme(QLatin1String("go-jump-today")));
    d->useCustomDateTodayBtn->setToolTip(i18n("Reset to current date"));

    useGBLayout->addWidget(d->useApplDateBtn,         0, 0, 1, 1);
    useGBLayout->addWidget(d->useApplDateLbl,         0, 1, 1, 1);
    useGBLayout->addWidget(d->useFileDateBtn,         1, 0, 1, 1);
    useGBLayout->addWidget(d->useFileDateTypeChooser, 1, 1, 1, 1);
    useGBLayout->addWidget(d->useMetaDateBtn,         2, 0, 1, 1);
    useGBLayout->addWidget(d->useMetaDateTypeChooser, 2, 1, 1, 1);
    useGBLayout->addWidget(d->useCustomDateBtn,       3, 0, 1, 1);
    useGBLayout->addWidget(d->useCustDateInput,       3, 1, 1, 1);
    useGBLayout->addWidget(d->useCustTimeInput,       3, 2, 1, 1);
    useGBLayout->addWidget(d->useCustomDateTodayBtn,  3, 3, 1, 1);
    useGBLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    useGBLayout->setSpacing(spacing);
    useGBLayout->setColumnStretch(1, 1);
    useGBLayout->setColumnStretch(2, 1);

    d->useButtonGroup->addButton(d->useApplDateBtn,   0);
    d->useButtonGroup->addButton(d->useFileDateBtn,   1);
    d->useButtonGroup->addButton(d->useMetaDateBtn,   2);
    d->useButtonGroup->addButton(d->useCustomDateBtn, 3);
    d->useApplDateBtn->setChecked(true);

    // -- Settings View TimesStamp Adjustments ---------------------------------------------------

    d->adjustSettingsBox              = new QWidget(d->settingsExpander);
    QGridLayout* const adjustGBLayout = new QGridLayout(d->adjustSettingsBox);

    d->adjTypeChooser           = new QComboBox(d->adjustSettingsBox);
    d->adjTypeChooser->insertItem(TimeAdjustContainer::COPYVALUE, i18nc("copy timestamp as well",             "Copy value"));
    d->adjTypeChooser->insertItem(TimeAdjustContainer::ADDVALUE,  i18nc("add a fixed timestamp to date",      "Add"));
    d->adjTypeChooser->insertItem(TimeAdjustContainer::SUBVALUE,  i18nc("subtract a fixed timestamp to date", "Subtract"));
    d->adjDaysInput             = new QSpinBox(d->adjustSettingsBox);
    d->adjDaysInput->setRange(0, 9999);
    d->adjDaysInput->setSingleStep(1);
    d->adjDaysLabel             = new QLabel(i18nc("time adjust offset, days value label", "days"), d->adjustSettingsBox);
    d->adjTimeInput             = new QTimeEdit(d->adjustSettingsBox);
    d->adjTimeInput->setDisplayFormat(QLatin1String("hh:mm:ss"));
    d->adjDetByClockPhotoBtn    = new DetByClockPhotoButton(i18n("Determine difference from clock photo"));
    d->adjDetByClockPhotoBtn->setToolTip(i18n("Either click or drag'n drop a photo on the button to choose a clock photo"));

    adjustGBLayout->addWidget(d->adjTypeChooser,        0, 0, 1, 1);
    adjustGBLayout->addWidget(d->adjDaysInput,          0, 1, 1, 1);
    adjustGBLayout->addWidget(d->adjDaysLabel,          0, 2, 1, 1);
    adjustGBLayout->addWidget(d->adjTimeInput,          0, 3, 1, 1);
    adjustGBLayout->addWidget(d->adjDetByClockPhotoBtn, 1, 0, 1, 4);
    adjustGBLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    adjustGBLayout->setSpacing(spacing);
    adjustGBLayout->setColumnStretch(0, 1);
    adjustGBLayout->setColumnStretch(1, 1);
    adjustGBLayout->setColumnStretch(3, 1);

    // -- Settings View Updated Timestamps -------------------------------------------------------

    d->updateSettingsBox              = new QWidget(d->settingsExpander);
    QGridLayout* const updateGBLayout = new QGridLayout(d->updateSettingsBox);

    d->updFileModDateCheck      = new QCheckBox(i18n("File last modified"), d->updateSettingsBox);
    d->updEXIFModDateCheck      = new QCheckBox(i18n("EXIF: created"),      d->updateSettingsBox);
    d->updEXIFOriDateCheck      = new QCheckBox(i18n("EXIF: original"),     d->updateSettingsBox);
    d->updEXIFDigDateCheck      = new QCheckBox(i18n("EXIF: digitized"),    d->updateSettingsBox);
    d->updEXIFThmDateCheck      = new QCheckBox(i18n("EXIF: Thumbnail"),    d->updateSettingsBox);
    d->updIPTCDateCheck         = new QCheckBox(i18n("IPTC: created"),      d->updateSettingsBox);
    d->updXMPDateCheck          = new QCheckBox(i18n("XMP"),                d->updateSettingsBox);

    updateGBLayout->addWidget(d->updEXIFModDateCheck, 0, 1, 1, 1);
    updateGBLayout->addWidget(d->updEXIFOriDateCheck, 1, 0, 1, 1);
    updateGBLayout->addWidget(d->updEXIFDigDateCheck, 1, 1, 1, 1);
    updateGBLayout->addWidget(d->updEXIFThmDateCheck, 2, 0, 1, 1);
    updateGBLayout->addWidget(d->updXMPDateCheck,     2, 1, 1, 1);
    updateGBLayout->addWidget(d->updIPTCDateCheck,    3, 0, 1, 1);
    updateGBLayout->addWidget(d->updFileModDateCheck, 3, 1, 1, 1);
    updateGBLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    updateGBLayout->setSpacing(spacing);
    updateGBLayout->setColumnStretch(0, 1);
    updateGBLayout->setColumnStretch(1, 1);

    if (!DMetadata::supportXmp())
    {
        d->updXMPDateCheck->setEnabled(false);
    }

    // -----------------------------------------------------------------------

    d->settingsExpander->addItem(d->useSettingsBox,    i18n("Timestamp Used"),        QLatin1String("timestampused"),        true);
    d->settingsExpander->addItem(d->adjustSettingsBox, i18n("Timestamp Adjustments"), QLatin1String("timestampadjustments"), true);
    d->settingsExpander->addItem(d->updateSettingsBox, i18n("Timestamp Updated"),     QLatin1String("timestampupdated"),     true);
    d->settingsExpander->addStretch();
    d->settingsExpander->setItemIcon(0, QIcon::fromTheme(QLatin1String("document-import")));
    d->settingsExpander->setItemIcon(1, QIcon::fromTheme(QLatin1String("document-edit")));
    d->settingsExpander->setItemIcon(2, QIcon::fromTheme(QLatin1String("document-export")));

    // -- Settings View Slots/Signals ----------------------------------------

    connect(d->useButtonGroup, SIGNAL(buttonReleased(int)),
            this, SLOT(slotSrcTimestampChanged()));

    connect(d->useFileDateTypeChooser, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSrcTimestampChanged()));

    connect(d->useMetaDateTypeChooser, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSrcTimestampChanged()));

    connect(d->adjTypeChooser, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotAdjustmentTypeChanged()));

    connect(d->useCustomDateTodayBtn, SIGNAL(clicked()),
            this, SLOT(slotResetDateToCurrent()));

    connect(d->adjDetByClockPhotoBtn, SIGNAL(clicked()),
            this, SLOT(slotDetAdjustmentByClockPhotoDialog()));

    connect(d->adjDetByClockPhotoBtn, SIGNAL(signalClockPhotoDropped(QUrl)),
            this, SLOT(slotDetAdjustmentByClockPhotoUrl(QUrl)));

    connect(d->useCustDateInput, SIGNAL(dateChanged(QDate)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->useCustTimeInput, SIGNAL(timeChanged(QTime)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->adjDaysInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->adjTimeInput, SIGNAL(timeChanged(QTime)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->updEXIFModDateCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->updEXIFOriDateCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->updEXIFDigDateCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->updEXIFThmDateCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->updXMPDateCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->updIPTCDateCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->updFileModDateCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));
}

TimeAdjustSettings::~TimeAdjustSettings()
{
    delete d;
}

void TimeAdjustSettings::setSettings(const TimeAdjustContainer& settings)
{
    //d->settingsExpander->readSettings(group);

    int useTimestampType = settings.dateSource;
    if      (useTimestampType == TimeAdjustContainer::APPDATE)      d->useApplDateBtn->setChecked(true);
    else if (useTimestampType == TimeAdjustContainer::FILEDATE)     d->useFileDateBtn->setChecked(true);
    else if (useTimestampType == TimeAdjustContainer::METADATADATE) d->useMetaDateBtn->setChecked(true);
    else if (useTimestampType == TimeAdjustContainer::CUSTOMDATE)   d->useCustomDateBtn->setChecked(true);

    d->useFileDateTypeChooser->setCurrentIndex(settings.fileDateSource);
    d->useMetaDateTypeChooser->setCurrentIndex(settings.metadataSource);
    d->useCustDateInput->setDateTime(settings.customDate);
    d->useCustTimeInput->setDateTime(settings.customTime);

    d->adjTypeChooser->setCurrentIndex(settings.adjustmentType);
    d->adjDaysInput->setValue(settings.adjustmentDays);
    d->adjTimeInput->setDateTime(settings.adjustmentTime);

    d->updFileModDateCheck->setChecked(settings.updFileModDate);
    d->updEXIFModDateCheck->setChecked(settings.updEXIFModDate);
    d->updEXIFOriDateCheck->setChecked(settings.updEXIFOriDate);
    d->updEXIFDigDateCheck->setChecked(settings.updEXIFDigDate);
    d->updEXIFThmDateCheck->setChecked(settings.updEXIFThmDate);
    d->updIPTCDateCheck->setChecked(settings.updIPTCDate);
    d->updXMPDateCheck->setChecked(settings.updXMPDate);

    slotSrcTimestampChanged();
    slotAdjustmentTypeChanged();
}

TimeAdjustContainer TimeAdjustSettings::settings() const
{
    TimeAdjustContainer settings;

    settings.customDate     = d->useCustDateInput->dateTime();
    settings.customTime     = d->useCustTimeInput->dateTime();

    settings.adjustmentType = d->adjTypeChooser->currentIndex();
    settings.adjustmentDays = d->adjDaysInput->value();
    settings.adjustmentTime = d->adjTimeInput->dateTime();

    settings.updEXIFModDate = d->updEXIFModDateCheck->isChecked();
    settings.updEXIFOriDate = d->updEXIFOriDateCheck->isChecked();
    settings.updEXIFDigDate = d->updEXIFDigDateCheck->isChecked();
    settings.updEXIFThmDate = d->updEXIFThmDateCheck->isChecked();
    settings.updIPTCDate    = d->updIPTCDateCheck->isChecked();
    settings.updXMPDate     = d->updXMPDateCheck->isChecked();
    settings.updFileModDate = d->updFileModDateCheck->isChecked();
    settings.dateSource     = TimeAdjustContainer::APPDATE;

    if (d->useFileDateBtn->isChecked())   settings.dateSource = TimeAdjustContainer::FILEDATE;
    if (d->useMetaDateBtn->isChecked())   settings.dateSource = TimeAdjustContainer::METADATADATE;
    if (d->useCustomDateBtn->isChecked()) settings.dateSource = TimeAdjustContainer::CUSTOMDATE;

    settings.metadataSource = d->useMetaDateTypeChooser->currentIndex();
    settings.fileDateSource = d->useFileDateTypeChooser->currentIndex();

    return settings;
}

void TimeAdjustSettings::detAdjustmentByClockPhotoUrl(const QUrl& url)
{
    /* When user press the clock photo button, a dialog is displayed and set the
    *  results to the proper widgets.
    */
    QPointer<ClockPhotoDialog> dlg = new ClockPhotoDialog(this, url);
    const int result               = dlg->exec();

    if (result == QDialog::Accepted)
    {
        DeltaTime dvalues = dlg->deltaValues();

        if (dvalues.isNull())
        {
            d->adjTypeChooser->setCurrentIndex(TimeAdjustContainer::COPYVALUE);
        }
        else if (dvalues.deltaNegative)
        {
            d->adjTypeChooser->setCurrentIndex(TimeAdjustContainer::SUBVALUE);
        }
        else
        {
            d->adjTypeChooser->setCurrentIndex(TimeAdjustContainer::ADDVALUE);
        }

        d->adjDaysInput->setValue(dvalues.deltaDays);
        QTime deltaTime;
        deltaTime.setHMS(dvalues.deltaHours, dvalues.deltaMinutes, dvalues.deltaSeconds);
        d->adjTimeInput->setTime(deltaTime);

        emit signalSettingsChanged();
    }

    delete dlg;
}

void TimeAdjustSettings::slotSrcTimestampChanged()
{
    d->useFileDateTypeChooser->setEnabled(false);
    d->useMetaDateTypeChooser->setEnabled(false);
    d->useCustDateInput->setEnabled(false);
    d->useCustTimeInput->setEnabled(false);
    d->useCustomDateTodayBtn->setEnabled(false);

    if (d->useFileDateBtn->isChecked())
    {
        d->useFileDateTypeChooser->setEnabled(true);
    }
    else if (d->useMetaDateBtn->isChecked())
    {
        d->useMetaDateTypeChooser->setEnabled(true);
    }
    else if (d->useCustomDateBtn->isChecked())
    {
        d->useCustDateInput->setEnabled(true);
        d->useCustTimeInput->setEnabled(true);
        d->useCustomDateTodayBtn->setEnabled(true);
    }

    emit signalSettingsChanged();
}

void TimeAdjustSettings::slotResetDateToCurrent()
{
    QDateTime currentDateTime(QDateTime::currentDateTime());
    d->useCustDateInput->setDateTime(currentDateTime);
    d->useCustTimeInput->setDateTime(currentDateTime);

    emit signalSettingsChanged();
}

void TimeAdjustSettings::slotAdjustmentTypeChanged()
{
    // If the addition or subtraction has been selected, enable the edit boxes to enter the adjustment length
    bool isAdjustment = (d->adjTypeChooser->currentIndex() > TimeAdjustContainer::COPYVALUE);
    d->adjDaysInput->setEnabled(isAdjustment);
    d->adjDaysLabel->setEnabled(isAdjustment);
    d->adjTimeInput->setEnabled(isAdjustment);

    emit signalSettingsChanged();
}

void TimeAdjustSettings::slotDetAdjustmentByClockPhotoUrl(const QUrl& url)
{
    //usually called when a photo is dropped onto the push button
    detAdjustmentByClockPhotoUrl(url);
}

void TimeAdjustSettings::slotDetAdjustmentByClockPhotoDialog()
{
    // Determine the currently selected item and preselect it as clock photo
    QUrl emptyUrl;

   detAdjustmentByClockPhotoUrl(emptyUrl);
}


}  // namespace Digikam
