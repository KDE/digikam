/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-05-12
 * @brief  A widget to apply Reverse Geocoding
 *
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RG_WIDGET_H
#define RG_WIDGET_H

// Qt includes

#include <QUrl>
#include <QWidget>

// Local includes

#include "geoiface_types.h"

// Local includes

#include "gpsimageitem.h"

class QItemSelectionModel;
class QAbstractItemModel;
class KConfigGroup;

namespace Digikam
{

class GPSImageModel;
class GPSUndoCommand;

class RGWidget : public QWidget
{
    Q_OBJECT

public:

    RGWidget(GPSImageModel* const imageModel, QItemSelectionModel* const selectionModel,
             QAbstractItemModel* externTagModel, QWidget* const parent = 0);
    ~RGWidget();

    void setUIEnabled(const bool state);
    void readSettingsFromGroup(const KConfigGroup* const group);
    void saveSettingsToGroup(KConfigGroup* const group);

private Q_SLOTS:

    void slotButtonRGSelected();
    void slotRGReady(QList<RGInfo>& returnedRGList);
    void slotHideOptions();
    void updateUIState();
    void slotAddCustomizedSpacer();
    void slotRemoveTag();
    void slotRemoveAllSpacers();
    void slotReaddNewTags();
    void slotRegenerateNewTags();
    void slotAddAllAddressElementsToTag();
    void slotAddSingleSpacer();
    void slotRGCanceled();

Q_SIGNALS:

    /**
     * This signal emits when containing widgets need to be enabled or disabled.
     * @param enabledState If true, the containing widgets will be enabled. Else, they will be disabled.
     */
    void signalSetUIEnabled(const bool enabledState);
    void signalSetUIEnabled(const bool enabledState, QObject* const cancelObject, const QString& cancelSlot);

    /**
     * Update the progress bar.
     */
    void signalProgressSetup(const int maxProgress, const QString& progressText);

    /**
     * Counts how many images were processed.
     * @param currentProgress The number of processed images.
     */
    void signalProgressChanged(const int currentProgress);

    /**
     * Sends the needed data to Undo/Redo Widget.
     * @param undoCommand Holds the data that will be used for undo or redo actions
     */
    void signalUndoCommand(GPSUndoCommand* undoCommand);

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event);

private:

    class Private;
    Private* const d;
};

}  /* namespace Digikam */

#endif /* RG_WIDGET_H */
