/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ABSTRACT_THEME_PARAMETER_H
#define ABSTRACT_THEME_PARAMETER_H

class QByteArray;
class QString;
class QWidget;

class KConfigGroup;

namespace Digikam
{

/**
 * Represents a theme parameter. For each type of parameter, one should inherit
 * from this class and add the necessary code in the Theme class to load the
 * new type.
 */
class AbstractThemeParameter
{
public:

    explicit AbstractThemeParameter();
    virtual ~AbstractThemeParameter();

    /**
     * Reads theme parameters from configGroup. Initializes the internalName,
     * name and defaultValue fields.
     */
    virtual void init(const QByteArray& internalName, const KConfigGroup* configGroup);

    QByteArray internalName() const;

    QString name() const;

    QString defaultValue() const;

    /**
     * This method should return a QWidget representing the parameter,
     * initialized with value.
     */
    virtual QWidget* createWidget(QWidget* parent, const QString& value) const = 0;

    /**
     * The opposite of createWidget: given a widget previously created with
     * createWidget, this method returns the current widget value.
     */
    virtual QString valueFromWidget(QWidget*) const = 0;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ABSTRACT_THEME_PARAMETER_H
