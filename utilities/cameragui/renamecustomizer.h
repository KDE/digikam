/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : a options group to set renaming files
 *               operations during camera downloading
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef RENAMECUSTOMIZER_H
#define RENAMECUSTOMIZER_H

// Qt includes

#include <QMap>
#include <QRegExp>
#include <QWidget>

class QDateTime;

class KLineEdit;

namespace Digikam
{

class ManualRenameInputPriv;

class ManualRenameInput : public QWidget
{
    Q_OBJECT

public:

    ManualRenameInput(QWidget* parent = 0);
    ~ManualRenameInput();

    QString text() const;
    void    setText(const QString& text);

    void setTrackerAlignment(Qt::Alignment alignment);

    KLineEdit* input() const;


    QString parse(const QString& fileName, const QString& cameraName,
                  const QDateTime& dateTime, int index) const;

    static QString parser(const QString& parseString,
                          const QString& fileName, const QString& cameraName,
                          const QDateTime& dateTime, int index);

Q_SIGNALS:

    void signalTextChanged(const QString&);

public Q_SLOTS:

    void slotUpdateTrackerPos();
    void slotHideToolTipTracker();

private Q_SLOTS:

    void slotToolTipButtonToggled(bool);

private:

    QString createToolTip();

private:

    ManualRenameInputPriv* const d;
};

// ------------------------------------------------------------

class RenameCustomizerPriv;

class RenameCustomizer : public QWidget
{
    Q_OBJECT

public:

    enum Case
    {
        NONE = 0,
        UPPER,
        LOWER
    };

    RenameCustomizer(QWidget* parent, const QString& cameraTitle);
    ~RenameCustomizer();

    void    setUseDefault(bool val);
    bool    useDefault() const;
    QString newName(const QString& fileName, const QDateTime& date, int index, const QString& extension) const;
    Case    changeCase() const;
    int     startIndex() const;

Q_SIGNALS:

    void signalChanged();

public Q_SLOTS:

    void restoreFocus();
    void slotUpdateTrackerPos();
    void slotHideToolTipTracker();

private:

    void readSettings();
    void saveSettings();

private Q_SLOTS:

    void slotRadioButtonClicked(int);
    void slotRenameOptionsChanged();
    void slotDateTimeBoxToggled(bool);
    void slotDateTimeFormatChanged(int);
    void slotDateTimeButtonClicked();

private:

    RenameCustomizerPriv* const d;
};

}  // namespace Digikam

#endif /* RENAMECUSTOMIZER_H */
