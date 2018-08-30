/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-12-17
 * Description : workflow manager.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "workflowmanager.h"

// Qt includes

#include <QMutex>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>
#include <QTextCodec>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "queuesettings.h"
#include "batchtoolsfactory.h"

namespace Digikam
{

class Q_DECL_HIDDEN WorkflowManager::Private
{
public:

    explicit Private()
        :mutex()
    {
        modified = false;
    }

    bool            modified;

    QList<Workflow> qList;
    QString         file;

    QMutex          mutex;
};

// ------------------------------------------------------------------------------------------

class WorkflowManagerCreator
{
public:

    WorkflowManager object;
};

Q_GLOBAL_STATIC(WorkflowManagerCreator, creator)

WorkflowManager* WorkflowManager::instance()
{
    return &creator->object;
}

// ------------------------------------------------------------------------------------------

WorkflowManager::WorkflowManager()
    : d(new Private)
{
    d->file = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/queue.xml");
}

WorkflowManager::~WorkflowManager()
{
    save();
    clear();
    delete d;
}

void WorkflowManager::insert(const Workflow& q)
{
    if (q.title.isNull())
    {
        return;
    }

    d->modified = true;
    insertPrivate(q);
}

void WorkflowManager::remove(const Workflow& q)
{
    if (q.title.isNull())
    {
        return;
    }

    d->modified = true;
    removePrivate(q);
}

void WorkflowManager::insertPrivate(const Workflow& q)
{
    if (q.title.isNull())
    {
        return;
    }

    {
        QMutexLocker lock(&d->mutex);
        d->qList.append(q);
        qCDebug(DIGIKAM_GENERAL_LOG) << "add : " << q.title;
    }

    emit signalQueueSettingsAdded(q.title);
}

void WorkflowManager::removePrivate(const Workflow& q)
{
    if (q.title.isNull())
    {
        return;
    }

    {
        QMutexLocker lock(&d->mutex);

        for (QList<Workflow>::iterator it = d->qList.begin(); it != d->qList.end(); ++it)
        {
            if (it->title == q.title)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Remove " << it->title << " from Workflow list";
                it = d->qList.erase(it);
                break;
            }
        }
    }

    emit signalQueueSettingsRemoved(q.title);
}

Workflow WorkflowManager::findByTitle(const QString& title) const
{
    QMutexLocker lock(&d->mutex);

    foreach(const Workflow& q, d->qList)
    {
        if (q.title == title)
        {
            return q;
        }
    }

    return Workflow();
}

void WorkflowManager::clear()
{
    QList<Workflow> oldQueues = d->qList;

    {
        QMutexLocker lock(&d->mutex);
        d->qList.clear();
    }

    foreach(const Workflow& q, d->qList)
    {
        emit signalQueueSettingsRemoved(q.title);
    }
}

QList<Workflow> WorkflowManager::queueSettingsList() const
{
    return d->qList;
}

