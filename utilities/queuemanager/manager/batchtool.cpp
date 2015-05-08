/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tool Container.
 *
 * Copyright (C) 2008-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QPolygon>
#include <QTemporaryFile>
#include <QWidget>
#include <QLabel>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

// Local includes

#include "dimgbuiltinfilter.h"
#include "dimgloaderobserver.h"
#include "dimgthreadedfilter.h"
#include "filereadwritelock.h"
#include "batchtoolutils.h"
#include "jpegsettings.h"
#include "pngsettings.h"

using namespace KDcrawIface;

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
        observer(0),
        toolGroup(BaseTool),
        rawLoadingRule(QueueSettings::DEMOSAICING)
    {
    }

    bool                          exifResetOrientation;
    bool                          exifCanEditOrientation;
    bool                          branchHistory;
    bool                          cancel;
    bool                          last;

    QString                       errorMessage;
    QString                       toolTitle;          // User friendly tool title.
    QString                       toolDescription;    // User friendly tool description.
    QString                       toolIconName;

    KUrl                          inputUrl;
    KUrl                          outputUrl;
    KUrl                          workingUrl;

    DImg                          image;

    RawDecodingSettings           rawDecodingSettings;

    IOFileSettings                ioFileSettings;

    BatchToolSettings             settings;

    BatchToolObserver*            observer;

    BatchTool::BatchToolGroup     toolGroup;

    QueueSettings::RawLoadingRule rawLoadingRule;
};

class BatchToolObserver : public DImgLoaderObserver
{

public:

    explicit BatchToolObserver(BatchTool::Private* const priv)
        : DImgLoaderObserver(), d(priv)
    {
    }

    ~BatchToolObserver()
    {
    }

    bool continueQuery(const DImg* const)
    {
        return (!d->cancel);
    }

private:

    BatchTool::Private* const d;
};

BatchTool::BatchTool(const QString& name, BatchToolGroup group, QObject* const parent)
    : QObject(parent), d(new Private)
{
    d->observer      = new BatchToolObserver(d);
    d->toolGroup     = group;
    m_settingsWidget = 0;
    setObjectName(name);
}

BatchTool::~BatchTool()
{
    // NOTE: See bug #341566: no need to delete settings widget here.
    // Owner is passed to ToolSettingsView, which will delete instance,
    // even if Valgrind report a memory leak.

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

void BatchTool::setToolIconName(const QString& iconName)
{
    d->toolIconName = iconName;
}

QString BatchTool::toolIconName() const
{
    return d->toolIconName;
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

void BatchTool::setRawLoadingRules(QueueSettings::RawLoadingRule rule)
{
    d->rawLoadingRule = rule;
}

void BatchTool::setBranchHistory(bool branch)
{
    d->branchHistory = branch;
}

bool BatchTool::getBranchHistory() const
{
    return d->branchHistory;
}

void BatchTool::setRawDecodingSettings(const RawDecodingSettings& settings)
{
    d->rawDecodingSettings = settings;
}

RawDecodingSettings BatchTool::rawDecodingSettings() const
{
    return d->rawDecodingSettings;
}

void BatchTool::setIOFileSettings(const IOFileSettings& settings)
{
    d->ioFileSettings = settings;
}

IOFileSettings BatchTool::ioFileSettings() const
{
    return d->ioFileSettings;
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

bool BatchTool::isRawFile(const KUrl& url) const
{
    QString   rawFilesExt(KDcraw::rawFiles());
    QFileInfo fileInfo(url.toLocalFile());
    return (rawFilesExt.toUpper().contains(fileInfo.suffix().toUpper()));
}

bool BatchTool::loadToDImg() const
{
    if (!d->image.isNull())
    {
        return true;
    }

    if (d->rawLoadingRule == QueueSettings::USEEMBEDEDJPEG && isRawFile(inputUrl()))
    {
        QImage img;
        bool   ret = KDcraw::loadRawPreview(img, inputUrl().toLocalFile());
        d->image   = DImg(img);
        return ret;
    }

    return (d->image.load(inputUrl().toLocalFile(), d->observer, DRawDecoding(rawDecodingSettings())));
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
        if (detectedFormat == DImg::JPEG)
        {
            d->image.setAttribute("quality",     JPEGSettings::convertCompressionForLibJpeg(ioFileSettings().JPEGCompression));
            d->image.setAttribute("subsampling", ioFileSettings().JPEGSubSampling);
        }
        else if (detectedFormat == DImg::PNG)
        {
            d->image.setAttribute("quality",     PNGSettings::convertCompressionForLibPng(ioFileSettings().PNGCompression));
        }
        else if (detectedFormat == DImg::TIFF)
        {
            d->image.setAttribute("compress",    ioFileSettings().TIFFCompression);
        }
        else if (detectedFormat == DImg::JP2K)
        {
            d->image.setAttribute("quality",     ioFileSettings().JPEG2000LossLess ? 100 :
                                                 ioFileSettings().JPEG2000Compression);
        }
        else if (detectedFormat == DImg::PGF)
        {
            d->image.setAttribute("quality",     ioFileSettings().PGFLossLess ? 0 :
                                                 ioFileSettings().PGFCompression);
        }

        d->image.prepareMetadataToSave(outputUrl().toLocalFile(), DImg::formatToMimeType(detectedFormat), resetOrientation);
        bool b = d->image.save(outputUrl().toLocalFile(), detectedFormat, d->observer);
        return b;
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
    //kDebug() << "Settings:   ";

    BatchToolSettings prm = settings();

    for (BatchToolSettings::const_iterator it = prm.constBegin() ; it != prm.constEnd() ; ++it)
    {
        if (it.value().canConvert<QPolygon>())
        {
            QPolygon pol = it.value().value<QPolygon>();
            int     size = (pol.size() > 20) ? 20 : pol.size();
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

            //kDebug() << "   " << it.key() << ": " << tmp;
        }
        else
        {
            //kDebug() << "   " << it.key() << ": " << it.value();
        }
    }

    return toolOperations();
}

void BatchTool::applyFilter(DImgThreadedFilter* const filter)
{
    filter->startFilterDirectly();

    if (isCancelled())
    {
        return;
    }

    d->image.putImageData(filter->getTargetImage().bits());
    d->image.addFilterAction(filter->filterAction());
}

void BatchTool::applyFilterChangedProperties(DImgThreadedFilter* const filter)
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

void BatchTool::applyFilter(DImgBuiltinFilter* const filter)
{
    filter->apply(d->image);
    d->image.addFilterAction(filter->filterAction());
}

// -- Settings Widgets methods ---------------------------------------------------------------------------

QWidget* BatchTool::settingsWidget() const
{
    return m_settingsWidget;
}

void BatchTool::registerSettingsWidget()
{
    // NOTE: see bug #209225 : signal/slot connection used internally to prevent crash when settings
    // are assigned to settings widget by main thread to tool thread.

    connect(this, SIGNAL(signalAssignSettings2Widget()),
            this, SLOT(slotAssignSettings2Widget()));

    if (!m_settingsWidget)
    {
        QLabel* const label = new QLabel;
        label->setText(i18n("No setting available"));
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        m_settingsWidget = label;
    }
}

}  // namespace Digikam
