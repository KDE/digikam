/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-22
 * Description : a generic widget to display metadata
 * 
 * Copyright 2006 by Gilles Caulier
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

// Local includes

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
        SIMPLE,
        FULL
    };

    typedef QMap<QString, QString>  MetaDataMap;

public:

    MetadataWidget(QWidget* parent, const char* name=0);
    ~MetadataWidget();

    int     getMode(void);
    void    setMode(int mode);
    
    QString getCurrentItemKey() const;
    void    setCurrentItemByKey(const QString& itemKey);

    void    setUserAreaWidget(QWidget *w);

    virtual QString getTagTitle(const QString& key);
    virtual QString getTagDescription(const QString& key);
    
    virtual bool loadFromData(QString fileName, const QByteArray& data=QByteArray());
    virtual bool loadFromURL(const KURL& url)=0;

private slots:

    void slotModeChanged(int);
    void slotCopy2Clipboard(void);
    void slotPrintMetadata(void);

protected:

    void   setFileName(QString fileName);
    MetadataListView* view(void);

    bool   setMetadata(const QByteArray& data=QByteArray());
    const  QByteArray& getMetadata();

    void   setMetadataMap(const MetaDataMap& data=MetaDataMap());
    const  MetadataWidget::MetaDataMap& getMetadataMap();

    void   setIfdList(const MetaDataMap &ifds, const QStringList& tagsFilter=QStringList());
    void   setIfdList(const MetaDataMap &ifds, const QStringList& keysFilter,
                      const QStringList& tagsFilter);
        
    virtual bool decodeMetadata(void)=0;
    virtual void buildView(void)=0;
    virtual QString getMetadataTitle(void)=0;

private:

    MetadataWidgetPriv* d;    
};

}  // namespace Digikam

#endif /* METADATAWIDGET_H */
