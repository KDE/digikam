/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : abstract class to define digiKam plugin
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DPLUGIN_H
#define DIGIKAM_DPLUGIN_H

// Qt includes

#include <QList>
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QtPlugin>
#include <QObject>
#include <QIcon>

// Local includes

#include "dinfointerface.h"
#include "dpluginauthor.h"
#include "dpluginaction.h"
#include "digikam_export.h"

namespace Digikam
{

/**
 * A digiKam external plugin abstract class.
 */
class DIGIKAM_EXPORT DPlugin : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPlugin(QObject* const parent = 0);

    /**
     * Destructor
     */
    ~DPlugin() override;

    /**
     * Manage the info interface instance set by plugin loader.
     */
    void setInfoIface(DInfoInterface* const iface);
    DInfoInterface* infoIface() const;

    /**
     * Return all plugin actions registered in setup() method with addAction() for a given parent.
     */
    QList<DPluginAction*> actions(QObject* const parent) const;

    /**
     * Return the count of action registered to all parents.
     */
    int count() const;

    /**
     * Return a plugin action instance found by name in plugin action list for a given parent.
     */
    DPluginAction* findActionByName(const QString& name, QObject* const parent) const;

    /**
     * Return a list of categories as strings registered in this plugin.
     */
    QStringList pluginCategories() const;

    /**
     * Return a list of authors as strings registered in this plugin.
     */
    QStringList pluginAuthors() const;

    /**
     * Return the internal version used to check the binary compatibility at run-time.
     * This is typically the same version of digiKam core used at compilation time.
     */
    QString version() const;

    /**
     * Return the should loaded property.
     * If it's true, the plugin must be loaded in appliction GUI at startup by plugin loader.
     */
    bool shouldLoaded() const;

    /**
     * Accessor to adjust the should loaded plugin property.
     * This property is adjusted by plugin loader at start-up.
     */
    void setShouldLoaded(bool b);

public:

    /**
     * Plugin factory method to create all internal actions for a given parent windows list.
     * To retrieve all plugin actions, use actions() public methods.
     * To register a new plugin action in this method, use addAction() protected method.
     */
    virtual void setup(const QList<QObject*>& parents) = 0;

    /**
     * @brief Returns the user-visible name of the plugin.
     *
     * The user-visible name should be context free, i.e. the name should
     * provide enough information as to what the plugin is about in the context
     * of digiKam.
     */
    virtual QString name() const = 0;

    /**
     * @brief Returns the unique identification string of the plugin.
     */
    virtual QString id() const = 0;

    /**
     * @brief Returns a short description about the plugin.
     */
    virtual QString description() const = 0;

    /**
     * @brief Returns an icon for the plugin.
     *        Default implementation return host application icon.
     */
    virtual QIcon icon() const;

    /**
     * @brief Returns authors list for the plugin.
     */
    virtual QList<DPluginAuthor> authors() const = 0;

    /**
     * @brief Returns a long description about the plugin.
     */
    virtual QString details() const = 0;

protected:

    void addAction(DPluginAction* const ac);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_TYPEINFO(Digikam::DPluginAuthor, Q_MOVABLE_TYPE);

Q_DECLARE_INTERFACE(Digikam::DPlugin, "org.kde.digikam.DPlugin/1.0.0" )

#endif // DIGIKAM_DPLUGIN_H
