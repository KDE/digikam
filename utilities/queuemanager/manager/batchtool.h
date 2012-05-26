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
#include "drawdecoding.h"

class QWidget;

namespace Digikam
{

class DImgBuiltinFilter;
class DImgThreadedFilter;

/** A map of batch tool settings (setting key, setting value).
 */
typedef QMap<QString, QVariant> BatchToolSettings;

class BatchTool : public QObject
{
    Q_OBJECT

public:

    enum BatchToolGroup
    {
        BaseTool = 0,             // digiKam core tools.
        KipiTool,                 // Exported kipi-plugins tools.
        CustomTool,               // List of tools grouped and customized by users.

        ColorTool,                // Tools to manage image colors (Curves, BCG, etc...)
        EnhanceTool,              // Tools to ehance images (NR, sharp, etc...)
        TransformTool,            // Tools to transform images geometry (resize, rotate, flip, etc...)
        DecorateTool,             // Tools to decorate images (Border, watermark, etc...)
        FiltersTool,              // Tools to apply filters and special effects (film grain, BlurFx, etc...)
        ConvertTool,              // Tools to convert images format (PNG, JPEG, TIFF, etc...)
        MetadataTool              // Tools to play with metadata.
    };

public:

    BatchTool(const QString& name, BatchToolGroup group, QObject* const parent = 0);
    ~BatchTool();

    /** Get description of an error which appear during apply() method.
     */
    QString errorDescription() const;

    /** Return group of tool. See BatchToolGroup enum for details.
     */
    BatchToolGroup toolGroup() const;

    /** Manage Tool title on settings view.
     */
    void setToolTitle(const QString& toolTitle);
    QString toolTitle() const;

    /** Manage Tool description on settings view.
     */
    void setToolDescription(const QString& toolDescription);
    QString toolDescription() const;

    /** Manage Tool icon on settings view.
     */
    void setToolIcon(const KIcon& toolIcon);
    KIcon toolIcon() const;

    /** Assign no settings view to tool. A label is just displayed.
     */
    void setNoSettingsWidget();

    /** Manage customized settings widget on settings view.
     */
    void setSettingsWidget(QWidget* const settingsWidget);
    QWidget* settingsWidget() const;

    /** Manage settings values to tool. See BatchToolSettings container for details.
     */
    void setSettings(const BatchToolSettings& settings);
    BatchToolSettings settings() const;

    /** Manage current input url processed by this tool.
     */
    void setInputUrl(const KUrl& inputUrl);
    KUrl inputUrl() const;

    /** Manage current output url processed by this tool.
     */
    void setOutputUrl(const KUrl& outputUrl);
    KUrl outputUrl() const;

    /** Manage current working url used by this tool to process items.
     */
    void setWorkingUrl(const KUrl& workingUrl);
    KUrl workingUrl() const;

    /** Manage instance of current image data container loaded by this tool.
     */
    void setImageData(const DImg& img);
    DImg imageData() const;

    /** Manage flag properties to indicate if this tool is last one to process on current item.
     */
    void setLastChainedTool(bool last);
    bool isLastChainedTool() const;

    /** Set output url using input url content + annotation based on time stamp + file
        extension defined by outputSuffix().
        if outputSuffix() return null, file extension is the same than original.
     */
    void setOutputUrlFromInputUrl();

    /** Load image data using input Url set by setInputUrl() to instance of internal
        DImg container.
     */
    bool loadToDImg() const;

    /** Save image data from instance of internal DImg container using :
        - output Url set by setOutputUrl() or setOutputUrlFromInputUrl()
        - output file format set by outputSuffix(). If this one is empty,
          format of original image is used instead.
     */
    bool savefromDImg() const;

    /** Set that the Exif orientation flag is allowed be reset to NORMAL after tool operation
     */
    void setResetExifOrientationAllowed(bool reset);

    /** Returns true if the Exif orientation tag is allowed to be reset after tool operation
     */
    bool getResetExifOrientationAllowed() const;

    /** Set that the Exif orientation flag should be reset to NORMAL after tool operation
     */
    void setNeedResetExifOrientation(bool reset);

