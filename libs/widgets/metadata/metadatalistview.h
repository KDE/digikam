/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-21
 * Description : a generic list view widget to 
 *               display metadata
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

#ifndef METADATALISTVIEW_H
#define METADATALISTVIEW_H

// Qt includes.

#include <qstring.h>
#include <qptrlist.h>
#include <qmap.h>

// KDE includes.

#include <klistview.h>

// Local includes.

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetadataListView : public KListView
{
    Q_OBJECT

public:

    MetadataListView(QWidget* parent);
    ~MetadataListView();

    QString getCurrentItemKey();
    void    setCurrentItemByKey(QString itemKey);

    void    setIfdList(MetadataWidget::MetaDataMap ifds, const QStringList& tagsfilter);
    void    setIfdList(MetadataWidget::MetaDataMap ifds, QStringList keysFilter,
                       const QStringList& tagsFilter);

protected:

    void viewportResizeEvent(QResizeEvent*);

private slots:

    void slotSelectionChanged(QListViewItem *item);

private:

    MetadataWidget *m_parent;

    QString         m_selectedItemKey;    
};

}  // namespace Digikam

#endif // METADATALISTVIEW_H 
