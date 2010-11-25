/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a
 *               template to an image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIRSELECTWIDGET_H
#define DIRSELECTWIDGET_H

// Qt includes

#include <QWidget>
#include <QString>

// KDE includes

#include <k3filetreeview.h>
#include <kurl.h>

namespace DigikamDecorateImagePlugin
{

class DirSelectWidget : public K3FileTreeView
{
    Q_OBJECT

public:

    explicit DirSelectWidget(QWidget* parent, const char* name=0, const QString& headerLabel=QString());

    explicit DirSelectWidget(const KUrl& rootUrl=KUrl("/"), const KUrl& currentUrl=KUrl(),
                             QWidget* parent=0, const char* name=0, const QString& headerLabel=QString());

    ~DirSelectWidget();

    KUrl path() const;
    KUrl rootPath() const;
    void setRootPath(const KUrl& rootUrl, const KUrl& currentUrl=KUrl(QString()));
    void setCurrentPath(const KUrl& currentUrl);

Q_SIGNALS:

    void folderItemSelected(const KUrl& url);

protected Q_SLOTS:

    void load();
    void slotFolderSelected(Q3ListViewItem*);

private:

    class DirSelectWidgetPrivate;
    DirSelectWidgetPrivate* const d;
};

}  // namespace DigikamDecorateImagePlugin

#endif /* DIRSELECTWIDGET_H */
