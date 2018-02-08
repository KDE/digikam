/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef IMAGESHACK_WINDOW_H
#define IMAGESHACK_WINDOW_H

// Qt includes

#include <QString>
#include <QWidget>

// Local includes

#include "wstooldialog.h"
#include "digikam_export.h"
#include "dimageslist.h"
#include "dinfointerface.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageShackWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit ImageShackWindow(DInfoInterface* const iface, QWidget* const parent);
    ~ImageShackWindow();

public:

    DImagesList* getImagesList() const;

Q_SIGNALS:

    void signalBusy(bool val);

private Q_SLOTS:

    void slotImageListChanged();
    void slotStartTransfer();
    void slotBusy(bool val);
    void slotFinished();
    void slotCancelClicked();

    void slotJobInProgress(int step, int, const QString& format);
    void slotLoginDone(int errCode, const QString& errMsg);
    void slotGetGalleriesDone(int errCode, const QString& errMsg);
    void slotGetGalleries();

    void slotAddPhotoDone(int errCode, const QString& errMsg);

    void authenticate();

private:

    void readSettings();
    void saveSettings();

    void authenticationDone(int errCode, const QString& errMsg);

    void uploadNextItem();

    void closeEvent(QCloseEvent* e) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotChangeRegistrantionCode();
    void slotNewAlbumRequest();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGESHACK_WINDOW_H
