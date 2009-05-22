/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : an input widget that allows manual renaming of files
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef MANUALRENAMEINPUT_H
#define MANUALRENAMEINPUT_H

// Qt includes

#include <QWidget>
#include <QString>

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

}  // namespace Digikam

#endif /* MANUALRENAMEINPUT_H */
