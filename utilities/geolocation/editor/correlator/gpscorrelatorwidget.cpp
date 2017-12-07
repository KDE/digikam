/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-26
 * Description : A widget to configure the GPS correlation
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014      by Justus Schwartz <justus at gmx dot li>
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

#include "gpscorrelatorwidget.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QPointer>
#include <QRadioButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>
#include <QComboBox>
#include <QMenu>
#include <QStandardPaths>
#include <QSpinBox>
#include <QMessageBox>
#include <QDateEdit>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dmessagebox.h"
#include "gpsimagemodel.h"
#include "gpsimageitem.h"
#include "gpsundocommand.h"
#include "track_listmodel.h"
#include "dexpanderbox.h"
#include "dfiledialog.h"

namespace Digikam
{

class GPSCorrelatorWidget::Private
{
public:

    Private()
      : gpxLoadFilesButton(0),
        gpxFileList(0),
        offsetSign(0),
        offsetTime(0),
        interpolateButton(0),
        interpolateLimitLabel(0),
        interpolateLimitInput(0),
        directMatchButton(0),
        directMatchLimitLabel(0),
        directMatchLimitInput(0),
        showTracksOnMap(0),
        correlateButton(0),
        trackManager(0),
        trackCorrelator(0),
        trackListModel(0),
        uiEnabledInternal(true),
        uiEnabledExternal(true),
        imageModel(0),
        correlationTotalCount(0),
        correlationCorrelatedCount(0),
        correlationTriedCount(0),
        correlationUndoCommand(0)
    {
    }

    QString                 gpxFileOpenLastDirectory;
    QPushButton*            gpxLoadFilesButton;
    QTreeView*              gpxFileList;

    QComboBox*              offsetSign;
    QTimeEdit*              offsetTime;

    QRadioButton*           interpolateButton;
    QLabel*                 interpolateLimitLabel;
    QTimeEdit*              interpolateLimitInput;
    QRadioButton*           directMatchButton;
    QLabel*                 directMatchLimitLabel;
    QTimeEdit*              directMatchLimitInput;
    QCheckBox*              showTracksOnMap;

    QPushButton*            correlateButton;

    TrackManager*           trackManager;
    TrackCorrelator*        trackCorrelator;
    TrackListModel*         trackListModel;
    bool                    uiEnabledInternal;
    bool                    uiEnabledExternal;
    GPSImageModel*          imageModel;

