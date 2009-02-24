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

// Qt includes.

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>
#include <QList>

// KDE includes.

#include <kurl.h>
#include <kicon.h>

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

    /** Set output url using input url content + annotation based on time stamp + file 
        extension defined by outputSuffix().
        if outputSuffix() return null, file extension is the same than original.
     */
    void setOutputUrlFromInputUrl();

    void setExifSetOrientation(bool set);
    bool getExifSetOrientation() const;

    bool apply();

    /** Re-implement this method is you want customize cancelization of tool, for ex. to call 
        a dedicated method to kill sub-threads parented to this tool instance.
        Unforget to call parent BatchTool::cancel() method in you customized implementation.
     */
    virtual void cancel();

    /** Re-implemnt this method if tool change file extension during batch process (ex: "png").
        Typicaly, this is used with tool which convert to new file format.
        This method return and empty string by default
     */
    virtual QString outputSuffix() const;

    /** Re-implement this method to initialize Settings Widget value with default settings.
     */
    virtual BatchToolSettings defaultSettings() = 0;

signals:

    void signalSettingsChanged(const BatchToolSettings&);

protected:

    /** Return true if cancel() have been called. Use this method to stop loop in your toolOperations() implementation.
     */
    bool isCancelled();

    /** Re-implement this method to customize how all settings values must be assigned to settings widget.
        This method is called by setSettings().
     */
    virtual void assignSettings2Widget()=0;

    /** re-implement this method to customize th batch operations done by tool
     */
    virtual bool toolOperations()=0;

protected slots:

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

    KUrl         itemUrl;
    BatchToolMap toolsMap;
};

}  // namespace Digikam

#endif /* BATCHTOOL_H */
