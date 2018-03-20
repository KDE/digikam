/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-28
 * Description : a tool to export image to a KIO accessible
 *               location
 *
 * Copyright (C) 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FT_EXPORT_WINDOW_H
#define FT_EXPORT_WINDOW_H

// Qt includes

#include <QUrl>

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"
#include "digikam_export.h"

class KJob;

namespace KIO
{
    class Job;
}

namespace Digikam
{

class FTExportWidget;

/**
 * Main window of the KIOExport tool.
 */

class DIGIKAM_EXPORT FTExportWindow: public WSToolDialog
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent QWidget
     */
    explicit FTExportWindow(DInfoInterface* const iface, QWidget* const parent);

    /**
     * Destructor.
     */
    ~FTExportWindow();

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

private Q_SLOTS:

    /**
     * Processes changes on the image list.
     */
    void slotImageListChanged();

    /**
     * Starts uploading the selected images.
     */
    void slotUpload();

    /**
     * Processes changes in the target url.
     */
    void slotTargetUrlChanged(const QUrl& target);

    /**
     * Removes the copied image from the image list.
     */
    void slotCopyingDone(KIO::Job* job, const QUrl& from, const QUrl& to,
                         const QDateTime& mtime, bool directory, bool renamed);

    /**
     * Re-enables the dialog after the job finished and displays a warning if
     * something didn't work.
     */
    void slotCopyingFinished(KJob* job);

    void slotFinished();

protected:

    /**
     * Handle Close event from dialog title bar.
     */
    void closeEvent(QCloseEvent* e) Q_DECL_OVERRIDE;

    /**
     * Refresh status (enabled / disabled) of the upload button according to
     * contents of the image list and the specified target.
     */
    void updateUploadButton();

    /**
     * Restores settings.
     */
    void restoreSettings();

    /**
     * Saves settings.
     */
    void saveSettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FT_EXPORT_WINDOW_H
