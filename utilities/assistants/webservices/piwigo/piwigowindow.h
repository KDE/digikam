/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * Copyright (C) 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PIWIGO_WINDOW_H
#define PIWIGO_WINDOW_H

// Qt includes

#include <QList>

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"

namespace Digikam
{

class PiwigoAlbum;

class DIGIKAM_EXPORT PiwigoWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit PiwigoWindow(DInfoInterface* const iface,
                          QWidget* const parent);
    ~PiwigoWindow();

private:

    void connectSignals();
    void readSettings();
    QString cleanName(const QString&) const;

private Q_SLOTS:

    void slotDoLogin();
    void slotLoginFailed(const QString& msg);
    void slotBusy(bool val);
    void slotProgressInfo(const QString& msg);
    void slotError(const QString& msg);
    void slotAlbums(const QList<PiwigoAlbum>& albumList);
    void slotAlbumSelected();
    void slotAddPhoto();
    void slotAddPhotoNext();
    void slotAddPhotoSucceeded();
    void slotAddPhotoFailed(const QString& msg);
    void slotAddPhotoCancel();
    void slotEnableSpinBox(int n);
    void slotSettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PIWIGO_WINDOW_H
