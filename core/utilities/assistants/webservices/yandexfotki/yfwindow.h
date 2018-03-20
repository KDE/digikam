/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-15
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2010      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Luka Renko <lure at kubuntu dot org>
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

#ifndef YF_WINDOW_H
#define YF_WINDOW_H

// Qt includes

#include <QWidget>

// Local includes

#include "wstooldialog.h"
#include "digikam_export.h"
#include "yfalbum.h"
#include "yfphoto.h"

class QCloseEvent;

namespace Digikam
{

class DInfoInterface;

class DIGIKAM_EXPORT YFWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit YFWindow(DInfoInterface* const iface, QWidget* const parent, bool import = false);
    ~YFWindow();

public:

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

    QString getDestinationPath() const;

private Q_SLOTS:

    // ui slots
    void slotChangeUserClicked();

    void slotError();
    void slotGetSessionDone();
    void slotGetTokenDone();
    void slotGetServiceDone();

    void slotListAlbumsDone(const QList<YandexFotkiAlbum>& albumsList);
    void slotListPhotosDone(const QList <YFPhoto>& photosList);
    void slotListPhotosDoneForUpload(const QList <YFPhoto>& photosList);
    void slotListPhotosDoneForDownload(const QList <YFPhoto>& photosList);
    void slotUpdatePhotoDone(YFPhoto& );
    void slotUpdateAlbumDone();

    void slotNewAlbumRequest();
    void slotReloadAlbumsRequest();

    void slotStartTransfer();
    void slotCancelClicked();
    void slotFinished();

private:

    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

    void readSettings();
    void writeSettings();

    void reset();
    void updateControls(bool val);
    void updateLabels();

    void authenticate(bool forceAuthWindow);
    void updateNextPhoto();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // YF_WINDOW_H
