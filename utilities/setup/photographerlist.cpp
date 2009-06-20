/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : Photographers list container.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "photographerlist.h"
#include "photographerlist.moc"

// Qt includes

#include <QString>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>
#include <QTextCodec>

// KDE includes

#include <klocale.h>

// Local includes

#include "photographer.h"

namespace Digikam
{

PhotographerList* PhotographerList::m_defaultList = 0;

PhotographerList* PhotographerList::defaultList()
{
    return m_defaultList;
}

class PhotographerListPrivate
{
public:

    PhotographerListPrivate()
    {
        modified = false;
    }

    bool                 modified;

    QList<Photographer*> pList;
    QString              file;
};

PhotographerList::PhotographerList(QObject *parent, const QString& file)
                : QObject(parent), d(new PhotographerListPrivate)
{
    d->file = file;
    if (!m_defaultList)
        m_defaultList = this;
}

PhotographerList::~PhotographerList()
{
    save();
    clear();
    delete d;

    if (m_defaultList == this)
        m_defaultList = 0;
}

bool PhotographerList::load()
{
    d->modified = false;

    QFile cfile(d->file);

    if (!cfile.open(QIODevice::ReadOnly))
        return false;

    QDomDocument doc("photographerlist");
    if (!doc.setContent(&cfile))
        return false;

    QDomElement docElem = doc.documentElement();
    if (docElem.tagName()!="photographerlist")
        return false;

    for (QDomNode n = docElem.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement e = n.toElement();
        if (e.isNull()) continue;
        if (e.tagName() != "item") continue;

        Photographer *photographer = new Photographer();
        photographer->setAuthor(e.attribute("author"));
        photographer->setAuthorName(e.attribute("authorname"));
        photographer->setCredit(e.attribute("credit"));
        photographer->setSource(e.attribute("source"));
        photographer->setCopyright(e.attribute("copyright"));
        insertPrivate(photographer);
    }

    return true;
}

bool PhotographerList::save()
{
    // If not modified don't save the file
    if (!d->modified)
        return true;

    QDomDocument doc("photographerlist");
    doc.setContent(QString("<!DOCTYPE XMLPhotographerList><photographerlist version=\"1.0\" client=\"digikam\"/>"));

    QDomElement docElem=doc.documentElement();

    foreach (Photographer *photographer, d->pList)
    {
       QDomElement elem = doc.createElement("item");
       elem.setAttribute("author",     photographer->author());
       elem.setAttribute("authorname", photographer->authorName());
       elem.setAttribute("credit",     photographer->credit());
       elem.setAttribute("source",     photographer->source());
       elem.setAttribute("copyright",  photographer->copyright());
       docElem.appendChild(elem);
    }

    QFile cfile(d->file);
    if (!cfile.open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(&cfile);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream.setAutoDetectUnicode(true);
    stream << doc.toString();
    cfile.close();

    return true;
}

void PhotographerList::insert(Photographer* photographer)
{
    if (!photographer) return;

    d->modified = true;
    insertPrivate(photographer);
}

void PhotographerList::remove(Photographer* photographer)
{
    if (!photographer) return;

    d->modified = true;
    removePrivate(photographer);
}

void PhotographerList::insertPrivate(Photographer* photographer)
{
    if (!photographer) return;
    d->pList.append(photographer);
    emit signalPhotographerAdded(photographer);
}

void PhotographerList::removePrivate(Photographer* photographer)
{
    if (!photographer) return;

    emit signalPhotographerRemoved(photographer);

    int i = d->pList.indexOf(photographer);
    if (i != -1)
        delete d->pList.takeAt(i);
}

void PhotographerList::clear()
{
    while (!d->pList.isEmpty())
        removePrivate(d->pList.first());
}

QList<Photographer*>* PhotographerList::photographerList()
{
    return &d->pList;
}

Photographer* PhotographerList::find(const QString& author) const
{
    foreach (Photographer *photographer, d->pList)
    {
        if (photographer->author() == author)
            return photographer;
    }
    return 0;
}

}  // namespace Digikam
