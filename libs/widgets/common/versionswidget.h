/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-03
 * Description : widget displaying all image versions in a list
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

class KConfigGroup;

namespace Digikam
{

class ImageVersionsModel;
class ImageInfo;
class ImageInfoList;

class VersionsWidget : public QWidget
{
    Q_OBJECT

public:

    VersionsWidget(QWidget* parent = 0);
    ~VersionsWidget();

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

public Q_SLOTS:

    void setCurrentItem(const ImageInfo& info);

Q_SIGNALS:

    void imageSelected(const ImageInfo& info);

protected Q_SLOTS:

    void slotViewItemSelected(const QModelIndex& index);
    void slotViewModeChanged(int mode);

private:

    class VersionsWidgetPriv;
    VersionsWidgetPriv* const d;
};

} // namespace Digikam

#endif // VERSIONSWIDGET_H