    int                     correlationTotalCount;
    int                     correlationCorrelatedCount;
    int                     correlationTriedCount;
    GPSUndoCommand*         correlationUndoCommand;
};

GPSCorrelatorWidget::GPSCorrelatorWidget(QWidget* const parent,
                                         GPSImageModel* const imageModel,
                                         TrackManager* const trackManager)
    : QWidget(parent),
      d(new Private())
{
    d->imageModel      = imageModel;
    d->trackManager    = trackManager;
    d->trackCorrelator = new TrackCorrelator(d->trackManager, this);
    d->trackListModel  = new TrackListModel(d->trackManager, this);

    connect(d->trackManager, SIGNAL(signalAllTrackFilesReady()),
            this, SLOT(slotAllTrackFilesReady()));

    connect(d->trackCorrelator, SIGNAL(signalItemsCorrelated(Digikam::TrackCorrelator::Correlation::List)),
            this, SLOT(slotItemsCorrelated(Digikam::TrackCorrelator::Correlation::List)));

    connect(d->trackCorrelator, SIGNAL(signalAllItemsCorrelated()),
            this, SLOT(slotAllItemsCorrelated()));

    connect(d->trackCorrelator, SIGNAL(signalCorrelationCanceled()),
            this, SLOT(slotCorrelationCanceled()));

    QGridLayout* const settingsLayout = new QGridLayout(this);

    d->gpxLoadFilesButton             = new QPushButton(i18n("Load GPX files..."), this);

    d->gpxFileList          = new QTreeView(this);
    d->gpxFileList->setModel(d->trackListModel);
    d->gpxFileList->setHeaderHidden(false);
    d->gpxFileList->setRootIsDecorated(false);

    d->showTracksOnMap      = new QCheckBox(i18n("Show tracks on Map"), this);
    d->showTracksOnMap->setWhatsThis(i18n("Set this option to show tracks on the Map"));
    DLineWidget* const line = new DLineWidget(Qt::Horizontal, this);

    connect(d->showTracksOnMap, SIGNAL(stateChanged(int)),
            this, SLOT(slotShowTracksStateChanged(int)));

    QWidget* const offsetWidget = new QWidget(this);
    QGridLayout* const offsetLayout = new QGridLayout(offsetWidget);

    QLabel* offsetLabel         = new QLabel(i18n("Offset of pictures (hh:mm:ss):"),
                                             offsetWidget);
    offsetLabel->setWhatsThis(i18n("Sets the offset between picture times "
                                        "and track times. E.g. to correct "
                                        "wrong camera clock or different time "
                                        "zone."));

    d->offsetSign = new QComboBox(offsetWidget);
    d->offsetSign->addItem(QLatin1String("+"));
    d->offsetSign->addItem(QLatin1String("-"));
    d->offsetSign->setWhatsThis(i18n("Set whether picture time is ahead or "
                                     "behind the track time."));

    d->offsetTime = new QTimeEdit(offsetWidget);
    d->offsetTime->setDisplayFormat(QLatin1String("HH:mm:ss"));

    offsetLayout->addWidget(offsetLabel,   0, 0, 1, 1);
    offsetLayout->addWidget(d->offsetSign, 0, 1, 1, 1);
    offsetLayout->addWidget(d->offsetTime, 0, 2, 1, 1);
    offsetLayout->setColumnStretch(0, 10);
    offsetLayout->setContentsMargins(this->contentsMargins());

    // track to picture matching options
    QWidget* const matchWidget     = new QWidget(this);
    QGridLayout* const matchLayout = new QGridLayout(matchWidget);

    d->interpolateButton     = new QRadioButton(QLatin1String("Interpolate"), matchWidget);
    d->interpolateLimitLabel = new QLabel(i18n("Max. time gap (hh:mm:ss):"), matchWidget);
    d->interpolateLimitInput = new QTimeEdit(matchWidget);
    d->interpolateLimitInput->setDisplayFormat(QLatin1String("HH:mm:ss"));
    d->interpolateLimitInput->setTime(QTime::fromString(QLatin1String("00:15:00")));
    d->interpolateLimitInput->setWhatsThis(i18n("Sets the maximum time difference "
                                                "to interpolate GPX file points "
                                                "to image time data. "
                                                "If the time difference exceeds "
                                                "this setting, no match will be attempted."));

    QRadioButton* directMatchButton = new QRadioButton(QLatin1String("Match directly"), matchWidget);
    d->directMatchLimitLabel        = new QLabel(i18n("Max. time gap (hh:mm:ss):"), matchWidget);
    d->directMatchLimitInput        = new QTimeEdit(matchWidget);
    d->directMatchLimitInput->setDisplayFormat(QLatin1String("HH:mm:ss"));
    d->directMatchLimitInput->setTime(QTime::fromString(QLatin1String("00:00:30")));
    d->directMatchLimitInput->setWhatsThis(i18n("Sets the maximum time difference "
                                                "from a GPS track point to the "
                                                "image time to be matched. "
                                                "If the time difference exceeds "
                                                "this setting, no match will be attempted."));

    matchLayout->addWidget(d->interpolateButton,       0, 0, 1, 3);
    matchLayout->addWidget(d->interpolateLimitLabel,   1, 1, 1, 1);
    matchLayout->addWidget(d->interpolateLimitInput,   1, 2, 1, 1);
    matchLayout->addWidget(directMatchButton,          2, 0, 1, 3);
    matchLayout->addWidget(d->directMatchLimitLabel,   3, 1, 1, 1);
    matchLayout->addWidget(d->directMatchLimitInput,   3, 2, 1, 1);
    matchLayout->setColumnStretch(1, 10);
    matchLayout->setColumnMinimumWidth(0, 40);
    matchLayout->setContentsMargins(this->contentsMargins());

    connect(d->interpolateButton, SIGNAL(toggled(bool)),
            this, SLOT(updateUIState()));

    d->correlateButton = new QPushButton(i18n("Correlate"), this);

    // layout form
    int row = 0;
    settingsLayout->addWidget(d->gpxLoadFilesButton, row, 0, 1, 1);
    row++;
    settingsLayout->addWidget(d->gpxFileList,        row, 0, 1, 1);
    row++;
    settingsLayout->addWidget(d->showTracksOnMap,    row, 0, 1, 1);
    row++;
    settingsLayout->addWidget(line,                  row, 0, 1, 1);
    row++;
    settingsLayout->addWidget(offsetWidget,          row, 0, 1, 1);
    row++;
    settingsLayout->addWidget(matchWidget,           row, 0, 4, 1);
    row += 4;
    settingsLayout->addWidget(d->correlateButton,    row, 0, 1, 1);

    settingsLayout->setRowStretch(row, 100);

    connect(d->gpxLoadFilesButton, SIGNAL(clicked()),
            this, SLOT(slotLoadTrackFiles()));

    connect(d->correlateButton, SIGNAL(clicked()),
            this, SLOT(slotCorrelate()));

    updateUIState();
}

GPSCorrelatorWidget::~GPSCorrelatorWidget()
{
}

void GPSCorrelatorWidget::slotLoadTrackFiles()
{
    const QStringList gpxFiles = DFileDialog::getOpenFileNames(this,
                                                               i18nc("@title:window", "Select GPX File to Load"),
                                                               d->gpxFileOpenLastDirectory,
                                                               i18n("GPS Exchange Format (*.gpx)"));

    if (gpxFiles.isEmpty())
        return;

    d->gpxFileOpenLastDirectory = QFileInfo(gpxFiles.first()).path();

    setUIEnabledInternal(false);

    QList<QUrl> list;

    foreach(const QString& str, gpxFiles)
    {
        list << QUrl::fromLocalFile(str);
    }

    d->trackManager->loadTrackFiles(list);
}

void GPSCorrelatorWidget::slotAllTrackFilesReady()
{
    // are there any invalid files?
    QStringList invalidFiles;
    const QList<QPair<QUrl, QString> > loadErrorFiles = d->trackManager->readLoadErrors();

    for (int i = 0; i < loadErrorFiles.count(); ++i)
    {
        const QPair<QUrl, QString> currentError = loadErrorFiles.at(i);
        const QString fileErrorString = QString::fromLatin1("%1: %2")
            .arg(currentError.first.toLocalFile())
            .arg(currentError.second);

        invalidFiles << fileErrorString;
    }

    if (!invalidFiles.isEmpty())
    {
        const QString errorString = i18np(
                "The following GPX file could not be loaded:",
                "The following %1 GPX files could not be loaded:",
                invalidFiles.count()
            );

        const QString errorTitleString = i18np(
                "Error loading GPX file",
                "Error loading GPX files",
                invalidFiles.count()
            );

        DMessageBox::showInformationList(QMessageBox::Critical,
                                         this,
                                         errorTitleString,
                                         errorString,
                                         invalidFiles);
    }

    emit(signalAllTrackFilesReady());

    setUIEnabledInternal(true);
}

void GPSCorrelatorWidget::setUIEnabledInternal(const bool state)
{
    d->uiEnabledInternal = state;
    updateUIState();
}

void GPSCorrelatorWidget::setUIEnabledExternal(const bool state)
{
    d->uiEnabledExternal = state;
    updateUIState();
}

void GPSCorrelatorWidget::updateUIState()
{
    const bool state = d->uiEnabledInternal && d->uiEnabledExternal;

    d->gpxLoadFilesButton->setEnabled(state);
    d->offsetSign->setEnabled(state);
    d->directMatchLimitInput->setEnabled(state && !d->interpolateButton->isChecked());
    d->interpolateLimitInput->setEnabled(state && d->interpolateButton->isChecked());

    const bool haveValidGpxFiles = d->trackManager->trackCount()>0;
    d->correlateButton->setEnabled(state && haveValidGpxFiles);
}

void GPSCorrelatorWidget::slotCorrelate()
{
    // disable the UI of the entire dialog:
    emit(signalSetUIEnabled(false, this, QLatin1String(SLOT(slotCancelCorrelation()))));

    // store the options:
    TrackCorrelator::CorrelationOptions options;
    options.maxGapTime = d->directMatchLimitInput->time().msecsSinceStartOfDay() / 1000;

    int userOffset = d->offsetTime->time().msecsSinceStartOfDay() / 1000;
    if (d->offsetSign->currentText() == QLatin1String("-"))
    {
        userOffset = (-1) * userOffset;
    }
    options.secondsOffset+=userOffset;

    options.interpolate          = d->interpolateButton->isChecked();
    options.interpolationDstTime = d->interpolateLimitInput->time().msecsSinceStartOfDay() / 1000;

    // create a list of items to be correlated
    TrackCorrelator::Correlation::List itemList;

    const int imageCount = d->imageModel->rowCount();

    for (int i = 0 ; i < imageCount ; ++i)
    {
        QPersistentModelIndex imageIndex = d->imageModel->index(i, 0);
        GPSImageItem* const imageItem    = d->imageModel->itemFromIndex(imageIndex);

        if (!imageItem)
            continue;

        TrackCorrelator::Correlation correlationItem;
        correlationItem.userData = QVariant::fromValue(imageIndex);
        correlationItem.dateTime = imageItem->dateTime();

        itemList << correlationItem;
    }

    d->correlationTotalCount      = imageCount;
    d->correlationCorrelatedCount = 0;
    d->correlationTriedCount      = 0;
    d->correlationUndoCommand     = new GPSUndoCommand;

    emit(signalProgressSetup(imageCount, i18n("Correlating images -")));

    d->trackCorrelator->correlate(itemList, options);

    // results will be sent to slotItemsCorrelated and slotAllItemsCorrelated
}

void GPSCorrelatorWidget::slotItemsCorrelated(const Digikam::TrackCorrelator::Correlation::List& correlatedItems)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << correlatedItems.count();
    d->correlationTriedCount += correlatedItems.count();

