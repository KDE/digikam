/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : camera setup tab.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPCAMERA_H
#define SETUPCAMERA_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "dbusydlg.h"

namespace Digikam
{

class SetupCamera : public QScrollArea
{
    Q_OBJECT

public:

    enum ConflictRule
    {
        OVERWRITE = 0,
        DIFFNAME,
        SKIPFILE
    };

public:

    explicit SetupCamera(QWidget* const parent = 0);
    ~SetupCamera();

    void applySettings();
    bool checkSettings();

    bool useFileMetadata();

Q_SIGNALS:

    void signalUseFileMetadataChanged(bool);

private Q_SLOTS:

    void slotSelectionChanged();

    void slotAddCamera();
    void slotRemoveCamera();
    void slotEditCamera();
    void slotAutoDetectCamera();

    void slotAddedCamera(const QString& title, const QString& model,
                         const QString& port,  const QString& path);
    void slotEditedCamera(const QString& title, const QString& model,
                          const QString& port,  const QString& path);

    void slotImportSelectionChanged();
    void slotAddFilter();
    void slotRemoveFilter();
    void slotEditFilter();
    void slotPreviewItemsClicked();
    void slotPreviewFullImageSizeClicked();

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class CameraAutoDetectThread : public DBusyThread
{
    Q_OBJECT

public:

    explicit CameraAutoDetectThread(QObject* const parent);
    virtual ~CameraAutoDetectThread();

    int     result() const;
    QString model()  const;
    QString port()   const;

private:

    void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SETUPCAMERA_H
