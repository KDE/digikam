/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_META_DATA_WIDGET_H
#define DIGIKAM_META_DATA_WIDGET_H

// Qt includes

#include <QWidget>
#include <QString>
#include <QUrl>

// Local includes

#include "dmetadata.h"
#include "digikam_export.h"

namespace Digikam
{

class MetadataListView;

class DIGIKAM_EXPORT MetadataWidget : public QWidget
{
    Q_OBJECT

public:

    enum TagFilters
    {
        NONE = 0,
        PHOTO,
        CUSTOM
    };

public:

    explicit MetadataWidget(QWidget* const parent, const QString& name=QString());
    ~MetadataWidget();

    int     getMode() const;
    void    setMode(int mode);

    QStringList getTagsFilter() const;
    void        setTagsFilter(const QStringList& list);

    QString getCurrentItemKey() const;
    void    setCurrentItemByKey(const QString& itemKey);

    void    setUserAreaWidget(QWidget* const w);

    virtual QString getTagTitle(const QString& key);
    virtual QString getTagDescription(const QString& key);

    virtual bool loadFromData(const QString& fileName, const DMetadata& data=DMetadata());
    virtual bool loadFromURL(const QUrl& url)=0;

Q_SIGNALS:

    void signalSetupMetadataFilters();

protected Q_SLOTS:

    virtual void slotSaveMetadataToFile()=0;

protected:

    void   enabledToolButtons(bool);
    void   setFileName(const QString& fileName);
    MetadataListView* view() const;

    bool   setMetadata(const DMetadata& data=DMetadata());
    const  DMetadata& getMetadata();

    void   setMetadataMap(const DMetadata::MetaDataMap& data=DMetadata::MetaDataMap());
    const  DMetadata::MetaDataMap& getMetadataMap();

    void   setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& tagsFilter=QStringList());
    void   setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& keysFilter,
                      const QStringList& tagsFilter);

    QUrl   saveMetadataToFile(const QString& caption, const QString& fileFilter);
    bool   storeMetadataToFile(const QUrl& url, const QByteArray& metaData);

    virtual void buildView();
    virtual void setMetadataEmpty();

    virtual bool    decodeMetadata()=0;
    virtual QString getMetadataTitle()=0;

private Q_SLOTS :

    void slotCopy2Clipboard();
    void slotPrintMetadata();
    void slotFilterChanged(QAction*);

protected:

    /**
     * Call this method in children class contructors to init signal/slots connections.
     */
    void setup();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_META_DATA_WIDGET_H