bool WorkflowManager::save()
{
    // If not modified don't save the file
    if (!d->modified)
    {
        return true;
    }

    QDomDocument doc(QLatin1String("queuelist"));
    doc.setContent(QString::fromUtf8("<!DOCTYPE XMLQueueList><queuelist version=\"2.0\" client=\"digikam\" encoding=\"UTF-8\"/>"));
    QDomElement docElem = doc.documentElement();

    {
        QMutexLocker lock(&d->mutex);

        foreach(const Workflow& q, d->qList)
        {

            QDomElement elm = doc.createElement(QLatin1String("queue"));
            QDomElement data;

            data = doc.createElement(QLatin1String("queuetitle"));
            data.setAttribute(QLatin1String("value"), q.title);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("queuedesc"));
            data.setAttribute(QLatin1String("value"), q.desc);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("renamingparser"));
            data.setAttribute(QLatin1String("value"), q.qSettings.renamingParser);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("useoriginalalbum"));
            data.setAttribute(QLatin1String("value"), q.qSettings.useOrgAlbum);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("usemulticorecpu"));
            data.setAttribute(QLatin1String("value"), q.qSettings.useMultiCoreCPU);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("workingurl"));
            data.setAttribute(QLatin1String("value"), q.qSettings.workingUrl.toLocalFile());
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("conflictrule"));
            data.setAttribute(QLatin1String("value"), q.qSettings.conflictRule);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("renamingrule"));
            data.setAttribute(QLatin1String("value"), q.qSettings.renamingRule);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("rawloadingrule"));
            data.setAttribute(QLatin1String("value"), q.qSettings.rawLoadingRule);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("jpegcompression"));
            data.setAttribute(QLatin1String("value"), q.qSettings.ioFileSettings.JPEGCompression);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("jpegsubsampling"));
            data.setAttribute(QLatin1String("value"), q.qSettings.ioFileSettings.JPEGSubSampling);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("pngcompression"));
            data.setAttribute(QLatin1String("value"), q.qSettings.ioFileSettings.PNGCompression);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("tiffcompression"));
            data.setAttribute(QLatin1String("value"), q.qSettings.ioFileSettings.TIFFCompression);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("jpeg2000lossless"));
            data.setAttribute(QLatin1String("value"), q.qSettings.ioFileSettings.JPEG2000LossLess);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("jpeg2000compression"));
            data.setAttribute(QLatin1String("value"), q.qSettings.ioFileSettings.JPEG2000Compression);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("pgflossless"));
            data.setAttribute(QLatin1String("value"), q.qSettings.ioFileSettings.PGFLossLess);
            elm.appendChild(data);

            data = doc.createElement(QLatin1String("pgfcompression"));
            data.setAttribute(QLatin1String("value"), q.qSettings.ioFileSettings.PGFCompression);
            elm.appendChild(data);

            // ----------------------

            QDomElement rawdecodingsettings = doc.createElement(QLatin1String("rawdecodingsettings"));
            elm.appendChild(rawdecodingsettings);
            DRawDecoding::decodingSettingsToXml(q.qSettings.rawDecodingSettings, rawdecodingsettings);

            // ----------------------

            foreach(const BatchToolSet& set, q.aTools)
            {
                QDomElement batchtool = doc.createElement(QLatin1String("tool"));
                elm.appendChild(batchtool);

                data = doc.createElement(QLatin1String("toolname"));
                data.setAttribute(QLatin1String("value"), set.name);
                batchtool.appendChild(data);

                data = doc.createElement(QLatin1String("toolgroup"));
                data.setAttribute(QLatin1String("value"), set.group);
                batchtool.appendChild(data);

                data = doc.createElement(QLatin1String("index"));
                data.setAttribute(QLatin1String("value"), set.index);
                batchtool.appendChild(data);

                data = doc.createElement(QLatin1String("version"));
                data.setAttribute(QLatin1String("value"), set.version);
                batchtool.appendChild(data);

                for (BatchToolSettings::const_iterator it = set.settings.constBegin() ; it != set.settings.constEnd() ; ++it)
                {
                    data = doc.createElement(QLatin1String("parameter"));
                    data.setAttribute(QLatin1String("name"),  it.key());
                    data.setAttribute(QLatin1String("type"),  QString::fromUtf8(it.value().typeName()));
                    data.setAttribute(QLatin1String("value"), it.value().toString());
                    batchtool.appendChild(data);
                }
            }

            docElem.appendChild(elm);
        }
    }

    QFile file(d->file);

    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open XML file to store Workflow";
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream.setAutoDetectUnicode(true);
    stream << doc.toString(4);
    file.close();

    return true;
}

