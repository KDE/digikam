/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-03
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#include <qstring.h>
#include <qfile.h>
#include <qdom.h>
#include <qtextstream.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "gpiface.h"
#include "cameratype.h"
#include "cameralist.h"

CameraList* CameraList::instance_ = 0;

CameraList* CameraList::instance()
{
    return instance_;    
}

class CameraListPrivate
{
public:

    QPtrList<CameraType> clist;
    QString file;
    bool    modified;
};

CameraList::CameraList(QObject *parent, const QString& file)
    : QObject(parent)
{
    d = new CameraListPrivate;
    d->clist.setAutoDelete(true);
    d->file = file;
    d->modified = false;
    instance_ = this;
}

CameraList::~CameraList()
{
    close();
    
    d->clist.clear();
    delete d;

    instance_ = 0;
}

bool CameraList::load()
{
    d->modified = false;
    
    QFile cfile(d->file);

    if (!cfile.open(IO_ReadOnly))
        return false;

    QDomDocument doc("cameralist");
    if (!doc.setContent(&cfile))
        return false;

    QDomElement docElem = doc.documentElement();
    if (docElem.tagName()!="cameralist")
        return false;

    
    for (QDomNode n = docElem.firstChild();
         !n.isNull(); n = n.nextSibling()) {

        QDomElement e = n.toElement();
        if (e.isNull()) continue;
        if (e.tagName() != "item") continue;

        QString title  = e.attribute("title");
        QString model  = e.attribute("model");
        QString port   = e.attribute("port");
        QString path   = e.attribute("path");

        CameraType *ctype = new CameraType(title, model,
                                           port, path);
        insertPrivate(ctype);
    }

    return true;

}

bool CameraList::close()
{
    // If not modified don't save the file
    if (!d->modified)
        return true;
    
    QDomDocument doc("cameralist");
    doc.setContent(QString("<!DOCTYPE XMLCameraList><cameralist version=\"1.0\" client=\"digikam\"/>"));

    QDomElement docElem=doc.documentElement();
    
    for (CameraType *ctype = d->clist.first(); ctype;
         ctype = d->clist.next()) {

       QDomElement elem = doc.createElement("item");
       elem.setAttribute("title", ctype->title());
       elem.setAttribute("model", ctype->model());
       elem.setAttribute("port", ctype->port());
       elem.setAttribute("path", ctype->path());
       docElem.appendChild(elem);
    }

    QFile cfile(d->file);
    if (!cfile.open(IO_WriteOnly))
        return false;

    QTextStream stream(&cfile);
    stream.setEncoding(QTextStream::UnicodeUTF8);
    stream << doc.toString();
    cfile.close();

    return true;    
}

void CameraList::insert(CameraType* ctype)
{
    if (!ctype) return;

    d->modified = true;    
    insertPrivate(ctype);
}

void CameraList::remove(CameraType* ctype)
{
    if (!ctype) return;

    d->modified = true;
    removePrivate(ctype);
}

void CameraList::insertPrivate(CameraType* ctype)
{
    if (!ctype) return;
    emit signalCameraAdded(ctype);    
    d->clist.append(ctype);
}

void CameraList::removePrivate(CameraType* ctype)
{
    if (!ctype) return;
    emit signalCameraRemoved(ctype);
    d->clist.remove(ctype);
}

QPtrList<CameraType>* CameraList::cameraList()
{
    return &d->clist; 
}

CameraType* CameraList::find(const QString& title)
{
    for (CameraType *ctype = d->clist.first(); ctype;
         ctype = d->clist.next()) {
        if (ctype->title() == title)
            return ctype;
    }
    return 0;
}

CameraType* CameraList::autoDetect(bool& retry)
{
    retry = false;
    
    QString model, port;
    if (GPIface::autoDetect(model, port) != 0)
    {
       retry = ( KMessageBox::warningYesNo(0,
                                           i18n("Failed to auto-detect camera; "
                                                "please make sure it is connected "
                                                "properly and is turned on. "
                                                "Would you like to try again?"))
                 == KMessageBox::Yes );
        return 0;
    }

    // check if the camera is already in the list
    for (CameraType *ctype = d->clist.first(); ctype;
         ctype = d->clist.next()) {
        // we can get away with checking only the model, as the auto-detection
        // works only for usb cameras. so the port is always usb:
        if (ctype->model() == model)
            return ctype;
    }

    // looks like a new camera
    
    // NOTE: libgphoto2 now (2.1.4+) expects port names to be
    // something like "usb:001,012". but on linux these port numbers
    // will change everytime camera is reconnected. gphoto port funcs
    // also allow regexp match, so the safe bet is to just pass in
    // "usb:" and cross your fingers that user doesn't have multiple cameras
    // connected at the same time (whack them if they do).
    port = "usb:";
    CameraType* ctype = new CameraType(model, model, port, "/");
    insert(ctype);

    return ctype;
}

void CameraList::clear()
{
    
    CameraType *ctype = d->clist.first();
    while (ctype) {
        remove(ctype);
        ctype = d->clist.first();
    }
}


#include "cameralist.moc"
