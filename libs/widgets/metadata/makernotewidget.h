/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display non standard Exif metadata
 *               used by camera makers
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

#ifndef MARKERNOTEWIDGET_H
#define MARKERNOTEWIDGET_H

// Local includes.

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MakerNoteWidget : public MetadataWidget
{
    Q_OBJECT
    
public:

    MakerNoteWidget(QWidget* parent, const char* name=0);
    ~MakerNoteWidget();

    bool loadFromURL(const KURL& url);
    
    QString getTagDescription(const QString& key);
    QString getTagTitle(const QString& key);

    QString getMetadataTitle();

protected slots:    
    
    virtual void slotSaveMetadataToFile();

private:

    bool decodeMetadata();
    void buildView();

private:

    QStringList m_tagsfilter;
    QStringList m_keysFilter;    
};

}  // namespace Digikam

#endif /* MARKERNOTEWIDGET_H */
