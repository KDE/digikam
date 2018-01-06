/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : Cameras list container
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

#include "cameralist.h"

// Qt includes

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QString>
#include <QTextCodec>
#include <QTextStream>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "cameratype.h"
#include "gpcamera.h"
#include "digikam_debug.h"

namespace Digikam
{

CameraList* CameraList::m_defaultList = 0;

CameraList* CameraList::defaultList()
{
    return m_defaultList;
}

class CameraList::Private
{
public:

    Private() :
        modified(false)
    {
    }

    bool               modified;

    QList<CameraType*> clist;
    QString            file;
};

CameraList::CameraList(QObject* const parent, const QString& file)
    : QObject(parent),
      d(new Private)
{
    d->file = file;
    qCDebug(DIGIKAM_GENERAL_LOG) << "Camera XML data: " << d->file;

    if (!m_defaultList)
    {
        m_defaultList = this;
    }
}

CameraList::~CameraList()
{
    save();
    clear();
    delete d;

    if (m_defaultList == this)
    {
        m_defaultList = 0;
    }
}

bool CameraList::load()
{
    d->modified = false;

    QFile cfile(d->file);

    if (!cfile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDomDocument doc(QLatin1String("cameralist"));

    if (!doc.setContent(&cfile))
    {
        return false;
    }

    QDomElement docElem = doc.documentElement();

    if (docElem.tagName() != QLatin1String("cameralist"))
    {
        return false;
    }

    for (QDomNode n = docElem.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement e = n.toElement();

        if (e.isNull())
        {
            continue;
        }

        if (e.tagName() != QLatin1String("item"))
        {
            continue;
        }

        QString title           = e.attribute(QLatin1String("title"));
        QString model           = e.attribute(QLatin1String("model"));
        QString port            = e.attribute(QLatin1String("port"));
        QString path            = e.attribute(QLatin1String("path"));
        int sn                  = e.attribute(QLatin1String("startingnumber")).toInt();
        CameraType* const ctype = new CameraType(title, model, port, path, sn);
        insertPrivate(ctype);
    }

    return true;
}

bool CameraList::save()
{
    // If not modified don't save the file
    if (!d->modified)
    {
        return true;
    }

    QDomDocument doc(QLatin1String("cameralist"));
    doc.setContent(QLatin1String("<!DOCTYPE XMLCameraList><cameralist version=\"1.2\" client=\"digikam\"/>"));

    QDomElement docElem = doc.documentElement();

    foreach(CameraType* const ctype, d->clist)
    {
        QDomElement elem = doc.createElement(QLatin1String("item"));
        elem.setAttribute(QLatin1String("title"),          ctype->title());
        elem.setAttribute(QLatin1String("model"),          ctype->model());
        elem.setAttribute(QLatin1String("port"),           ctype->port());
        elem.setAttribute(QLatin1String("path"),           ctype->path());
        elem.setAttribute(QLatin1String("startingnumber"), QString::number(ctype->startingNumber()));
        docElem.appendChild(elem);
    }

    QFile cfile(d->file);

    if (!cfile.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open Camera XML file to save data (" << d->file << ")";
        return false;
    }

    QTextStream stream(&cfile);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream.setAutoDetectUnicode(true);
    stream << doc.toString();
    cfile.close();

    d->modified = false;
    return true;
}

void CameraList::insert(CameraType* const ctype)
{
    if (!ctype)
    {
        return;
    }

    d->modified = true;
    insertPrivate(ctype);
}

void CameraList::remove(CameraType* const ctype)
{
    if (!ctype)
    {
        return;
    }

    d->modified = true;
    removePrivate(ctype);
}

void CameraList::insertPrivate(CameraType* const ctype)
{
    if (!ctype)
    {
        return;
    }

    d->clist.append(ctype);
    emit signalCameraAdded(ctype);
}

void CameraList::removePrivate(CameraType* const ctype)
{
    if (!ctype)
    {
        return;
    }

    emit signalCameraRemoved(ctype->action());

    int i = d->clist.indexOf(ctype);

    if (i != -1)
    {
        delete d->clist.takeAt(i);
    }
}

void CameraList::clear()
{
    while (!d->clist.isEmpty())
    {
        d->modified = true;
        removePrivate(d->clist.first());
    }
}

QList<CameraType*>* CameraList::cameraList() const
{
    return &d->clist;
}

CameraType* CameraList::find(const QString& title) const
{
    foreach(CameraType* const ctype, d->clist)
    {
        if (ctype->title() == title)
        {
            return ctype;
        }
    }

    return 0;
}

CameraType* CameraList::autoDetect(bool& retry)
{
    retry = false;

    QString model, port;

    if (GPCamera::autoDetect(model, port) != 0)
    {
        retry = (QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(),
                                      i18n("Failed to auto-detect camera; "
                                           "please make sure it is connected "
                                           "properly and is turned on. "
                                           "Would you like to try again?"),
                                      QMessageBox::Yes | QMessageBox::No)
                 == QMessageBox::Yes);
        return 0;
    }

    // Check if the camera is already in the list
    foreach(CameraType* const ctype, d->clist)
    {
        // We can get away with checking only the model, as the auto-detection
        // works only for usb cameras. so the port is always usb:
        if (ctype->model() == model)
        {
            return ctype;
        }
    }

    // Looks like a new camera

    // NOTE: libgphoto2 now (2.1.4+) expects port names to be
    // something like "usb:001,012". but on linux these port numbers
    // will change every time camera is reconnected. gphoto port funcs
    // also allow regexp match, so the safe bet is to just pass in
    // "usb:" and cross your fingers that user doesn't have multiple cameras
    // connected at the same time (whack them if they do).

    if (port.startsWith(QLatin1String("usb:")))
    {
        port = QLatin1String("usb:");
    }

    CameraType* ctype = new CameraType(model, model, port, QLatin1String("/"), 1);
    insert(ctype);

    return ctype;
}

bool CameraList::findConnectedCamera(int vendorId, int productId, QString& model, QString& port)
{
    return GPCamera::findConnectedUsbCamera(vendorId, productId, model, port);
}

bool CameraList::changeCameraStartIndex(const QString& cameraTitle, int startIndex)
{
    CameraType* const cam = find(cameraTitle);

    if (cam)
    {
        cam->setStartingNumber(startIndex);
        d->modified = true;
        save();
        return true;
    }

    return false;
}

}  // namespace Digikam
