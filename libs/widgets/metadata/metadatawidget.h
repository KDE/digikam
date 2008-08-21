/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATAWIDGET_H
#define METADATAWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "dmetadata.h"
#include "digikam_export.h"

namespace Digikam
{

class MetadataListView;
class MetadataWidgetPriv;

class DIGIKAM_EXPORT MetadataWidget : public QWidget
{
    Q_OBJECT

public:

    enum Mode
    {
        SIMPLE=0,
        FULL
    };

public:

    MetadataWidget(QWidget* parent, const char* name=0);
    ~MetadataWidget();

    int     getMode();
    void    setMode(int mode);

    QString getCurrentItemKey() const;
    void    setCurrentItemByKey(const QString& itemKey);

    void    setUserAreaWidget(QWidget *w);

    virtual QString getTagTitle(const QString& key);
    virtual QString getTagDescription(const QString& key);

    virtual bool loadFromData(const QString &fileName, const QByteArray& data=QByteArray());
    virtual bool loadFromURL(const KURL& url)=0;

private slots:

    void slotModeChanged(int);
    void slotCopy2Clipboard();
    void slotPrintMetadata();
    
protected slots:    
    
    virtual void slotSaveMetadataToFile()=0;
    
protected:

    void   enabledToolButtons(bool);
    void   setFileName(const QString& fileName);
    MetadataListView* view();

    bool   setMetadata(const QByteArray& data=QByteArray());
    const  QByteArray& getMetadata();

    void   setMetadataMap(const DMetadata::MetaDataMap& data=DMetadata::MetaDataMap());
    const  DMetadata::MetaDataMap& getMetadataMap();

    void   setIfdList(const DMetadata::MetaDataMap &ifds, const QStringList& tagsFilter=QStringList());
    void   setIfdList(const DMetadata::MetaDataMap &ifds, const QStringList& keysFilter,
                      const QStringList& tagsFilter);

    KURL   saveMetadataToFile(const QString& caption, const QString& fileFilter);
    bool   storeMetadataToFile(const KURL& url);

    virtual void buildView();
    virtual bool decodeMetadata()=0;
    virtual QString getMetadataTitle()=0;
    virtual void setMetadataEmpty();

private:

    MetadataWidgetPriv* d;
};

}  // namespace Digikam

#endif /* METADATAWIDGET_H */
