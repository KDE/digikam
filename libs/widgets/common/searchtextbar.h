/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string.
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SEARCH_TEXT_BAR_H
#define SEARCH_TEXT_BAR_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// KDE includes.

#include <klineedit.h>
#include <klocale.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DLineEditPriv;
class SearchTextBarPriv;

class DIGIKAM_EXPORT DLineEdit : public KLineEdit
{
    Q_OBJECT

public:

    DLineEdit(const QString &msg, QWidget *parent);
    ~DLineEdit();

    void    setMessage(const QString &msg);
    QString message() const;

    void setText(const QString& txt);

protected:

    void drawContents(QPainter *p);
    void dropEvent(QDropEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);

private :

    DLineEditPriv* d;
};

class DIGIKAM_EXPORT SearchTextBar : public QWidget
{
Q_OBJECT

public:

    SearchTextBar(QWidget *parent, const char* name, const QString &msg=i18n("Search..."));
    ~SearchTextBar();

    void setText(const QString& text);
    QString text() const;

    void setEnableTextQueryCompletion(bool b);
    bool textQueryCompletion() const;

    DLineEdit *lineEdit() const;

signals:

    void signalTextChanged(const QString&);

public slots:

    void slotSearchResult(bool);

private slots:

    void slotTextChanged(const QString&);

private :

    SearchTextBarPriv* d;
};

}  // namespace Digikam

#endif /* SEARCH_TEXT_BAR_H */
