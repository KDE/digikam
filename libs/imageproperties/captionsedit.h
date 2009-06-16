/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : multi-languages captions editor
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAPTIONSEDIT_H
#define CAPTIONSEDIT_H

// Qt includes

#include <QWidget>
#include <QString>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

using namespace KExiv2Iface;

namespace Digikam
{

class captionsEditPriv;

class captionsEdit : public QWidget
{
    Q_OBJECT

public:

    captionsEdit(QWidget* parent);
    ~captionsEdit();

    void reset();

    void setValues(const KExiv2::AltLangMap& values);
    KExiv2::AltLangMap& values();

    QString defaultAltLang() const;
    bool    asDefaultAltLang() const;

signals:

    void signalModified();

private slots:

    void slotSelectionChanged(int);
    void slotAddValue();
    void slotDeleteValue();

private:

    void loadLangAltListEntries();

private:

    captionsEditPriv* const d;
};

}  // namespace Digikam

#endif // CAPTIONSEDIT_H
