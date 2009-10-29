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

// Qt includes

#include <QDataStream>
#include <QDateTime>
#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimgloaderobserver.h"

namespace Digikam
{

class BatchToolObserver;

class BatchToolPriv
{

public:

    BatchToolPriv()
    {
        settingsWidget     = 0;
        exifSetOrientation = true;
        cancel             = false;
        last               = false;
        observer           = 0;
    }

    bool                       exifSetOrientation;
    bool                       cancel;
    bool                       last;

    QString                    toolTitle;          // User friendly tool title.
    QString                    toolDescription;    // User friendly tool description.

    QWidget                   *settingsWidget;

    KIcon                      toolIcon;

    KUrl                       inputUrl;
    KUrl                       outputUrl;
    KUrl                       workingUrl;

    DImg                       image;

    BatchToolSettings          settings;

    BatchToolObserver         *observer;

    BatchTool::BatchToolGroup  toolGroup;
};

class BatchToolObserver : public DImgLoaderObserver
{

public:

    BatchToolObserver(BatchToolPriv* priv)
        : DImgLoaderObserver(), d(priv)
    {
    }

    ~BatchToolObserver()
    {
    }

    bool continueQuery(const DImg*)
    {
        return !d->cancel;
    }

    BatchToolPriv* const d;
};

BatchTool::BatchTool(const QString& name, BatchToolGroup group, QObject* parent)
         : QObject(parent), d(new BatchToolPriv)
{
    d->observer  = new BatchToolObserver(d);
    d->toolGroup = group;
    setObjectName(name);

    // NOTE: see B.K.O #209225 : signal/slot connection used internally to prevent crash when settings
    // are assigned to settings widget by main thread to tool thread.

    connect(this, SIGNAL(signalSettingsChanged(const BatchToolSettings&)),
            this, SLOT(slotAssignSettings2Widget()));
}

BatchTool::~BatchTool()
{
    delete d->settingsWidget;
    delete d->observer;
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

void BatchTool::slotResetSettingsToDefault()
{
    setSettings(defaultSettings());
}

void BatchTool::setSettings(const BatchToolSettings& settings)
{
    d->settings = settings;
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

    QString path(workingUrl().toLocalFile());
    path.append("/.");
    path.append(QString::number(QDateTime::currentDateTime().toTime_t()));
    path.append("-");
    path.append(fi.fileName());
    kDebug() << "path: " << path;

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

    return d->image.load(inputUrl().toLocalFile(), d->observer);
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
        return( d->image.save(outputUrl().toLocalFile(), format, d->observer) );
    }

    d->image.updateMetadata(frm, QString(), getExifSetOrientation());
    bool b   = d->image.save(outputUrl().toLocalFile(), frm, d->observer);
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

    kDebug() << "Tool:       " << toolTitle();
    kDebug() << "Input url:  " << inputUrl();
    kDebug() << "Output url: " << outputUrl();
    kDebug() << "Settings:   ";

    BatchToolSettings prm = settings();
    for (BatchToolSettings::const_iterator it = prm.constBegin() ; it != prm.constEnd() ; ++it)
    {
        kDebug() << "   " << it.key() << ": " << it.value();
    }

    return toolOperations();
}

}  // namespace Digikam
