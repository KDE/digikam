/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : multi-languages string editor
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

#ifndef ALTLANGSTREDIT_H
#define ALTLANGSTREDIT_H

// Qt includes

#include <QtGui/QWidget>
#include <QtCore/QString>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

using namespace KExiv2Iface;

namespace Digikam
{

class AltLangStrEditPriv;

class AltLangStrEdit : public QWidget
{
    Q_OBJECT

public:

    AltLangStrEdit(QWidget* parent);
    ~AltLangStrEdit();

    void setTitle(const QString& title);

    void reset();

    void setValues(const KExiv2::AltLangMap& values);
    KExiv2::AltLangMap& values();

    QString defaultAltLang() const;
    bool    asDefaultAltLang() const;

    /** Force current text to be registered in captions map
     */
    void apply();

signals:

    void signalModified();

private slots:

    void slotSelectionChanged(int);
    void slotAddValue();
    void slotDeleteValue();
    void slotTextChanged();

private:

    void loadLangAltListEntries(const QString& currentLang=QString());

private:

    AltLangStrEditPriv* const d;
};

}  // namespace Digikam

#endif // ALTLANGSTREDIT_H
