/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : metadata template manager.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QMutex>
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
        :mutex(QMutex::Recursive)
    {
        modified = false;
    }

    bool            modified;

    QList<Template> pList;
    QString         file;

    QMutex          mutex;
};

TemplateManager::TemplateManager(QObject *parent, const QString& file)
                : QObject(parent), d(new TemplateManagerPrivate)
{
    d->file = file;
    if (!m_defaultManager)
        m_defaultManager = this;

    load();
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
        if (e.tagName() != "template") continue;

        Template t;

        for (QDomNode n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
        {
            QDomElement e2 = n2.toElement();
            if (e2.isNull()) continue;
            QString name2  = e2.tagName();
            QString val2   = e2.attribute(QString::fromLatin1("value"));

            if (name2 == QString::fromLatin1("templatetitle"))
            {
                t.setTemplateTitle(val2);
            }
            else if (name2 == QString::fromLatin1("authorsposition"))
            {
                t.setAuthorsPosition(val2);
            }
            else if (name2 == QString::fromLatin1("credit"))
            {
                t.setCredit(val2);
            }
            else if (name2 == QString::fromLatin1("source"))
            {
                t.setSource(val2);
            }
            else if (name2 == QString::fromLatin1("instructions"))
            {
                t.setInstructions(val2);
            }
            else if (name2 == QString::fromLatin1("authors"))
            {
                QStringList list;
                for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                {
                    QDomElement e3 = n3.toElement();
                    QString key    = e3.tagName();
                    QString val    = e3.attribute(QString::fromLatin1("value"));

                    if (key == QString::fromLatin1("name"))
                    {
                        if (val.isEmpty()) continue;
                        list.append(val);
                    }
                }
                t.setAuthors(list);
            }
            else if (name2 == QString::fromLatin1("copyright"))
            {
                KExiv2::AltLangMap copyrights;
                for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                {
                    QDomElement e3 = n3.toElement();
                    QString key    = e3.tagName();
                    QString val    = e3.attribute(QString::fromLatin1("value"));
                    copyrights.insert(key, val);
                }
                t.setCopyright(copyrights);
            }
            else if (name2 == QString::fromLatin1("rightusageterms"))
            {
                KExiv2::AltLangMap usages;
                for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                {
                    QDomElement e3 = n3.toElement();
                    QString key    = e3.tagName();
                    QString val    = e3.attribute(QString::fromLatin1("value"));
                    usages.insert(key, val);
                }
                t.setRightUsageTerms(usages);
            }
        }

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
    doc.setContent(QString("<!DOCTYPE XMLTemplateList><templatelist version=\"2.0\" client=\"digikam\" encoding=\"UTF-8\"/>"));
    QDomElement docElem = doc.documentElement();

    {
        QMutexLocker lock(&d->mutex);

        foreach (const Template& t, d->pList)
        {
            QDomElement elem = doc.createElement("template");

            QDomElement templatetitle = doc.createElement(QString::fromLatin1("templatetitle"));
            templatetitle.setAttribute(QString::fromLatin1("value"), t.templateTitle());
            elem.appendChild(templatetitle);

            QDomElement authorsposition = doc.createElement(QString::fromLatin1("authorsposition"));
            authorsposition.setAttribute(QString::fromLatin1("value"), t.authorsPosition());
            elem.appendChild(authorsposition);

            QDomElement credit = doc.createElement(QString::fromLatin1("credit"));
            credit.setAttribute(QString::fromLatin1("value"), t.credit());
            elem.appendChild(credit);

            QDomElement source = doc.createElement(QString::fromLatin1("source"));
            source.setAttribute(QString::fromLatin1("value"), t.source());
            elem.appendChild(source);

            QDomElement instructions = doc.createElement(QString::fromLatin1("instructions"));
            instructions.setAttribute(QString::fromLatin1("value"), t.instructions());
            elem.appendChild(instructions);

            QDomElement authors = doc.createElement(QString::fromLatin1("authors"));
            elem.appendChild(authors);
            foreach (QString name, t.authors())
            {
                QDomElement e = doc.createElement(QString::fromLatin1("name"));
                e.setAttribute(QString::fromLatin1("value"), name);
                authors.appendChild(e);
            }

            QDomElement copyright     = doc.createElement(QString::fromLatin1("copyright"));
            elem.appendChild(copyright);
            KExiv2::AltLangMap rights = t.copyright();
            KExiv2::AltLangMap::const_iterator it;
            for (it = rights.constBegin() ; it != rights.constEnd() ; ++it)
            {
                QDomElement e = doc.createElement(it.key());
                e.setAttribute(QString::fromLatin1("value"), it.value());
                copyright.appendChild(e);
            }

            QDomElement rightusageterms = doc.createElement(QString::fromLatin1("rightusageterms"));
            elem.appendChild(rightusageterms);
            KExiv2::AltLangMap usages   = t.rightUsageTerms();
            KExiv2::AltLangMap::const_iterator it2;
            for (it2 = usages.constBegin() ; it2 != usages.constEnd() ; ++it2)
            {
                QDomElement e = doc.createElement(it2.key());
                e.setAttribute(QString::fromLatin1("value"), it2.value());
                rightusageterms.appendChild(e);
            }

            docElem.appendChild(elem);
        }
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

void TemplateManager::insert(const Template& t)
{
    if (t.isNull()) return;

    d->modified = true;
    insertPrivate(t);
}

void TemplateManager::remove(const Template& t)
{
    if (t.isNull()) return;

    d->modified = true;
    removePrivate(t);
}

void TemplateManager::insertPrivate(const Template& t)
{
    if (t.isNull()) return;

    {
        QMutexLocker lock(&d->mutex);
        d->pList.append(t);
    }

    emit signalTemplateAdded(t);
}

void TemplateManager::removePrivate(const Template& t)
{
    if (t.isNull()) return;

    {
        QMutexLocker lock(&d->mutex);
        for (QList<Template>::iterator it = d->pList.begin(); it != d->pList.end(); ++it)
        {
            if (it->templateTitle() == t.templateTitle())
            {
                it = d->pList.erase(it);
                break;
            }
        }
    }

    emit signalTemplateRemoved(t);
}

void TemplateManager::clear()
{
    QList<Template> oldTemplates = d->pList;

    {
        QMutexLocker lock(&d->mutex);
        d->pList.clear();
    }

    foreach (const Template& t, d->pList)
    {
        emit signalTemplateRemoved(t);
    }
}

QList<Template> TemplateManager::templateList()
{
    return d->pList;
}

Template TemplateManager::findByTitle(const QString& title) const
{
    QMutexLocker lock(&d->mutex);

    foreach (const Template& t, d->pList)
    {
        if (t.templateTitle() == title)
            return t;
    }
    return Template();
}

Template TemplateManager::findByContents(const Template& tref) const
{
    QMutexLocker lock(&d->mutex);

    foreach (const Template& t, d->pList)
    {
        if (t == tref)
        {
            return t;
        }
    }
    return Template();
}

Template TemplateManager::fromIndex(int index) const
{
    QMutexLocker lock(&d->mutex);

    if (index >= 0 && index < d->pList.size())
        return d->pList[index];

    return Template();
}

}  // namespace Digikam
