/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-09-02
 * Description : Start Web Service methods.
 *
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#ifndef DIGIKAM_WS_STARTER_H
#define DIGIKAM_WS_STARTER_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DInfoInterface;

class DIGIKAM_EXPORT WSStarter : public QObject
{
    Q_OBJECT

public:

    enum WSTool
    {
        ExportUnknown = 0,
        ExportBox,
        ExportDropbox,
        ExportFacebook,
        ExportFileTransfer,
        ImportFileTransfer,
        ExportFlickr,
        ExportGdrive,
        ExportGphoto,
        ImportGphoto,
        ExportImageshack,
        ExportImgur,
        ExportIpfs,
        ExportMediawiki,
        ExportOnedrive,
        ExportPinterest,
        ExportPiwigo,
        ExportRajce,
        ExportSmugmug,
        ImportSmugmug,
        ExportVkontakte,
        ExportYandexfotki
    };

public:

    static WSStarter* instance();

    static void cleanUp();

    static void exportToWebService(int tool, DInfoInterface* const iface, QWidget* const parent);
    static void importFromWebService(int tool, DInfoInterface* const iface, QWidget* const parent);

private:

    explicit WSStarter();
    ~WSStarter();

    void toWebService(int tool, DInfoInterface* const iface, QWidget* const parent);
    void fromWebService(int tool, DInfoInterface* const iface, QWidget* const parent);

    bool checkWebService(QWidget* const widget) const;

private:

    class Private;
    Private* const d;

    friend class WSStarterCreator;
};

} // namespace Digikam

#endif // DIGIKAM_WS_STARTER_H
