/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-mm-dd
 * Description :
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef VERSIONSWIDGET_H
#define VERSIONSWIDGET_H

// Qt includes

#include <QWidget>
#include <QModelIndex>

// KDE includes

#include <KUrl>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ImageVersionsModel;
class ImageInfoList;

class DIGIKAM_EXPORT VersionsWidget : public QWidget
{
    Q_OBJECT

public:
    VersionsWidget(QWidget* parent = 0);
    ~VersionsWidget();
    void setupModelData(QList< QPair< QString, int > >& list) const;
    void setCurrentSelectedImage(const QString& path) const;

public Q_SLOTS:
    
    void slotDigikamViewNoCurrentItem();
    void slotViewItemSelected(QModelIndex index);
    
Q_SIGNALS:
    
    void newVersionSelected(KUrl url);

private:
    class VersionsWidgetPriv;
    VersionsWidgetPriv* const d;
};

} // namespace Digikam

#endif // VERSIONSWIDGET_H
