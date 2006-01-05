/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-12-21
 * Copyright 2004 by Renchi Raju
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
 * ============================================================ */

#ifndef DKCAMERA_H
#define DKCAMERA_H

// Qt includes.

#include <qstring.h>

// Local includes.

#include "gpiteminfo.h"

class QStringList;
class QImage;

namespace Digikam
{

class DKCamera
{
public:

    DKCamera(const QString& model,
           const QString& port,
           const QString& path);
    virtual ~DKCamera();

    virtual bool connect() = 0;
    virtual void cancel()  = 0;

    virtual void getAllFolders(const QString& folder,
                               QStringList& subFolderList) = 0;
    virtual bool getItemsInfoList(const QString& folder,
                                  GPItemInfoList& infoList) = 0;
    virtual bool getThumbnail(const QString& folder,
                              const QString& itemName,
                              QImage& thumbnail) = 0;
    virtual bool getExif(const QString& folder,
                         const QString& itemName,
                         char **edata, int& esize) = 0;

    virtual bool downloadItem(const QString& folder,
                              const QString& itemName,
                              const QString& saveFile) = 0;
    virtual bool deleteItem(const QString& folder,
                            const QString& itemName) = 0;
    virtual bool uploadItem(const QString& folder,
                            const QString& itemName,
                            const QString& localFile) = 0;

    QString model() const;
    QString port()  const;
    QString path()  const;

private:

    QString m_model;
    QString m_port;
    QString m_path;
};

}  // namespace Digikam

#endif /* DKCAMERA_H */
