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

#include "template.h"

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
        : unknowTitle(QString("_UNKNOW_TEMPLATE_")),
          removeTitle(QString("_REMOVE_TEMPLATE_"))
    {
        modified       = false;
        removeTemplate = 0;
        unknowTemplate = 0;
    }

    bool             modified;

    QList<Template*> pList;
    QString          file;

    Template*        unknowTemplate;  // Used to identify unregistered template informations found in metadata.
    Template*        removeTemplate;  // Used to identify template information to remove ferom metadata.

    const QString    unknowTitle;
    const QString    removeTitle;
};

TemplateManager::TemplateManager(QObject *parent, const QString& file)
                : QObject(parent), d(new TemplateManagerPrivate)
{
    d->unknowTemplate = new Template();
    d->unknowTemplate->setTemplateTitle(d->unknowTitle);

    d->removeTemplate = new Template();
    d->removeTemplate->setTemplateTitle(d->removeTitle);

    d->file = file;
    if (!m_defaultManager)
        m_defaultManager = this;

    load();
}

TemplateManager::~TemplateManager()
{
    save();
    clear();
    delete d->unknowTemplate;
    delete d->removeTemplate;
    delete d;

    if (m_defaultManager == this)
        m_defaultManager = 0;
}

Template* TemplateManager::unknowTemplate() const
{
    return d->unknowTemplate;
}

Template* TemplateManager::removeTemplate() const
{
    return d->removeTemplate;
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

        Template *t = new Template();
        t->setTemplateTitle(e.attribute("templatetitle"));
        t->setAuthors(e.attribute("authors").split(";", QString::SkipEmptyParts));
        t->setAuthorsPosition(e.attribute("authorsposition"));
        t->setCredit(e.attribute("credit"));
        t->setCopyright(e.attribute("copyright"));
        t->setRightUsageTerms(e.attribute("rightusageterms"));
        t->setSource(e.attribute("source"));
        t->setInstructions(e.attribute("instructions"));
        insertPrivate(t);
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

    foreach (Template *t, d->pList)
    {
        QDomElement elem = doc.createElement("item");
        elem.setAttribute("templatetitle",   t->templateTitle());
        elem.setAttribute("authors",         t->authors().join(";"));
        elem.setAttribute("authorsposition", t->authorsPosition());
        elem.setAttribute("credit",          t->credit());
        elem.setAttribute("copyright",       t->copyright());
        elem.setAttribute("rightusageterms", t->rightUsageTerms());
        elem.setAttribute("source",          t->source());
        elem.setAttribute("instructions",    t->instructions());
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

void TemplateManager::insert(Template* t)
{
    if (!t) return;

    d->modified = true;
    insertPrivate(t);
}

void TemplateManager::remove(Template* t)
{
    if (!t) return;

    d->modified = true;
    removePrivate(t);
}

void TemplateManager::insertPrivate(Template* t)
{
    if (!t) return;
    d->pList.append(t);
    emit signalTemplateAdded(t);
}

void TemplateManager::removePrivate(Template* t)
{
    if (!t) return;

    emit signalTemplateRemoved(t);

    int i = d->pList.indexOf(t);
    if (i != -1)
        delete d->pList.takeAt(i);
}

void TemplateManager::clear()
{
    while (!d->pList.isEmpty())
        removePrivate(d->pList.first());
}

QList<Template*>* TemplateManager::templateList()
{
    return &d->pList;
}

Template* TemplateManager::findByTitle(const QString& title) const
{
    foreach (Template *t, d->pList)
    {
        if (t->templateTitle() == title)
            return t;
    }
    return d->unknowTemplate;
}

Template* TemplateManager::findByContents(const Template& tref) const
{
    foreach (Template *t, d->pList)
    {
        if (*t == tref)
        {
            return t;
        }
    }
    return d->unknowTemplate;
}

Template* TemplateManager::fromIndex(int index) const
{
    if (index >= 0 && index < d->pList.size())
        return d->pList[index];

    return 0;
}

}  // namespace Digikam