    for (int i = 0 ; i < correlatedItems.count() ; ++i)
    {
        const TrackCorrelator::Correlation& itemCorrelation = correlatedItems.at(i);
        const QPersistentModelIndex itemIndex               = itemCorrelation.userData.value<QPersistentModelIndex>();

        if (!itemIndex.isValid())
            continue;

        GPSImageItem* const imageItem                       = d->imageModel->itemFromIndex(itemIndex);

        if (!imageItem)
            continue;

        if (itemCorrelation.flags&TrackCorrelator::CorrelationFlagCoordinates)
        {
            d->correlationCorrelatedCount++;

            GPSDataContainer newData;
            newData.setCoordinates(itemCorrelation.coordinates);

            if (itemCorrelation.nSatellites >= 0)
                newData.setNSatellites(itemCorrelation.nSatellites);

            // if hDop is available, use it
            if (itemCorrelation.hDop >= 0)
                newData.setDop(itemCorrelation.hDop);

            // but if pDop is available, prefer pDop over hDop
            if (itemCorrelation.pDop >= 0)
                newData.setDop(itemCorrelation.pDop);

            if (itemCorrelation.fixType >= 0)
            {
                newData.setFixType(itemCorrelation.fixType);
            }
            if (itemCorrelation.speed >= 0)
            {
                newData.setSpeed(itemCorrelation.speed);
            }

            GPSUndoCommand::UndoInfo undoInfo(itemIndex);
            undoInfo.readOldDataFromItem(imageItem);

            imageItem->setGPSData(newData);
            undoInfo.readNewDataFromItem(imageItem);

            d->correlationUndoCommand->addUndoInfo(undoInfo);
        }
    }

