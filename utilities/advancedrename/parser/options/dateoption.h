/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide date information to the parser
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

#ifndef DATEOPTION_H
#define DATEOPTION_H

// Qt includes

#include <QObject>
#include <QString>
#include <QDialog>

// Local includes

#include "parseobjectdialog.h"
#include "option.h"

namespace Ui
{
    class DateOptionDialogWidget;
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

class DateOptionDialog : public ParseObjectDialog
{
    Q_OBJECT

public:

    DateOptionDialog(ParseObject* parent);
    ~DateOptionDialog();

    Ui::DateOptionDialogWidget* const ui;

private Q_SLOTS:

    void slotDateFormatChanged(int);
    void slotCustomFormatChanged(const QString&);

private:

    QString formattedDateTime(const QDateTime& date);
    void    updateExampleLabel();
};

class DateOption : public Option
{
    Q_OBJECT

public:

    DateOption();
    ~DateOption() {};

protected:

    virtual void parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);
};

} // namespace Digikam

#endif /* DATEOPTION_H */
