/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a date parser class
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

#ifndef DATEPARSER_H
#define DATEPARSER_H

// Qt includes

#include <QString>
#include <QDialog>

// KDE includes

#include <kdialog.h>

// Local includes

#include "subparser.h"

namespace Ui
{
    class DateParserDialogWidget;
}

namespace Digikam
{

class DateFormat
{
public:

    DateFormat();
    ~DateFormat() {};

    typedef QPair<QString, QVariant> DateFormatDescriptor;
    typedef QList<DateFormatDescriptor> DateFormatMap;
    enum Type
    {
        Standard = 0,
        ISO,
        FullText,
        Locale,
        Custom
    };

    QString  identifier(Type type);

    QVariant formatType(Type type);
    QVariant formatType(QString identifier);

    DateFormatMap& map() { return m_map; };

private:

    DateFormatMap m_map;
};

// --------------------------------------------------------

class DateParserDialog : public KDialog
{
    Q_OBJECT

public:

    DateParserDialog(QWidget* parent = 0);
    ~DateParserDialog();

    Ui::DateParserDialogWidget* const ui;

private Q_SLOTS:

    void slotDateFormatChanged(int);
    void slotCustomFormatChanged(const QString&);

private:

    QString formattedDateTime(const QDateTime& date);
    void    updateExampleLabel();
};

class DateParser : public SubParser
{
    Q_OBJECT

public:

    DateParser();
    ~DateParser() {};

protected:

    virtual void parseOperation(const QString& parseString, const ParseInformation& info, ParseResultsMap& map);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);
};

} // namespace Digikam

#endif /* DATEPARSER_H */
