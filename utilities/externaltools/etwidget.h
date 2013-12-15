/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2010-11-15
 * Description : a kipi plugin to export images to Yandex.Fotki web service
 *
 * Copyright (C) 2010 by Roman Tsisyk <roman at tsisyk dot com>
 *
 * GUI based on PicasaWeb KIPI Plugin
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Luka Renko <lure at kubuntu dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ET_WIDGET_H
#define ET_WIDGET_H

#include <QWidget>
#include <QSharedPointer>

#include <kconfig.h>
#include <kconfiggroup.h>

struct ETConfig
{
    //Language to access KConfig fields and groups
    
    static const QString pluginName();

    static const QString firstStart;
    static const QString showInContext;
    static const QString shortcut;
    static const QString name;
    static const QString type;
    static const QString showTerminal;
    static const QString noClose;
    static const QString terminalArgs;
    
    struct Type
    {
        struct Executable
        {
            static const QString type;
            static const QString path;
            static const QString arguments;
        };
        struct Script
        {
            static const QString type;
            static const QString path;
            static const QString script;
        };
        struct SimpleScript
        {
            static const QString type;
            static const QString path;
            static const QString body;
        };
    };

    typedef QSharedPointer<ETConfig> Ptr;

    static ETConfig::Ptr config(const QString& tool = QString());

    template <typename T>
    T readEntry(const QString& path, const T& defvalue) const
    {
        return cfg.readEntry<T>(path, defvalue);
    }
    
    template <typename T>
    T readTypeEntry(const QString& type, const QString& path, const T& defvalue) const
    {
        return cfg.group(type).readEntry<T>(path, defvalue);
    }
    
    template <typename T>
    void writeEntry(const QString& path, const T& value)
    {
        cfg.writeEntry<T>(path, value);
    }
    
    template <typename T>
    void writeTypeEntry(const QString& type, const QString& path, const T& value)
    {
        cfg.group(type).writeEntry<T>(path, value);
    }
    
    KConfigGroup cfg;
    
private:
    ETConfig();
    
private:
    KConfig main;
};

class ETWidget : public QWidget
{
    Q_OBJECT

public:

    ETWidget(QWidget* parent, const QString& tool = QString());
    ~ETWidget();

    QString currentTool();

public Q_SLOTS:
    void save();

private Q_SLOTS:

    void typeSelected(int index);
    void scriptSelected(int index);
    void scriptRenamed(const QString&);
    void remove();
    void scriptEdit(bool);

private:
    void update(const QString& tool);

private:
    class Private;
    QScopedPointer<Private> d;    
};


#endif /* ET_SETTINGS_H */
