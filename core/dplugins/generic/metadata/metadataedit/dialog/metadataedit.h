/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a dialog to edit EXIF,IPTC and XMP metadata
 *
 * Copyright (C) 2011      by Victor Dodon <dodon dot victor at gmail dot com>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_META_DATA_EDIT_H
#define DIGIKAM_META_DATA_EDIT_H

// Qt includes

#include <QCloseEvent>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "dplugindialog.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace GenericDigikamMetadataEditPlugin
{

class DIGIKAM_EXPORT MetadataEditDialog : public DPluginDialog
{
    Q_OBJECT

public:

    explicit MetadataEditDialog(QWidget* const parent, DInfoInterface* const iface);
    ~MetadataEditDialog();

    QList<QUrl>::iterator currentItem() const;
    QString currentItemTitleHeader(const QString& title) const;

Q_SIGNALS:

    void signalMetadataChangedForUrl(const QUrl&);
    
public Q_SLOTS:

    void slotModified();

private Q_SLOTS:

    void slotOk();
    void slotClose();
    void slotItemChanged();
    void slotApply();
    void slotNext();
    void slotPrevious();
    void slotSetReadOnly(bool);

protected:

    void closeEvent(QCloseEvent*);
    bool eventFilter(QObject*, QEvent*);

private:

    void saveSettings();
    void readSettings();
    void updatePreview();

private:

    class Private;
    Private* const d;
};

} // namespace GenericDigikamMetadataEditPlugin

#endif // DIGIKAM_META_DATA_EDIT_H
