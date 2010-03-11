/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-13
 * Description : a list of selectable options with preview
 *               effects as thumbnails.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PREVIEWLIST_H
#define PREVIEWLIST_H

// Qt includes

#include <QtCore/QObject>
#include <QtGui/QPixmap>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

// Local includes

#include "digikam_export.h"

namespace Digikam
{
  
class DImgThreadedFilter;
class PreviewThreadWrapperPriv;

class DIGIKAM_EXPORT PreviewThreadWrapper : public QObject
{
    Q_OBJECT

public:

    PreviewThreadWrapper(QObject* parent=0);
    ~PreviewThreadWrapper();
    
    void registerFilter(int id, DImgThreadedFilter* filter);
    
    void startFilters();
    void stopFilters();

Q_SIGNALS:

    void signalFilterStarted(int);
    void signalFilterFinished(int, const QPixmap&);
    
private Q_SLOTS:

    void slotFilterStarted();
    void slotFilterFinished(bool success);
    void slotFilterProgress(int progress);

private:

    PreviewThreadWrapperPriv* const d;  
};  

// -------------------------------------------------------------------

class PreviewListItemPriv;

class DIGIKAM_EXPORT PreviewListItem : public QTreeWidgetItem
{

public:

    PreviewListItem(QTreeWidget* parent=0);
    ~PreviewListItem();

    void setPixmap(const QPixmap& pix);

    void setId(int id);
    int  id();

    void setBusy(bool b);
    bool isBusy();

private:

    PreviewListItemPriv* const d;
};

// -------------------------------------------------------------------

class PreviewListPriv;

class DIGIKAM_EXPORT PreviewList : public QTreeWidget
{
    Q_OBJECT

public:

    PreviewList(QObject* parent = 0);
    ~PreviewList();

    PreviewListItem* addItem(DImgThreadedFilter* filter, const QString& txt, int id);

    void setCurrentId(int id);
    int currentId();

    void startFilters();
    void stopFilters();

private Q_SLOTS:

    void slotProgressTimerDone();
    void slotFilterStarted(int);
    void slotFilterFinished(int, const QPixmap&);

private:

    PreviewListItem* findItem(int id);

private:

    PreviewListPriv* const d;
};

} // namespace Digikam

#endif /* PREVIEWLIST_H */
