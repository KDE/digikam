/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-20
 * Description : a widget to display Standard Exif metadata
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

#ifndef EXIFWIDGET_H
#define EXIFWIDGET_H

// Qt includes.

#include <qstring.h>

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExifWidget : public MetadataWidget
{
    Q_OBJECT
    
public:

    ExifWidget(QWidget* parent, const char* name=0);
    ~ExifWidget();

    bool loadFromURL(const KURL& url);
    
    QString getTagDescription(const QString& key);
    QString getTagTitle(const QString& key);

    QString getMetadataTitle(void);

protected slots:    
    
    virtual void slotSaveMetadataToFile(void);
 
private:

    bool decodeMetadata(void);
    void buildView(void);
    
private:

    QStringList m_tagsfilter;
    QStringList m_keysFilter;
};

}  // namespace Digikam

#endif /* EXIFWIDGET_H */
