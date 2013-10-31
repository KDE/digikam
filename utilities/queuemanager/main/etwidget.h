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

namespace Ui
{
struct etwidget;
}


struct ETConfig
{
    typedef QSharedPointer<ETConfig> Ptr;

    static ETConfig::Ptr config(const QString& tool = QString());

    KConfig main;
    KConfigGroup cfg;

    static const QString pluginName();

    static const QString firstStart;
    static const QString showInContext;
    static const QString shortcut;
    static const QString name;
    static const QString interpretter;
    static const QString script;

private:
    ETConfig();
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

    void scriptSelected(int index);
    void remove();

private:
    void update(const QString& tool);

private:
    QScopedPointer<Ui::etwidget> ui_;
};


#endif /* ET_SETTINGS_H */
