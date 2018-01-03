/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-16
 * Description : XMP categories settings page.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013      by Victor Dodon <dodonvictor at gmail dot com>
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

#ifndef XMP_CATEGORIES_H
#define XMP_CATEGORIES_H

// Qt includes

#include <QWidget>
#include <QByteArray>

namespace Digikam
{

class XMPCategories : public QWidget
{
    Q_OBJECT

public:

    explicit XMPCategories(QWidget* const parent);
    ~XMPCategories();

    void applyMetadata(QByteArray& xmpData);
    void readMetadata(QByteArray& xmpData);

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotCategorySelectionChanged();
    void slotAddCategory();
    void slotDelCategory();
    void slotRepCategory();

    void slotCheckCategoryToggled(bool checked);
    void slotCheckSubCategoryToggled(bool checked);

private:

    void enableWidgets(bool checked1, bool checked2);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // XMP_CATEGORIES_H
