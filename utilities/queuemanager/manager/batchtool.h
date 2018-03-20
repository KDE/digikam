/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tool Container.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BATCH_TOOL_H
#define BATCH_TOOL_H

// Qt includes

#include <QObject>

// Local includes

#include "drawdecodersettings.h"
#include "dimg.h"
#include "imageinfo.h"
#include "queuesettings.h"
#include "iofilesettings.h"

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
        CustomTool,               // List of tools grouped and customized by users.

        ColorTool,                // Tools to manage image colors (Curves, BCG, etc...)
        EnhanceTool,              // Tools to enhance images (NR, sharp, etc...)
        TransformTool,            // Tools to transform images geometry (resize, rotate, flip, etc...)
        DecorateTool,             // Tools to decorate images (Border, watermark, etc...)
        FiltersTool,              // Tools to apply filters and special effects (film grain, BlurFx, etc...)
        ConvertTool,              // Tools to convert images format (PNG, JPEG, TIFF, etc...)
        MetadataTool              // Tools to play with metadata.
    };

/// Tool data and properties management. NOTE: these methods can be used safely in multi-threading part (ActionThread).

public:

    explicit BatchTool(const QString& name, BatchToolGroup group, QObject* const parent = 0);
    ~BatchTool();

    /** Get description of an error which appear during apply() method.
     */
    QString errorDescription() const;

    /** Return group of tool. See BatchToolGroup enum for details.
     */
    BatchToolGroup toolGroup() const;

    /** Manage Tool title.
     */
    void setToolTitle(const QString& toolTitle);
    QString toolTitle() const;

    /** Manage Tool description.
     */
    void setToolDescription(const QString& toolDescription);
    QString toolDescription() const;

    /** Manage Tool icon name.
     */
    void setToolIconName(const QString& iconName);
    QString toolIconName() const;

    /** Manage settings values to tool. See BatchToolSettings container for details.
     */
    void setSettings(const BatchToolSettings& settings);
    BatchToolSettings settings() const;

    /** Manage current input url processed by this tool.
     */
    void setInputUrl(const QUrl& inputUrl);
    QUrl inputUrl() const;

    /** Manage current output url processed by this tool.
     */
    void setOutputUrl(const QUrl& outputUrl);
    QUrl outputUrl() const;

    /** Manage current working url used by this tool to process items.
     */
    void setWorkingUrl(const QUrl& workingUrl);
    QUrl workingUrl() const;

    /** Manage instance of current image data container loaded by this tool.
     */
    void setImageData(const DImg& img);
    DImg imageData() const;

    /** Manage instance of current image info loaded by this tool.
     */
    void setImageInfo(const ImageInfo& info);
    ImageInfo imageInfo() const;

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

    /** Set that RAW files loading rule to use (demosaicing or JPEG embedded).
     */
    void setRawLoadingRules(QueueSettings::RawLoadingRule rule);

    /**
     * Sets if the history added by tools shall be made a branch (new version).
     * Applies only when the file is actually saved on disk, and takes the history
     * since the loading from disk to set the first added step as creating a branch.
     */
    void setBranchHistory(bool branch = true);
    bool getBranchHistory() const;

    /** Set-up RAW decoding settings no use during tool operations.
     */
    void setDRawDecoderSettings(const DRawDecoderSettings& settings);

    /** Return RAW decoding settings used during tool operations.
     */
    DRawDecoderSettings rawDecodingSettings() const;

    /** Set-up IOFile settings no use during tool operations.
     */
    void setIOFileSettings(const IOFileSettings& settings);

    /** Return IOFile settings used during tool operations.
     */
    IOFileSettings ioFileSettings() const;

    /** Apply all change to perform by this tool. This method call customized toolOperations().
     */
    bool apply();

    /** Return version of tool. By default, ID is 1. Re-implement this method and increase this ID when tool settings change.
     */
    virtual int toolVersion() const { return 1; };

    /** Re-implement this method is you want customize cancellation of tool, for ex. to call
        a dedicated method to kill sub-threads parented to this tool instance.
        Unforget to call parent BatchTool::cancel() method in your customized implementation.
     */
    virtual void cancel();

    /** Re-implement this method if tool change file extension during batch process (ex: "png").
        Typically, this is used with tool which convert to new file format.
        This method return and empty string by default.
     */
    virtual QString outputSuffix() const;

    /** Re-implement this method to initialize Settings Widget value with default settings.
     */
    virtual BatchToolSettings defaultSettings() = 0;

    /** Clone this tool without to create settings widget.
     *  It's a safe construction of tools instance used in multithreading (ActionThread) to process items in parallel.
     */
    virtual BatchTool* clone(QObject* const parent=0) const = 0;

Q_SIGNALS:

    void signalSettingsChanged(const BatchToolSettings&);

public Q_SLOTS:

    void slotResetSettingsToDefault();
    void slotSettingsChanged(const BatchToolSettings& settings);

protected:

    /** Method to check if file pointed by url is a RAW image
     */
    bool isRawFile(const QUrl& url) const;

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
    void applyFilter(DImgThreadedFilter* const filter);
    void applyFilterChangedProperties(DImgThreadedFilter* const filter);
    void applyFilter(DImgBuiltinFilter* const filter);

    /** Re-implement this method to customize all batch operations done by this tool.
        This method is called by apply().
     */
    virtual bool toolOperations() = 0;

protected Q_SLOTS:

    virtual void slotSettingsChanged() = 0;

/// Settings widget management. NOTE: do not use these methods in multi-threading part (ActionThread), only in main thread (GUI)

public:

    /** Return dedicated settings widget registered with registerSettingsWidget().
     */
    QWidget* settingsWidget() const;

    /** Setup dedicated settings widget. Default implementation assign no settings view (a message label is just displayed).
     *  You need to call default implementation in your child class to init default signals and slots connections,
     *  after to have instanced your dedicated settings widget.
     */
    virtual void registerSettingsWidget();

Q_SIGNALS:

    /** Only used internally. See registerSettingsWidget() implementation.
     */
    void signalAssignSettings2Widget();

protected:

    /** Host settings widget instance.
     */
    QWidget* m_settingsWidget;

protected Q_SLOTS:

    /** Re-implement this method to customize how all settings values must be assigned to settings widget.
        This method is called by setSettings() through signalAssignSettings2Widget().
     */
    virtual void slotAssignSettings2Widget() = 0;

/// Private section

public:

    // Declared as public due to BatchToolObserver class.
    class Private;

private:

    Private* const d;
};

} // namespace Digikam

#endif // BATCH_TOOL_H