    emit(signalProgressChanged(d->correlationTriedCount));
}

void GPSCorrelatorWidget::slotAllItemsCorrelated()
{
    if (d->correlationCorrelatedCount == 0)
    {
        QMessageBox::warning(this, i18n("Correlation failed"),
                             i18n("Could not correlate any image - please make sure the offset and gap settings are correct."));
    }
    else if (d->correlationCorrelatedCount == d->correlationTotalCount)
    {
        QMessageBox::information(this, i18n("Correlation succeeded"),
                                 i18n("All images have been correlated. You can now check their position on the map."));
    }
    else
    {
        // note: no need for i18np here, because the case of correlationTotalCount==1 is covered in the other two cases.
        QMessageBox::warning(this, i18n("Correlation finished"),
                           i18n("%1 out of %2 images have been correlated. Please check the offset and gap settings if you think that more images should have been correlated.",
                                d->correlationCorrelatedCount, d->correlationTotalCount));
    }

    if (d->correlationCorrelatedCount == 0)
    {
        delete d->correlationUndoCommand;
    }
    else
    {
        d->correlationUndoCommand->setText(i18np("1 image correlated",
                                                 "%1 images correlated",
                                                 d->correlationCorrelatedCount));
        emit(signalUndoCommand(d->correlationUndoCommand));
    }

    // enable the UI:
    emit(signalSetUIEnabled(true));
}

