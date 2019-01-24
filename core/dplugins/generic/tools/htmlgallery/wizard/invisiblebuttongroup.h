/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_INVISIBLE_BUTTON_GROUP_H
#define DIGIKAM_INVISIBLE_BUTTON_GROUP_H

// Qt includes

#include <QWidget>

class QAbstractButton;

namespace GenericDigikamHtmlGalleryPlugin
{

class InvisibleButtonGroup : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int current READ selected WRITE setSelected)

public:

    explicit InvisibleButtonGroup(QWidget* const parent);
    virtual ~InvisibleButtonGroup();

    int selected() const;

    void addButton(QAbstractButton* button, int id);

public Q_SLOTS:

    void setSelected(int id);

Q_SIGNALS:

    void selectionChanged(int id);

private:

    InvisibleButtonGroup(); // Disable

    class Private;
    Private* const d;
};

} // namespace GenericDigikamHtmlGalleryPlugin

#endif // DIGIKAM_INVISIBLE_BUTTON_GROUP_H
