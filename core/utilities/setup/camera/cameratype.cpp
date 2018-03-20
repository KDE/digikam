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

#include "cameratype.h"

// Qt includes

#include <QAction>

// Local includes

#include "importui.h"

namespace Digikam
{

class CameraType::Private
{
public:

    Private() :
        valid(false),
        startingNumber(1),
        action(0)
    {
    }

    bool               valid;

    int                startingNumber;

    QString            title;
    QString            model;
    QString            port;
    QString            path;

    QAction*           action;

    QPointer<ImportUI> currentImportUI;
};

CameraType::CameraType()
    : d(new Private)
{
}

CameraType::CameraType(const QString& title, const QString& model,
                       const QString& port, const QString& path,
                       int startingNumber, QAction* const action)
    : d(new Private)
{
    setTitle(title);
    setModel(model);
    setPort(port);
    setPath(path);
    setStartingNumber(startingNumber);
    setAction(action);
    d->valid = true;
}

CameraType::~CameraType()
{
    delete d;
}

CameraType::CameraType(const CameraType& ctype)
    : d(new Private)
{
    d->title          = ctype.d->title;
    d->model          = ctype.d->model;
    d->port           = ctype.d->port;
    d->path           = ctype.d->path;
    d->startingNumber = ctype.d->startingNumber;
    d->action         = ctype.d->action;
    d->valid          = ctype.d->valid;
}

CameraType& CameraType::operator=(const CameraType& ctype)
{
    if (this != &ctype)
    {
        d->title          = ctype.d->title;
        d->model          = ctype.d->model;
        d->port           = ctype.d->port;
        d->path           = ctype.d->path;
        d->startingNumber = ctype.d->startingNumber;
        d->action         = ctype.d->action;
        d->valid          = ctype.d->valid;
    }

    return *this;
}

void CameraType::setTitle(const QString& title)
{
    d->title = title;
}

void CameraType::setModel(const QString& model)
{
    d->model = model;
}

void CameraType::setPort(const QString& port)
{
    d->port = port;
}

void CameraType::setPath(const QString& path)
{
    d->path = path;
}

void CameraType::setStartingNumber(int sn)
{
    d->startingNumber = sn;
}

void CameraType::setAction(QAction* const action)
{
    d->action = action;
}

void CameraType::setValid(bool valid)
{
    d->valid = valid;
}

void CameraType::setCurrentImportUI(ImportUI* const importui)
{
    d->currentImportUI = importui;
}

QString CameraType::title() const
{
    return d->title;
}

QString CameraType::model() const
{
    return d->model;
}

QString CameraType::port() const
{
    return d->port;
}

QString CameraType::path() const
{
    return d->path;
}

int CameraType::startingNumber() const
{
    return d->startingNumber;
}

QAction* CameraType::action() const
{
    return d->action;
}

bool CameraType::valid() const
{
    return d->valid;
}

ImportUI* CameraType::currentImportUI() const
{
    return d->currentImportUI;
}

}  // namespace Digikam