    /** Returns true if the Exif orientation tag should be reset after tool operation
     */
    bool getNeedResetExifOrientation() const;

    /**
     * Sets if the history added by tools shall be made a branch (new version).
     * Applies only when the file is actually saved on disk, and takes the history
     * since the loading from disk to set the first added step as creating a branch.
     */
    void setBranchHistory(bool branch = true);
    bool getBranchHistory() const;

    /** Set-up RAW decoding settings no use during tool operations.
     */
    void setRawDecodingSettings(const DRawDecoding& settings);

    /** Return RAW decoding settings used during tool operations.
     */
    DRawDecoding getRawDecodingSettings() const;

    /** Apply all change to perform by this tool. This method call customized toolOperations().
     */
    bool apply();

    /** Re-implement this method is you want customize cancelization of tool, for ex. to call
        a dedicated method to kill sub-threads parented to this tool instance.
        Unforget to call parent BatchTool::cancel() method in you customized implementation.
     */
    virtual void cancel();

    /** Re-implemnt this method if tool change file extension during batch process (ex: "png").
        Typicaly, this is used with tool which convert to new file format.
        This method return and empty string by default.
     */
    virtual QString outputSuffix() const;

    /** Re-implement this method to initialize Settings Widget value with default settings.
     */
    virtual BatchToolSettings defaultSettings() = 0;

    /** For delayed creation: Ensure that createSettingsWidget() has been called.
     */
    void ensureIsInitialized() const;

Q_SIGNALS:

    void signalSettingsChanged(const BatchToolSettings&);
    void signalAssignSettings2Widget();

public Q_SLOTS:

    void slotResetSettingsToDefault();
    void slotSettingsChanged(const BatchToolSettings& settings);

protected:

    /** Set string to describe an error which appear during apply() method.
     */
    void setErrorDescription(const QString& errmsg);

    /** Return a reference of internal DImg container used to modify image data.
     */
    DImg& image() const;

    /** Return true if cancel() have been called. Use this method to stop loop in your toolOperations() implementation.
     */
    bool isCancelled() const;

    /**
     * Use this if you have a filter ready to run.
     * Will call startFilterDirectly and apply the result to image().
     */
    void applyFilter(DImgThreadedFilter* filter);
    void applyFilterChangedProperties(DImgThreadedFilter* filter);
    void applyFilter(DImgBuiltinFilter* filter);

    /** Re-implement this method to customize all batch operations done by this tool.
        This method is called by apply().
     */
    virtual bool toolOperations() = 0;

    /** For delayed creation of a settings widget:
     *  If your tool's settings widget takes long to create, you can
     *  avoid creating it in your constructor. If you call neither
     *  setSettingsWidget() nor setNoSettingsWidget() from your constructor,
     *  this method will be invoked later when the settings widget is requested.
     */
    virtual QWidget* createSettingsWidget();

protected Q_SLOTS:

    virtual void slotSettingsChanged() = 0;

    /** Re-implement this method to customize how all settings values must be assigned to settings widget.
        This method is called by setSettings().
     */
    virtual void slotAssignSettings2Widget() = 0;

public:

    // Declared as public due to BatchToolObserver class.
    class BatchToolPriv;

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

    BatchTool*        tool;
    BatchToolSettings settings;
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

    AssignedBatchTools() {};

    QString targetSuffix(bool* const extSet = 0) const
    {
        QString suffix;
        foreach(BatchToolSet set, m_toolsMap)
        {
            QString s = set.tool->outputSuffix();

            if (!s.isEmpty())
            {
                suffix = s;

                if (extSet != 0)
                {
                    *extSet = true;
                }
            }
        }

        if (suffix.isEmpty())
        {
            if (extSet != 0)
            {
                *extSet = false;
            }

            return (QFileInfo(m_itemUrl.fileName()).suffix());
        }

        return suffix;
    }

public:

    KUrl         m_itemUrl;
    BatchToolMap m_toolsMap;
};

}  // namespace Digikam

#endif /* BATCHTOOL_H */
