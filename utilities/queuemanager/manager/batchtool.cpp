/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tool Container.
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchtool.moc"

// Qt includes

#include <QDataStream>
#include <QDateTime>
#include <QFileInfo>
#include <QWidget>
#include <QLabel>
#include <QPolygon>
#include <QTemporaryFile>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "dimgbuiltinfilter.h"
#include "dimgloaderobserver.h"
#include "dimgthreadedfilter.h"
#include "filereadwritelock.h"

namespace Digikam
{

class BatchToolObserver;

class BatchTool::Private
{

public:

    Private() :
        exifResetOrientation(false),
        exifCanEditOrientation(true),
        branchHistory(true),
        cancel(false),
        last(false),
        settingsWidget(0),
        observer(0)
    {
    }

    bool                      exifResetOrientation;
    bool                      exifCanEditOrientation;
    bool                      branchHistory;
    bool                      cancel;
    bool                      last;

    QString                   errorMessage;
    QString                   toolTitle;          // User friendly tool title.
    QString                   toolDescription;    // User friendly tool description.

    QWidget*                  settingsWidget;

    KIcon                     toolIcon;

    KUrl                      inputUrl;
    KUrl                      outputUrl;
    KUrl                      workingUrl;

    DImg                      image;

    DRawDecoding              rawDecodingSettings;

    BatchToolSettings         settings;

    BatchToolObserver*        observer;

    BatchTool::BatchToolGroup toolGroup;
};

class BatchToolObserver : public DImgLoaderObserver
{

public:

    BatchToolObserver(BatchTool::Private* const priv)
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

