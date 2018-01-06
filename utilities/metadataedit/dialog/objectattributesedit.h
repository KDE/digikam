/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-08
 * Description : a widget to edit Application2 ObjectAttribute
 *               Iptc tag.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef OBJECT_ATTRIBUTES_EDIT_H
#define OBJECT_ATTRIBUTES_EDIT_H

// Qt includes

#include <QWidget>

namespace Digikam
{

class ObjectAttributesEdit : public QWidget
{
    Q_OBJECT

public:

    explicit ObjectAttributesEdit(QWidget* const parent, bool ascii=true, int size=-1);
    ~ObjectAttributesEdit();

    void setValid(bool v);
    bool isValid() const;

    void setValues(const QStringList& values);
    bool getValues(QStringList& oldValues, QStringList& newValues);

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSelectionChanged();
    void slotAddValue();
    void slotDeleteValue();
    void slotReplaceValue();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // OBJECT_ATTRIBUTES_EDIT_H
