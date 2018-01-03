/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-29
 * Description : Camera settings container.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAMERATYPE_H
#define CAMERATYPE_H

// Qt includes

#include <QString>
#include <QPointer>

class QAction;

namespace Digikam
{

class ImportUI;

class CameraType
{
public:

    CameraType();
    CameraType(const QString& title, const QString& model,
               const QString& port, const QString& path,
               int startingNumber, QAction* const action = 0);
    ~CameraType();

    CameraType(const CameraType& ctype);
    CameraType& operator=(const CameraType& type);

    void setTitle(const QString& title);
    void setModel(const QString& model);
    void setPort(const QString& port);
    void setPath(const QString& path);
    void setStartingNumber(int sn);
    void setAction(QAction* const action);
    void setValid(bool valid);
    void setCurrentImportUI(ImportUI* const importui);

    QString   title()           const;
    QString   model()           const;
    QString   port()            const;
    QString   path()            const;
    int       startingNumber()  const;
    QAction*  action()          const;
    bool      valid()           const;
    ImportUI* currentImportUI() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* CAMERATYPE_H */
