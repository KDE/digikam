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

#ifndef BATCHTOOL_H
#define BATCHTOOL_H

// Qt includes

#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>
#include <QList>

// KDE includes

#include <kurl.h>
#include <kicon.h>

// Local includes

#include "dimg.h"
#include "parser.h"

class QWidget;

namespace Digikam
{

/** A map of batch tool settings (setting key, setting value).
 */
typedef QMap<QString, QVariant> BatchToolSettings;

class BatchToolPriv;

class BatchTool : public QObject
{
    Q_OBJECT

public:

    enum BatchToolGroup
    {
        BaseTool   = 0,
        KipiTool   = 1,
        CustomTool = 2
    };

public:

    BatchTool(const QString& name, BatchToolGroup group, QObject* parent=0);
    ~BatchTool();

    BatchToolGroup toolGroup() const;

    void setToolTitle(const QString& toolTitle);
    QString toolTitle() const;

    void setToolDescription(const QString& toolDescription);
    QString toolDescription() const;

    void setToolIcon(const KIcon& toolIcon);
    KIcon toolIcon() const;

    void setSettingsWidget(QWidget* settingsWidget);
    QWidget* settingsWidget() const;

    void setSettings(const BatchToolSettings& settings);
    BatchToolSettings settings();

    void setInputUrl(const KUrl& inputUrl);
    KUrl inputUrl() const;

    void setOutputUrl(const KUrl& outputUrl);
    KUrl outputUrl() const;

    void setWorkingUrl(const KUrl& workingUrl);
    KUrl workingUrl() const;

    void setImageData(const DImg& img);
    DImg imageData() const;

    void setLastChainedTool(bool last);
    bool isLastChainedTool() const;

    ManualRename::ParseInformation parseInformation() const;
    void setParseInformation(ManualRename::ParseInformation& info);

    /** Set output url using input url content + annotation based on time stamp + file
        extension defined by outputSuffix().
        if outputSuffix() return null, file extension is the same than original.
     */
    void setOutputUrlFromInputUrl();

    /** Load image data using input Url set by setInputUrl() to instance of internal
        DImg container.
     */
    bool loadToDImg();

    /** Save image data from instance of internal DImg container using :
        - output Url set by setOutputUrl() or setOutputUrlFromInputUrl()
        - output file format set by outputSuffix(). If this one is empty,
          format of original image is used instead.
     */
    bool savefromDImg();

    /** Set-up Exif orientation tag needs to be fixed during tool operations.
     */
    void setExifSetOrientation(bool set);

    /** Return true if Exif orientation tag will be fixed during tool operations.
     */
    bool getExifSetOrientation() const;

    /** Apply all change to perform by this tool. This method call customized toolOperations().
     */
    bool apply();

    /** Re-implement this method is you want customize cancelization of tool, for ex. to call
        a dedicated method to kill sub-threads parented to this tool instance.
        Unforget to call parent BatchTool::cancel() method in you customized implementation.
     */
    virtual void cancel();

    /** Re-implement this method if tool change file extension during batch process (ex: "png").
        Typically, this is used with tool which convert to new file format.
        This method return and empty string by default.
     */
    virtual QString outputSuffix() const;

    /** Re-implement this method if tool change file base name during batch process.
        Typically, this is used with tool which convert to a new file name.
        This method return and empty string by default.
     */
    virtual QString outputBaseName() const;

    /** Re-implement this method to initialize Settings Widget value with default settings.
     */
    virtual BatchToolSettings defaultSettings() = 0;

Q_SIGNALS:

    void signalSettingsChanged(const BatchToolSettings&);

public Q_SLOTS:

    void slotResetSettingsToDefault();

protected:

    /** Return a reference of internal DImg container used to modify image data.
     */
    DImg& image();

    /** Return true if cancel() have been called. Use this method to stop loop in your toolOperations() implementation.
     */
    bool isCancelled();

    /** Re-implement this method to customize how all settings values must be assigned to settings widget.
        This method is called by setSettings().
     */
    virtual void assignSettings2Widget()=0;

    /** Re-implement this method to customize all batch operations done by this tool.
        This method is called by apply().
     */
    virtual bool toolOperations()=0;

protected Q_SLOTS:

    virtual void slotSettingsChanged()=0;

private:

    BatchToolPriv* const d;
};

/** A list of batch tool instances.
 */
typedef QList<BatchTool*> BatchToolsList;

/** A container of associated batch tool and settings.
 */
class BatchToolSet
{
public:

    BatchToolSet()
    {
        tool = 0;
    };

    BatchTool         *tool;
    BatchToolSettings  settings;
};

/** An indexed map of batch tools with settings.
 */
typedef QMap<int, BatchToolSet> BatchToolMap;

/** Container to assign Batch tools and settings to an item by Url.
    Url is used only with ActionThread class.
 */
class AssignedBatchTools
{
public:

    AssignedBatchTools(){};

    QString targetBaseName()
    {
        QString baseName;
        foreach(BatchToolSet set, toolsMap)
        {
            set.tool->setParseInformation(parseInfo);
            QString s = set.tool->outputBaseName();
            if (!s.isEmpty())
                baseName = s;
        }

        if (baseName.isEmpty())
            return (QFileInfo(itemUrl.fileName()).baseName());

        return baseName;
    }

    QString targetSuffix()
    {
        QString suffix;
        foreach(BatchToolSet set, toolsMap)
        {
            QString s = set.tool->outputSuffix();
            if (!s.isEmpty())
                suffix = s;
        }

        if (suffix.isEmpty())
            return (QFileInfo(itemUrl.fileName()).suffix());

        return suffix;
    }

    KUrl                           itemUrl;
    BatchToolMap                   toolsMap;
    ManualRename::ParseInformation parseInfo;
};

}  // namespace Digikam

#endif /* BATCHTOOL_H */
