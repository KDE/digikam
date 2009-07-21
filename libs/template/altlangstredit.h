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
    void setClickMessage(const QString& msg);

    void setValues(const KExiv2::AltLangMap& values);
    KExiv2::AltLangMap& values();

    QString currentLanguageCode() const;
    QString languageCode(int index) const;

    QString defaultAltLang() const;
    bool    asDefaultAltLang() const;

    /** Reset all entries
     */
    void reset();

    /** Force current text to be registered in captions map
     */
    void apply();

    void setDirty(bool dirty);
    bool isDirty() const;

Q_SIGNALS:

    void signalModified();
    void signalSelectionChanged(const QString&);
    void signalAddValue(const QString&, const QString&);
    void signalDeleteValue(const QString&);

protected Q_SLOTS:

    void slotTextChanged();
    void slotSelectionChanged(int);
    void slotAddValue();
    void slotDeleteValue();

protected:

    void loadLangAltListEntries(const QString& currentLang=QString("x-default"));

private:

    AltLangStrEditPriv* const d;
};

}  // namespace Digikam

#endif // ALTLANGSTREDIT_H