bool WorkflowManager::load(QStringList& failed)
{
    d->modified = false;

    QFile file(d->file);

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open XML file to load Workflow";
            return false;
        }

        QDomDocument doc(QLatin1String("queuelist"));

        if (!doc.setContent(&file))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot load Workflow XML file";
            return false;
        }

        QDomElement docElem = doc.documentElement();

        if (docElem.tagName() != QLatin1String("queuelist"))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Workflow XML file do not content Queue List data";
            return false;
        }

        for (QDomNode n = docElem.firstChild(); !n.isNull(); n = n.nextSibling())
        {
            QDomElement e = n.toElement();

            if (e.isNull())
            {
                continue;
            }

            if (e.tagName() != QLatin1String("queue"))
            {
                continue;
            }

            Workflow q;
            bool     versionOk = true;

            for (QDomNode n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
            {
                QDomElement e2 = n2.toElement();

                if (e2.isNull())
                {
                    continue;
                }

                QString name2 = e2.tagName();
                QString val2  = e2.attribute(QLatin1String("value"));
                bool ok       = true;

                if (name2 == QLatin1String("queuetitle"))
                {
                    q.title = val2;
                }
                else if (name2 == QLatin1String("queuedesc"))
                {
                    q.desc = val2;
                }
                else if (name2 == QLatin1String("renamingparser"))
                {
                    q.qSettings.renamingParser = val2;
                }
                else if (name2 == QLatin1String("useoriginalalbum"))
                {
                    q.qSettings.useOrgAlbum = (bool)val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("usemulticorecpu"))
                {
                    q.qSettings.useMultiCoreCPU = (bool)val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("workingurl"))
                {
                    q.qSettings.workingUrl = QUrl::fromLocalFile(val2);
                }
                else if (name2 == QLatin1String("conflictrule"))
                {
                    q.qSettings.conflictRule = (FileSaveConflictBox::ConflictRule)val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("renamingrule"))
                {
                    q.qSettings.renamingRule = (QueueSettings::RenamingRule)val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("rawloadingrule"))
                {
                    q.qSettings.rawLoadingRule = (QueueSettings::RawLoadingRule)val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("jpegcompression"))
                {
                    q.qSettings.ioFileSettings.JPEGCompression = val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("jpegsubsampling"))
                {
                    q.qSettings.ioFileSettings.JPEGSubSampling = val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("pngcompression"))
                {
                    q.qSettings.ioFileSettings.PNGCompression = val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("tiffcompression"))
                {
                    q.qSettings.ioFileSettings.TIFFCompression = (bool)val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("jpeg2000lossless"))
                {
                    q.qSettings.ioFileSettings.JPEG2000LossLess = (bool)val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("jpeg2000compression"))
                {
                    q.qSettings.ioFileSettings.JPEG2000Compression = val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("pgflossless"))
                {
                    q.qSettings.ioFileSettings.PGFLossLess = (bool)val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("pgfcompression"))
                {
                    q.qSettings.ioFileSettings.PGFCompression = val2.toUInt(&ok);
                }
                else if (name2 == QLatin1String("rawdecodingsettings"))
                {
                    DRawDecoding::decodingSettingsFromXml(e2, q.qSettings.rawDecodingSettings);
                }
                else if (name2 == QLatin1String("tool"))
                {
                    BatchToolSet set;

                    for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                    {
                        QDomElement e3 = n3.toElement();

                        if (e3.isNull())
                        {
                            continue;
                        }

                        QString name3  = e3.tagName();
                        QString val3   = e3.attribute(QLatin1String("value"));

                        if (name3 == QLatin1String("toolname"))
                        {
                            set.name = val3;
                        }
                        else if (name3 == QLatin1String("toolgroup"))
                        {
                            set.group = (BatchTool::BatchToolGroup)val3.toInt(&ok);
                        }
                        else if (name3 == QLatin1String("index"))
                        {
                            set.index = val3.toInt(&ok);
                        }
                        else if (name3 == QLatin1String("version"))
                        {
                            set.version = val3.toInt(&ok);
                        }
                        else if (name3 == QLatin1String("parameter"))
                        {
                            QString pname = e3.attribute(QLatin1String("name"));
                            QString type  = e3.attribute(QLatin1String("type"));
                            QVariant var(val3);
                            var.convert(QVariant::nameToType(type.toLatin1().constData()));
/*
                            qCDebug(DIGIKAM_GENERAL_LOG) << "name=" << pname << " :: "
                                                         << "type=" << type << " :: "
                                                         << "value=" << val3 << " :: "
                                                         << "QVariant=" << var;
*/
                            set.settings.insert(pname, var);
                        }
                    }

                    BatchTool* const tool = BatchToolsFactory::instance()->findTool(set.name, set.group);

                    if (tool)
                    {
                        if (set.version == tool->toolVersion())
                        {
                            q.aTools.append(set);
                        }
                    }
                    else
                    {
                        versionOk   = false;
                        d->modified = true;
                    }
                }
            }

            if (versionOk)
            {
                // We only insert workflow if all tools version are compatible
                insertPrivate(q);
            }
            else
            {
                failed.append(QString::fromUtf8("%1 [%2]").arg(q.title).arg(q.desc));
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace Digikam