    BatchTool::Private* const d;
};

BatchTool::BatchTool(const QString& name, BatchToolGroup group, QObject* const parent)
    : QObject(parent), d(new Private)
{
    d->observer  = new BatchToolObserver(d);
    d->toolGroup = group;
    setObjectName(name);

    // NOTE: see B.K.O #209225 : signal/slot connection used internally to prevent crash when settings
    // are assigned to settings widget by main thread to tool thread.

    connect(this, SIGNAL(signalAssignSettings2Widget()),
            this, SLOT(slotAssignSettings2Widget()));
}

BatchTool::~BatchTool()
{
    delete d->settingsWidget;
    delete d->observer;
    delete d;
}

QString BatchTool::errorDescription() const
{
    return d->errorMessage;
}

void BatchTool::setErrorDescription(const QString& errmsg)
{
    d->errorMessage = errmsg;
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
    ensureIsInitialized();
    return d->settingsWidget;
}

void BatchTool::setSettingsWidget(QWidget* const settingsWidget)
{
    d->settingsWidget = settingsWidget;
}

void BatchTool::setNoSettingsWidget()
{
    QLabel* label = new QLabel;
    label->setText(i18n("No setting available"));
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    setSettingsWidget(label);
}

void BatchTool::ensureIsInitialized() const
{
    if (!d->settingsWidget)
    {
        // lazy caching: indication for const-cast
        BatchTool* tool = const_cast<BatchTool*>(this);
        tool->setSettingsWidget(tool->createSettingsWidget());
    }
}

QWidget* BatchTool::createSettingsWidget()
{
    // default implementation: return 0.
    return 0;
}

void BatchTool::slotResetSettingsToDefault()
{
    slotSettingsChanged(defaultSettings());
}

void BatchTool::slotSettingsChanged(const BatchToolSettings& settings)
{
    setSettings(settings);
    emit signalSettingsChanged(d->settings);
}

void BatchTool::setSettings(const BatchToolSettings& settings)
{
    d->settings = settings;
    ensureIsInitialized();
    emit signalAssignSettings2Widget();
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

void BatchTool::setNeedResetExifOrientation(bool set)
{
    d->exifResetOrientation = set;
}

bool BatchTool::getNeedResetExifOrientation() const
{
    return d->exifResetOrientation;
}

void BatchTool::setResetExifOrientationAllowed(bool set)
{
    d->exifCanEditOrientation = set;
}

bool BatchTool::getResetExifOrientationAllowed() const
{
    return d->exifCanEditOrientation;
}

void BatchTool::setBranchHistory(bool branch)
{
    d->branchHistory = branch;
}

bool BatchTool::getBranchHistory() const
{
    return d->branchHistory;
}

void BatchTool::setRawDecodingSettings(const DRawDecoding& settings)
{
    d->rawDecodingSettings = settings;
}

DRawDecoding BatchTool::getRawDecodingSettings() const
{
    return d->rawDecodingSettings;
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

bool BatchTool::isCancelled() const
{
    return d->cancel;
}

BatchToolSettings BatchTool::settings() const
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

    QString path(workingUrl().toLocalFile());
    QString suffix = outputSuffix();

    if (suffix.isEmpty())
    {
        QFileInfo fi(inputUrl().fileName());
        suffix = fi.completeSuffix();
    }

    SafeTemporaryFile temp(workingUrl().toLocalFile() + "/BatchTool-XXXXXX.digikamtempfile." + suffix);
    temp.setAutoRemove(false);
    temp.open();
    kDebug() << "path: " << temp.fileName();

    KUrl url;
    url.setPath(path);
    setOutputUrl(KUrl::fromPath(temp.fileName()));
}

bool BatchTool::loadToDImg() const
{
    if (!d->image.isNull())
    {
        return true;
    }

    return d->image.load(inputUrl().toLocalFile(), d->observer, getRawDecodingSettings());
}

bool BatchTool::savefromDImg() const
{
    if (!isLastChainedTool() && outputSuffix().isEmpty())
    {
        return true;
    }

    DImg::FORMAT detectedFormat = d->image.detectedFormat();
    QString frm                 = outputSuffix().toUpper();
    bool resetOrientation       = getResetExifOrientationAllowed() &&
                                  (getNeedResetExifOrientation() || detectedFormat == DImg::RAW);

    if (d->branchHistory)
    {
        // image has its original history
        d->image.setHistoryBranch();
    }

    if (frm.isEmpty())
    {
        // In case of output support is not set for ex. with all tool which do not convert to new format.
        d->image.prepareMetadataToSave(outputUrl().toLocalFile(), DImg::formatToMimeType(detectedFormat),
                                       resetOrientation);
        return(d->image.save(outputUrl().toLocalFile(), detectedFormat, d->observer));
    }

    d->image.prepareMetadataToSave(outputUrl().toLocalFile(), frm, resetOrientation);
    bool b   = d->image.save(outputUrl().toLocalFile(), frm, d->observer);
    d->image = DImg();
    return b;
}

DImg& BatchTool::image() const
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
        if (it.value().canConvert<QPolygon>())
        {
            QPolygon pol = it.value().value<QPolygon>();
            int size     = pol.size() > 20 ? 20 : pol.size();
            QString tmp;
            tmp.append(QString("[%1 items] : ").arg(pol.size()));

            for (int i = 0 ; i < size ; ++i)
            {
                tmp.append("(");
                tmp.append(QString::number(pol.point(i).x()));
                tmp.append(", ");
                tmp.append(QString::number(pol.point(i).y()));
                tmp.append(") ");
            }

            kDebug() << "   " << it.key() << ": " << tmp;
        }
        else
        {
            kDebug() << "   " << it.key() << ": " << it.value();
        }
    }

    return toolOperations();
}

void BatchTool::applyFilter(DImgThreadedFilter* filter)
{
    filter->startFilterDirectly();
    if (isCancelled())
    {
        return;
    }
    d->image.putImageData(filter->getTargetImage().bits());
    d->image.addFilterAction(filter->filterAction());
}

void BatchTool::applyFilterChangedProperties(DImgThreadedFilter* filter)
{
    filter->startFilterDirectly();
    if (isCancelled())
    {
        return;
    }
    DImg trg = filter->getTargetImage();
    d->image.putImageData(trg.width(), trg.height(), trg.sixteenBit(), trg.hasAlpha(), trg.bits());
    d->image.addFilterAction(filter->filterAction());
}

void BatchTool::applyFilter(DImgBuiltinFilter* filter)
{
    filter->apply(d->image);
    d->image.addFilterAction(filter->filterAction());
}

}  // namespace Digikam
