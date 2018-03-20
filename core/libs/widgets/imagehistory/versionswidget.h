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
#include <QUrl>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class ActionVersionsOverlay;
class ImageInfo;
class ShowHideVersionsOverlay;
class VersionsDelegate;
class VersionsTreeView;

class VersionsWidget : public QWidget
{
    Q_OBJECT

public:

    explicit VersionsWidget(QWidget* const parent = 0);
    ~VersionsWidget();

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    VersionsTreeView*        view()     const;
    VersionsDelegate*        delegate() const;

    ActionVersionsOverlay*   addActionOverlay(const QIcon& icon, const QString& text, const QString& tip = QString());
    ShowHideVersionsOverlay* addShowHideOverlay();

public Q_SLOTS:

    void setCurrentItem(const ImageInfo& info);

Q_SIGNALS:

    void imageSelected(const ImageInfo& info);

protected Q_SLOTS:

    void slotViewCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void slotViewModeChanged(int mode);
    void slotSetupChanged();

private:

    void applyViewMode();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // VERSIONSWIDGET_H
