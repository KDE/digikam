/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-06-23
 * Description : a tab widget to display ICC profile infos
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

#ifndef ICCPROFILEWIDGET_H
#define ICCPROFILEWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class ICCProfileWidgetPriv;

class DIGIKAM_EXPORT ICCProfileWidget : public MetadataWidget
{
    Q_OBJECT
    
public:

    ICCProfileWidget(QWidget* parent, const char* name=0, int w=256, int h=256);
    ~ICCProfileWidget();

    bool    loadFromURL(const KURL& url);
    
    QString getTagDescription(const QString& key);
    QString getTagTitle(const QString& key);

    QString getMetadataTitle(void);

    void    setLoadingComplete(bool b);
    void    setDataLoading();

protected slots:    
    
    virtual void slotSaveMetadataToFile(void);
    
private:

    bool decodeMetadata(void);
    void buildView(void);
    
private:

    ICCProfileWidgetPriv *d;

};

}  // namespace Digikam

#endif /* ICCPROFILEWIDGET_H */
