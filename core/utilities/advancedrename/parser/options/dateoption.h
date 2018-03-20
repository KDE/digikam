/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide date information to the parser
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DATEOPTION_H
#define DATEOPTION_H

// Qt includes

#include <QDialog>
#include <QObject>
#include <QString>

// Local includes

#include "option.h"
#include "ruledialog.h"

namespace Ui
{
class DateOptionDialogWidget;
}

namespace Digikam
{

class DateFormat
{
public:

    enum Type
    {
        Standard = 0,
        ISO,
        FullText,
        UnixTimeStamp,
        Custom
    };

public:

    typedef QPair<QString, QVariant>    DateFormatDescriptor;
    typedef QList<DateFormatDescriptor> DateFormatMap;

public:

    DateFormat();
    ~DateFormat() {};

    Type     type(const QString& identifier);

    QString  identifier(Type type);

    QVariant format(Type type);
    QVariant format(const QString& identifier);

    DateFormatMap& map()
    {
        return m_map;
    };

private:

    DateFormat(const DateFormat&);
    DateFormat& operator=(const DateFormat&);

private:

    DateFormatMap m_map;
};

// --------------------------------------------------------

class DateOptionDialog : public RuleDialog
{
    Q_OBJECT

public:

    enum DateSource
    {
        FromImage = 0,
        CurrentDateTime,
        FixedDateTime
    };

public:

    explicit DateOptionDialog(Rule* parent);
    ~DateOptionDialog();

    Ui::DateOptionDialogWidget* const ui;

    DateSource dateSource();

private Q_SLOTS:

    void slotDateSourceChanged(int);
    void slotDateFormatChanged(int);
    void slotCustomFormatChanged(const QString&);

private:

    DateOptionDialog(const DateOptionDialog&);
    DateOptionDialog& operator=(const DateOptionDialog&);

    QString formattedDateTime(const QDateTime& date);
    void    updateExampleLabel();
};

// --------------------------------------------------------

class DateOption : public Option
{
    Q_OBJECT

public:

    DateOption();
    ~DateOption() {};

protected:

    virtual QString parseOperation(ParseSettings& settings);

private:

    DateOption(const DateOption&);
    DateOption& operator=(const DateOption&);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);
};

} // namespace Digikam

#endif /* DATEOPTION_H */
