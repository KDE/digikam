/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a dialog to edit EXIF,IPTC and XMP metadata
 *
 * Copyright (C) 2011      by Victor Dodon <dodon dot victor at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATAEDIT_H
#define METADATAEDIT_H

// Qt includes

#include <QCloseEvent>
#include <QDialog>
#include <QUrl>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetadataEditDialog : public QDialog
{
    Q_OBJECT

public:

    MetadataEditDialog(QWidget* const parent, const QList<QUrl>& urls);
    ~MetadataEditDialog();

    QList<QUrl>::iterator currentItem() const;
    QString currentItemTitleHeader(const QString& title) const;

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

} // namespace Digikam

#endif // METADATAEDIT_H