void GPSCorrelatorWidget::saveSettingsToGroup(KConfigGroup* const group)
{
    group->writeEntry("ShowTracksOnMap",              d->showTracksOnMap->isChecked());
    group->writeEntry("Interpolate",                  d->interpolateButton->isChecked());
    group->writeEntry("Max Inter Dist Time",          d->interpolateLimitInput->time().toString());
    group->writeEntry("Max Gap Time",                 d->directMatchLimitInput->time().toString());
    group->writeEntry("Offset Sign",                  d->offsetSign->currentIndex());
    group->writeEntry("Offset Time",                   d->offsetTime->time().toString());
    group->writeEntry("GPX File Open Last Directory", d->gpxFileOpenLastDirectory);
}

void GPSCorrelatorWidget::readSettingsFromGroup(const KConfigGroup* const group)
{
    d->showTracksOnMap->setChecked(group->readEntry("ShowTracksOnMap", true));
    d->interpolateButton->setChecked(group->readEntry("Interpolate", true));
    d->interpolateLimitInput->setTime(QTime::fromString(group->readEntry("Max Inter Dist Time", "00:15:00")));
    d->directMatchLimitInput->setTime(QTime::fromString(group->readEntry("Max Gap Time", "00:00:30")));
    d->offsetSign->setCurrentIndex(group->readEntry("Offset Sign", 0));
    d->offsetTime->setTime(QTime::fromString(group->readEntry("Offset Time", "00:00:00")));
    d->gpxFileOpenLastDirectory = group->readEntry("GPX File Open Last Directory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

    updateUIState();
}

void GPSCorrelatorWidget::slotCancelCorrelation()
{
    d->trackCorrelator->cancelCorrelation();
}

void GPSCorrelatorWidget::slotCorrelationCanceled()
{
    d->correlationUndoCommand->undo();

    delete d->correlationUndoCommand;

    emit(signalSetUIEnabled(true));
}

QList<GeoCoordinates::List> GPSCorrelatorWidget::getTrackCoordinates() const
{
    QList<GeoCoordinates::List> trackList;

    for (int i = 0 ; i < d->trackManager->trackCount() ; ++i)
    {
        const TrackManager::Track& gpxData = d->trackManager->getTrack(i);
        GeoCoordinates::List track;

        for (int coordIdx = 0 ; coordIdx < gpxData.points.count() ; ++coordIdx)
        {
            TrackManager::TrackPoint const& point = gpxData.points.at(coordIdx);
            track << point.coordinates;
        }

        trackList << track;
    }

    return trackList;
}

void GPSCorrelatorWidget::slotShowTracksStateChanged(int state)
{
    const bool doShowTracks = (state == Qt::Checked);
    d->trackManager->setVisibility(doShowTracks);
}

bool GPSCorrelatorWidget::getShowTracksOnMap() const
{
    return d->showTracksOnMap->isChecked();
}

} // namespace Digikam
