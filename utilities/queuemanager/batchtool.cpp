/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tool Container.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchtool.h"
#include "batchtool.moc"

// Qt includes.

#include <QWidget>
#include <QDataStream>
#include <QDateTime>
#include <QFileInfo>

// KDE includes.

#include <kdebug.h>

namespace Digikam
{

class BatchToolPriv
{

public:

    BatchToolPriv()
    {
        settingsWidget     = 0;
        exifSetOrientation = true;
        cancel             = false;
        last               = false;
    }

    bool                      exifSetOrientation;
    bool                      cancel;
    bool                      last;

    QString                   toolTitle;          // User friendly tool title.
    QString                   toolDescription;    // User friendly tool description.

    QWidget*                  settingsWidget;

    KIcon                     toolIcon;

    KUrl                      inputUrl;
    KUrl                      outputUrl;
    KUrl                      workingUrl;

    DImg                      image;

    BatchToolSettings         settings;

    BatchTool::BatchToolGroup toolGroup;
};

BatchTool::BatchTool(const QString& name, BatchToolGroup group, QObject* parent)
         : QObject(parent), d(new BatchToolPriv)
{
    setObjectName(name);
    d->toolGroup = group;
}

BatchTool::~BatchTool()
{
    wait();
    delete d->settingsWidget;
    delete d;
}

BatchTool::BatchToolGroup BatchTool::toolGroup() const
{
    return d->toolGroup;
}

void BatchTool::setToolTitle(const QString& toolTitle)
{
    d->toolTitle = toolTitle;
}

QString BatchTool::toolTitle() const
{
    return d->toolTitle;
}

void BatchTool::setToolDescription(const QString& toolDescription)
{
    d->toolDescription = toolDescription;
}

QString BatchTool::toolDescription() const
{
    return d->toolDescription;
}

void BatchTool::setToolIcon(const KIcon& toolIcon)
{
    d->toolIcon = toolIcon;
}

KIcon BatchTool::toolIcon() const
{
    return d->toolIcon;
}

QWidget* BatchTool::settingsWidget() const
{
    return d->settingsWidget;
}

void BatchTool::setSettingsWidget(QWidget* settingsWidget)
{
    d->settingsWidget = settingsWidget;
}

void BatchTool::setSettings(const BatchToolSettings& settings)
{
    d->settings = settings;
    assignSettings2Widget();
    emit signalSettingsChanged(d->settings);
}

void BatchTool::setInputUrl(const KUrl& inputUrl)
{
    d->inputUrl = inputUrl;
}

KUrl BatchTool::inputUrl() const
{
    return d->inputUrl;
}

void BatchTool::setOutputUrl(const KUrl& outputUrl)
{
    d->outputUrl = outputUrl;
}

KUrl BatchTool::outputUrl() const
{
    return d->outputUrl;
}

QString BatchTool::outputSuffix() const
{
    return QString();
}

void BatchTool::setImageData(const DImg& img)
{
    d->image = img;
}

DImg BatchTool::imageData() const
{
    return d->image;
}

void BatchTool::setExifSetOrientation(bool set)
{
    d->exifSetOrientation = set;
}

bool BatchTool::getExifSetOrientation() const
{
    return d->exifSetOrientation;
}

void BatchTool::setWorkingUrl(const KUrl& workingUrl)
{
    d->workingUrl = workingUrl;
}

KUrl BatchTool::workingUrl() const
{
    return d->workingUrl;
}

void BatchTool::cancel()
{
    d->cancel = true;
}

bool BatchTool::isCancelled()
{
    return d->cancel;
}

BatchToolSettings BatchTool::settings()
{
    return d->settings;
}

void BatchTool::setLastChainedTool(bool last)
{
    d->last = last;
}

bool BatchTool::isLastChainedTool() const
{
    return d->last;
}

void BatchTool::setOutputUrlFromInputUrl()
{
    QFileInfo fi(inputUrl().fileName());

    QString path(workingUrl().path());
    path.append("/.");
    path.append(QString::number(QDateTime::currentDateTime().toTime_t()));
    path.append("-");
    path.append(fi.fileName());
    kDebug(50003) << "path: " << path << endl;

    KUrl url;
    url.setPath(path);
    setOutputUrl(url);

    if (!outputSuffix().isEmpty())
    {
        KUrl url     = outputUrl();
        QString base = url.fileName();
        base.append(QString(".%1").arg(outputSuffix()));
        url.setFileName(base);
        setOutputUrl(url);
    }
}

bool BatchTool::loadToDImg()
{
    if (!d->image.isNull()) return true;

    return d->image.load(inputUrl().path());
}

bool BatchTool::savefromDImg()
{
    if (!isLastChainedTool() && outputSuffix().isEmpty()) return true;

    QString frm = outputSuffix().toUpper();
    if (frm.isEmpty())
    {
        // In case of output support is not set for ex. with all tool which do not convert to new format.
        DImg::FORMAT format = (DImg::FORMAT)(d->image.attribute("detectedFileFormat").toInt());
        d->image.updateMetadata(DImg::formatToMimeType(format), QString(), getExifSetOrientation());
        return( d->image.save(outputUrl().path(), format) );
    }

    d->image.updateMetadata(frm, QString(), getExifSetOrientation());
    bool b   = d->image.save(outputUrl().path(), frm);
    d->image = DImg();
    return b;
}

DImg& BatchTool::image()
{
    return d->image;
}

bool BatchTool::apply()
{
    d->cancel = false;

    kDebug(50003) << "Tool:       " << toolTitle() << endl;
    kDebug(50003) << "Input url:  " << inputUrl()  << endl;
    kDebug(50003) << "Output url: " << outputUrl() << endl;
    kDebug(50003) << "Settings:   " << endl;

    BatchToolSettings prm = settings();
    for (BatchToolSettings::const_iterator it = prm.begin() ; it != prm.end() ; ++it)
    {
        kDebug(50003) << "   " << it.key() << ": " << it.value() << endl;
    }

    return toolOperations();
}

}  // namespace Digikam
