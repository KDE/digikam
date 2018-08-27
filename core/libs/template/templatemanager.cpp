/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : metadata template manager.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QMutex>
#include <QTextStream>
#include <QTextCodec>
#include <QStandardPaths>

// Local includes

#include "template.h"

namespace Digikam
{

class Q_DECL_HIDDEN TemplateManager::Private
{
public:

    explicit Private()
        :mutex()
    {
        modified = false;
    }

    bool            modified;

    QList<Template> pList;
    QString         file;

    QMutex          mutex;
};

class TemplateManagerCreator
{
public:

    TemplateManager object;
};

Q_GLOBAL_STATIC(TemplateManagerCreator, creator)

TemplateManager* TemplateManager::defaultManager()
{
    return &creator->object;
}

TemplateManager::TemplateManager()
    : d(new Private)
{
    d->file = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/template.xml");

    load();
}

TemplateManager::~TemplateManager()
{
    save();
    clear();
    delete d;
}

bool TemplateManager::load()
{
    d->modified = false;

    QFile file(d->file);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDomDocument doc(QLatin1String("templatelist"));

    if (!doc.setContent(&file))
    {
        return false;
    }

    QDomElement docElem = doc.documentElement();

    if (docElem.tagName() != QLatin1String("templatelist"))
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

        if (e.tagName() != QLatin1String("template"))
        {
            continue;
        }

        Template             t;
        IptcCoreLocationInfo locationInf;
        IptcCoreContactInfo  contactInf;

        for (QDomNode n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
        {
            QDomElement e2 = n2.toElement();

            if (e2.isNull())
            {
                continue;
            }

            QString name2  = e2.tagName();
            QString val2   = e2.attribute(QLatin1String("value"));

            if (name2 == QLatin1String("templatetitle"))
            {
                t.setTemplateTitle(val2);
            }
            else if (name2 == QLatin1String("authorsposition"))
            {
                t.setAuthorsPosition(val2);
            }
            else if (name2 == QLatin1String("credit"))
            {
                t.setCredit(val2);
            }
            else if (name2 == QLatin1String("source"))
            {
                t.setSource(val2);
            }
            else if (name2 == QLatin1String("instructions"))
            {
                t.setInstructions(val2);
            }
            else if (name2 == QLatin1String("authors"))
            {
                QStringList list;

                for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                {
                    QDomElement e3 = n3.toElement();
                    QString key    = e3.tagName();
                    QString val    = e3.attribute(QLatin1String("value"));

                    if (key == QLatin1String("name"))
                    {
                        if (val.isEmpty())
                        {
                            continue;
                        }

                        list.append(val);
                    }
                }

                t.setAuthors(list);
            }
            else if (name2 == QLatin1String("copyright"))
            {
                MetaEngine::AltLangMap copyrights;

                for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                {
                    QDomElement e3 = n3.toElement();
                    QString key    = e3.tagName();
                    QString val    = e3.attribute(QLatin1String("value"));
                    copyrights.insert(key, val);
                }

                t.setCopyright(copyrights);
            }
            else if (name2 == QLatin1String("rightusageterms"))
            {
                MetaEngine::AltLangMap usages;

                for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                {
                    QDomElement e3 = n3.toElement();
                    QString key    = e3.tagName();
                    QString val    = e3.attribute(QLatin1String("value"));
                    usages.insert(key, val);
                }

                t.setRightUsageTerms(usages);
            }
            else if (name2 == QLatin1String("locationcountry"))
            {
                locationInf.country = val2;
            }
            else if (name2 == QLatin1String("locationcountrycode"))
            {
                locationInf.countryCode = val2;
            }
            else if (name2 == QLatin1String("locationprovincestate"))
            {
                locationInf.provinceState = val2;
            }
            else if (name2 == QLatin1String("locationcity"))
            {
                locationInf.city = val2;
            }
            else if (name2 == QLatin1String("locationlocation"))
            {
                locationInf.location = val2;
            }
            else if (name2 == QLatin1String("contactcity"))
            {
                contactInf.city = val2;
            }
            else if (name2 == QLatin1String("contactcountry"))
            {
                contactInf.country = val2;
            }
            else if (name2 == QLatin1String("contactaddress"))
            {
                contactInf.address = val2;
            }
            else if (name2 == QLatin1String("contactpostalcode"))
            {
                contactInf.postalCode = val2;
            }
            else if (name2 == QLatin1String("contactprovincestate"))
            {
                contactInf.provinceState = val2;
            }
            else if (name2 == QLatin1String("contactemail"))
            {
                contactInf.email = val2;
            }
            else if (name2 == QLatin1String("contactphone"))
            {
                contactInf.phone = val2;
            }
            else if (name2 == QLatin1String("contactweburl"))
            {
                contactInf.webUrl = val2;
            }
            else if (name2 == QLatin1String("subjects"))
            {
                QStringList list;

                for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                {
                    QDomElement e3 = n3.toElement();
                    QString key    = e3.tagName();
                    QString val    = e3.attribute(QLatin1String("value"));

                    if (key == QLatin1String("subject"))
                    {
                        if (val.isEmpty())
                        {
                            continue;
                        }

                        list.append(val);
                    }
                }

                t.setIptcSubjects(list);
            }
        }

        t.setLocationInfo(locationInf);
        t.setContactInfo(contactInf);
        insertPrivate(t);
    }

    return true;
}

bool TemplateManager::save()
{
    // If not modified don't save the file
    if (!d->modified)
    {
        return true;
    }

    QDomDocument doc(QLatin1String("templatelist"));
    doc.setContent(QLatin1String("<!DOCTYPE XMLTemplateList><templatelist version=\"2.0\" client=\"digikam\" encoding=\"UTF-8\"/>"));
    QDomElement docElem = doc.documentElement();

    {
        QMutexLocker lock(&d->mutex);

        foreach(const Template& t, d->pList)
        {
            QDomElement elem = doc.createElement(QLatin1String("template"));

            QDomElement templatetitle = doc.createElement(QLatin1String("templatetitle"));
            templatetitle.setAttribute(QLatin1String("value"), t.templateTitle());
            elem.appendChild(templatetitle);

            QDomElement authorsposition = doc.createElement(QLatin1String("authorsposition"));
            authorsposition.setAttribute(QLatin1String("value"), t.authorsPosition());
            elem.appendChild(authorsposition);

            QDomElement credit = doc.createElement(QLatin1String("credit"));
            credit.setAttribute(QLatin1String("value"), t.credit());
            elem.appendChild(credit);

            QDomElement source = doc.createElement(QLatin1String("source"));
            source.setAttribute(QLatin1String("value"), t.source());
            elem.appendChild(source);

            QDomElement instructions = doc.createElement(QLatin1String("instructions"));
            instructions.setAttribute(QLatin1String("value"), t.instructions());
            elem.appendChild(instructions);

            QDomElement authors = doc.createElement(QLatin1String("authors"));
            elem.appendChild(authors);

            foreach(const QString& name, t.authors())
            {
                QDomElement e = doc.createElement(QLatin1String("name"));
                e.setAttribute(QLatin1String("value"), name);
                authors.appendChild(e);
            }

            QDomElement copyright = doc.createElement(QLatin1String("copyright"));
            elem.appendChild(copyright);
            MetaEngine::AltLangMap rights = t.copyright();
            MetaEngine::AltLangMap::const_iterator it;

            for (it = rights.constBegin() ; it != rights.constEnd() ; ++it)
            {
                QDomElement e = doc.createElement(it.key());
                e.setAttribute(QLatin1String("value"), it.value());
                copyright.appendChild(e);
            }

            QDomElement rightusageterms = doc.createElement(QLatin1String("rightusageterms"));
            elem.appendChild(rightusageterms);
            MetaEngine::AltLangMap usages   = t.rightUsageTerms();
            MetaEngine::AltLangMap::const_iterator it2;

            for (it2 = usages.constBegin() ; it2 != usages.constEnd() ; ++it2)
            {
                QDomElement e = doc.createElement(it2.key());
                e.setAttribute(QLatin1String("value"), it2.value());
                rightusageterms.appendChild(e);
            }

            docElem.appendChild(elem);

            QDomElement locationcountry = doc.createElement(QLatin1String("locationcountry"));
            locationcountry.setAttribute(QLatin1String("value"), t.locationInfo().country);
            elem.appendChild(locationcountry);

            QDomElement locationcountrycode = doc.createElement(QLatin1String("locationcountrycode"));
            locationcountrycode.setAttribute(QLatin1String("value"), t.locationInfo().countryCode);
            elem.appendChild(locationcountrycode);

            QDomElement locationprovincestate = doc.createElement(QLatin1String("locationprovincestate"));
            locationprovincestate.setAttribute(QLatin1String("value"), t.locationInfo().provinceState);
            elem.appendChild(locationprovincestate);

            QDomElement locationcity = doc.createElement(QLatin1String("locationcity"));
            locationcity.setAttribute(QLatin1String("value"), t.locationInfo().city);
            elem.appendChild(locationcity);

            QDomElement locationlocation = doc.createElement(QLatin1String("locationlocation"));
            locationlocation.setAttribute(QLatin1String("value"), t.locationInfo().location);
            elem.appendChild(locationlocation);

            QDomElement contactcity = doc.createElement(QLatin1String("contactcity"));
            contactcity.setAttribute(QLatin1String("value"), t.contactInfo().city);
            elem.appendChild(contactcity);

            QDomElement contactcountry = doc.createElement(QLatin1String("contactcountry"));
            contactcountry.setAttribute(QLatin1String("value"), t.contactInfo().country);
            elem.appendChild(contactcountry);

            QDomElement contactaddress = doc.createElement(QLatin1String("contactaddress"));
            contactaddress.setAttribute(QLatin1String("value"), t.contactInfo().address);
            elem.appendChild(contactaddress);

            QDomElement contactpostalcode = doc.createElement(QLatin1String("contactpostalcode"));
            contactpostalcode.setAttribute(QLatin1String("value"), t.contactInfo().postalCode);
            elem.appendChild(contactpostalcode);

            QDomElement contactprovincestate = doc.createElement(QLatin1String("contactprovincestate"));
            contactprovincestate.setAttribute(QLatin1String("value"), t.contactInfo().provinceState);
            elem.appendChild(contactprovincestate);

            QDomElement contactemail = doc.createElement(QLatin1String("contactemail"));
            contactemail.setAttribute(QLatin1String("value"), t.contactInfo().email);
            elem.appendChild(contactemail);

            QDomElement contactphone = doc.createElement(QLatin1String("contactphone"));
            contactphone.setAttribute(QLatin1String("value"), t.contactInfo().phone);
            elem.appendChild(contactphone);

            QDomElement contactweburl = doc.createElement(QLatin1String("contactweburl"));
            contactweburl.setAttribute(QLatin1String("value"), t.contactInfo().webUrl);
            elem.appendChild(contactweburl);

            QDomElement subjects = doc.createElement(QLatin1String("subjects"));
            elem.appendChild(subjects);

            foreach(const QString& subject, t.IptcSubjects())
            {
                QDomElement e = doc.createElement(QLatin1String("subject"));
                e.setAttribute(QLatin1String("value"), subject);
                subjects.appendChild(e);
            }
        }
    }

    QFile file(d->file);

    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream.setAutoDetectUnicode(true);
    stream << doc.toString();
    file.close();

    return true;
}

void TemplateManager::insert(const Template& t)
{
    if (t.isNull())
    {
        return;
    }

    d->modified = true;
    insertPrivate(t);
}

void TemplateManager::remove(const Template& t)
{
    if (t.isNull())
    {
        return;
    }

    d->modified = true;
    removePrivate(t);
}

void TemplateManager::insertPrivate(const Template& t)
{
    if (t.isNull())
    {
        return;
    }

    {
        QMutexLocker lock(&d->mutex);
        d->pList.append(t);
    }

    emit signalTemplateAdded(t);
}

void TemplateManager::removePrivate(const Template& t)
{
    if (t.isNull())
    {
        return;
    }

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

    foreach(const Template& t, d->pList)
    {
        emit signalTemplateRemoved(t);
    }
}

QList<Template> TemplateManager::templateList() const
{
    return d->pList;
}

Template TemplateManager::findByTitle(const QString& title) const
{
    QMutexLocker lock(&d->mutex);

    foreach(const Template& t, d->pList)
    {
        if (t.templateTitle() == title)
        {
            return t;
        }
    }

    return Template();
}

Template TemplateManager::findByContents(const Template& tref) const
{
    QMutexLocker lock(&d->mutex);

    foreach(const Template& t, d->pList)
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
    {
        return d->pList.at(index);
    }

    return Template();
}

} // namespace Digikam
