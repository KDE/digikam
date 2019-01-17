/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-26
 * Description : A widget to configure the GPS correlation
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_GPS_CORRELATOR_WIDGET_H
#define DIGIKAM_GPS_CORRELATOR_WIDGET_H

// Qt includes

#include <QUrl>
#include <QWidget>

// local includes

#include "track_correlator.h"
#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class GPSItemModel;
class GPSUndoCommand;

class DIGIKAM_EXPORT GPSCorrelatorWidget : public QWidget
{
    Q_OBJECT

public:

    explicit GPSCorrelatorWidget(QWidget* const parent, GPSItemModel* const imageModel, TrackManager* const trackManager);
    virtual ~GPSCorrelatorWidget();

    void setUIEnabledExternal(const bool state);
    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);
    QList<GeoCoordinates::List> getTrackCoordinates() const;
    bool getShowTracksOnMap() const;

protected:

    void setUIEnabledInternal(const bool state);

Q_SIGNALS:

    void signalSetUIEnabled(const bool enabledState);
    void signalSetUIEnabled(const bool enabledState, QObject* const cancelObject, const QString& cancelSlot);
    void signalProgressSetup(const int maxProgress, const QString& progressText);
    void signalProgressChanged(const int currentProgress);
    void signalUndoCommand(GPSUndoCommand* undoCommand);
    void signalAllTrackFilesReady();

public Q_SLOTS:

    void slotCancelCorrelation();

private Q_SLOTS:

    void updateUIState();
    void slotLoadTrackFiles();
    void slotAllTrackFilesReady();
    void slotCorrelate();
    void slotItemsCorrelated(const Digikam::TrackCorrelator::Correlation::List& correlatedItems);
    void slotAllItemsCorrelated();
    void slotCorrelationCanceled();
    void slotShowTracksStateChanged(int state);

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

#endif // DIGIKAM_GPS_CORRELATOR_WIDGET_H
