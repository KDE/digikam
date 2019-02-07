/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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
#include <QPointer>

// Local includes

#include "dpluginauthor.h"
#include "digikam_export.h"

/** The plugin interface identifier definitions shared between DPlugins and Loader to
 *  check the binary compatibility at run-time.
 */
#define DIGIKAM_DPLUGIN_GENERIC_IID "org.kde.digikam.DPluginGeneric/1.1.0"
#define DIGIKAM_DPLUGIN_EDITOR_IID  "org.kde.digikam.DPluginEditor/1.1.0"
#define DIGIKAM_DPLUGIN_BQM_IID     "org.kde.digikam.DPluginBqm/1.1.0"

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

public:

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

    /**
     * @brief Returns the file name of the library for this plugin.
     *        This string is filled at run-time by plugin loader.
     */
    QString libraryFileName() const;

    /**
     * @brief Sets the file name of the library for this plugin.
     *        This string is filled at run-time by plugin loader.
     */
    void setLibraryFileName(const QString&);

public:

    /**
     * Return true if plugin can be configured in setup dialog about the visibility property.
     * Default implementation return true.
     */
    virtual bool hasVisibilityProperty() const;

    /**
     * Holds whether the plugin can be seen in parent view.
     */
    virtual void setVisible(bool b) = 0;

    /**
     * Return the amount of tools registered to all parents.
     */
    virtual int count() const = 0;

    /**
     * Return a list of categories as strings registered in this plugin.
     */
    virtual QStringList categories() const = 0;

    /**
     * Plugin factory method to create all internal object instances for a given parent.
     */
    virtual void setup(QObject* const parent) = 0;

    /**
     * @brief Returns the user-visible name of the plugin.
     *
     * The user-visible name should be context free, i.e. the name should
     * provide enough information as to what the plugin is about in the context
     * of digiKam.
     */
    virtual QString name() const = 0;

    /**
     * @brief Returns the unique internal identification property of the plugin.
     *        Must be formated as "org.kde.digikam.plugin._PLUGIN_TYPE_._NAME_OF_PLUGIN_".
     *        Examples: "org.kde.digikam.plugin.generic.Calendar"
     *                  "org.kde.digikam.plugin.editor.AdjustCurvesTool"
     *                  "org.kde.digikam.plugin.bqm.NoiseReduction"
     */
    virtual QString iid() const = 0;

    /**
     * @brief Returns the unique top level internal identification property of the plugin interface.
     *        Must be formated as "org.kde.digikam._NAME_OF_INTERFACE_/_VERSION_".
     *        Examples: "org.kde.digikam.DPluginGeneric/1.1.0"
     *                  "org.kde.digikam.DPluginEditor/1.1.0"
     *                  "org.kde.digikam.DPluginBqm/1.1.0"
     */
    virtual QString ifaceIid() const = 0;

    /**
     * @brief Returns a short description about the plugin.
     */
    virtual QString description() const = 0;

    /**
     * @brief Returns an icon for the plugin.
     *        Default implementation return the system plugin icon.
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

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_TYPEINFO(Digikam::DPluginAuthor, Q_MOVABLE_TYPE);

#endif // DIGIKAM_DPLUGIN_H
