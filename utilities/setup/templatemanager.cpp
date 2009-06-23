/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : metadata template manager.
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

#include "templatemanager.h"
#include "templatemanager.moc"

// Qt includes

#include <QString>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>
#include <QTextCodec>

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "photographer.h"

namespace Digikam
{

TemplateManager* TemplateManager::m_defaultManager = 0;

TemplateManager* TemplateManager::defaultManager()
{
    return m_defaultManager;
}

class TemplateManagerPrivate
{
public:

    TemplateManagerPrivate()
    {
        modified = false;
    }

    bool                 modified;

    QList<Photographer*> pList;
    QString              file;
};

TemplateManager::TemplateManager(QObject *parent, const QString& file)
                : QObject(parent), d(new TemplateManagerPrivate)
{
    d->file = file;
    if (!m_defaultManager)
        m_defaultManager = this;
}

TemplateManager::~TemplateManager()
{
    save();
    clear();
    delete d;

    if (m_defaultManager == this)
        m_defaultManager = 0;
}

bool TemplateManager::load()
{
    d->modified = false;

    QFile file(d->file);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDomDocument doc("templatelist");
    if (!doc.setContent(&file))
        return false;

    QDomElement docElem = doc.documentElement();
    if (docElem.tagName() != "templatelist")
        return false;

    for (QDomNode n = docElem.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement e = n.toElement();
        if (e.isNull()) continue;
        if (e.tagName() != "item") continue;

        Photographer *photographer = new Photographer();
        photographer->setAuthor(e.attribute("author"));
        photographer->setAuthorPosition(e.attribute("authorposition"));
        photographer->setCredit(e.attribute("credit"));
        photographer->setCopyright(e.attribute("copyright"));
        photographer->setRightUsageTerms(e.attribute("rightusageterms"));
        photographer->setSource(e.attribute("source"));
        photographer->setInstructions(e.attribute("instructions"));
        insertPrivate(photographer);
    }

    return true;
}

bool TemplateManager::save()
{
    // If not modified don't save the file
    if (!d->modified)
        return true;

    QDomDocument doc("templatelist");
    doc.setContent(QString("<!DOCTYPE XMLTemplateList><templatelist version=\"1.0\" client=\"digikam\"/>"));

    QDomElement docElem = doc.documentElement();

    foreach (Photographer *photographer, d->pList)
    {
       QDomElement elem = doc.createElement("item");
       elem.setAttribute("author",          photographer->author());
       elem.setAttribute("authorposition",  photographer->authorPosition());
       elem.setAttribute("credit",          photographer->credit());
       elem.setAttribute("copyright",       photographer->copyright());
       elem.setAttribute("rightusageterms", photographer->rightUsageTerms());
       elem.setAttribute("source",          photographer->source());
       elem.setAttribute("instructions",    photographer->instructions());
       docElem.appendChild(elem);
    }

    QFile file(d->file);

    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream.setAutoDetectUnicode(true);
    stream << doc.toString();
    file.close();

    return true;
}

void TemplateManager::insert(Photographer* photographer)
{
    if (!photographer) return;

    d->modified = true;
    insertPrivate(photographer);
}

void TemplateManager::remove(Photographer* photographer)
{
    if (!photographer) return;

    d->modified = true;
    removePrivate(photographer);
}

void TemplateManager::insertPrivate(Photographer* photographer)
{
    if (!photographer) return;
    d->pList.append(photographer);
    emit signalPhotographerAdded(photographer);
}

void TemplateManager::removePrivate(Photographer* photographer)
{
    if (!photographer) return;

    emit signalPhotographerRemoved(photographer);

    int i = d->pList.indexOf(photographer);
    if (i != -1)
        delete d->pList.takeAt(i);
}

void TemplateManager::clear()
{
    while (!d->pList.isEmpty())
        removePrivate(d->pList.first());
}

QList<Photographer*>* TemplateManager::templateList()
{
    return &d->pList;
}

Photographer* TemplateManager::find(const QString& author) const
{
    foreach (Photographer *photographer, d->pList)
    {
        if (photographer->author() == author)
            return photographer;
    }
    return 0;
}

}  // namespace Digikam
