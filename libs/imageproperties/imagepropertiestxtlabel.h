/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-05
 * Description : simple text labels to display image
 *               properties meta infos
 *
 * Copyright (C) 2008-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPROPERTIESTXTLABEL_H
#define IMAGEPROPERTIESTXTLABEL_H

// Qt includes

#include <QLabel>
#include <QPalette>
#include <QColor>
#include <QString>
#include <QFontMetrics>
#include <QTextBrowser>
#include <QListWidget>

// KDE includes

#include <kglobalsettings.h>

// Libkdcraw includes

#include <rwidgetutils.h>

using namespace KDcrawIface;

class DTextLabelName : public QLabel
{

public:

    explicit DTextLabelName(const QString& name, QWidget* const parent=0)
        : QLabel(parent)
    {
        setText(name);
        QFont fnt = KGlobalSettings::smallestReadableFont();
        fnt.setItalic(true);
        setFont(fnt);
        setAlignment(Qt::AlignRight | Qt::AlignTop);
        setWordWrap(false);
    };

    ~DTextLabelName()
    {
    };
};

// -------------------------------------------------------------------

class DTextLabelValue : public RAdjustableLabel
{

public:

    explicit DTextLabelValue(const QString& value, QWidget* const parent=0)
        : RAdjustableLabel(parent)
    {
        setAdjustedText(value);
        setFont(KGlobalSettings::smallestReadableFont());
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setWordWrap(false);
        setElideMode(Qt::ElideRight);
    };

    ~DTextLabelValue()
    {
    };
};

// -------------------------------------------------------------------

class DTextBrowser : public QTextBrowser
{
public:

    explicit DTextBrowser(const QString& text, QWidget* const parent=0)
        : QTextBrowser(parent)
    {
        setOpenExternalLinks(false);
        setOpenLinks(false);
        setText(text);
        setLinesNumber(3);
        setFocusPolicy(Qt::NoFocus);
    };

    ~DTextBrowser()
    {
    };

    void setLinesNumber(int l)
    {
        QFont fnt = KGlobalSettings::smallestReadableFont();
        document()->setDefaultFont(fnt);
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        setFixedHeight(top + bottom + frameWidth() + fontMetrics().lineSpacing()*l);
    };
};

// -------------------------------------------------------------------

class DTextList : public QListWidget
{
public:

    explicit DTextList(const QStringList& list, QWidget* const parent=0)
        : QListWidget(parent)
    {
        addItems(list);
        setLinesNumber(6);
        setFocusPolicy(Qt::NoFocus);
        sortItems();
    };

    ~DTextList()
    {
    };

    void setLinesNumber(int l)
    {
        QFont fnt = KGlobalSettings::smallestReadableFont();
        setFont(fnt);
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        setFixedHeight(top + bottom + frameWidth() + fontMetrics().lineSpacing()*l);
    };
};

#endif /* IMAGEPROPERTIESTXTLABEL_H */
