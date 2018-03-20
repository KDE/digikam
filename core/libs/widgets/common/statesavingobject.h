/* ============================================================
 *
 * This file is a part of digikam project
 * http://www.digikam.org
 *
 * Date        : 2009-20-12
 * Description : Interface class for objects that can store their state.
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef STATESAVINGOBJECT_H
#define STATESAVINGOBJECT_H

// Qt includes

#include <QObject>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * An interface-like class with utility methods and a general public interface
 * to support state saving and restoring for objects via KConfig. Use this class
 * as a Mixin.
 *
 * The public interface for loading and saving state is implemented designed as
 * template methods. To store or restore the state of a class, inherit from
 * this class via multiple inheritance and implement doLoadState() and
 * doSaveState(). In these methods always use the protected method
 * getConfigGroup() to access a config group. Also always use the entryName()
 * method for generating keys in the config (for prefixes, see below).
 *
 * Ensure that this class is inherited after a QObject-based class and pass
 * "this" as constructor argument.
 *
 * By default a config group based on Qt's object name of the class is used.
 * This behaviour can be changed by setting a dedicated config group via
 * setConfigGroup(). This is useful for to externally control the config group
 * and shouldn't be used inside the implementing class.
 *
 * Additionally to setting the config group, also a prefix for each config group
 * entry can be defined via setEntryPrefix(). This may be useful if multiple
 * instances of the same class shall be stored in the same config group or can
 * generally be a good idea to make the config more readable and recognizable.
 * By default this prefix is empty.
 *
 * This class also supports recursive saving / loading invocations based on the
 * QT object hierarchy. As default, calls to loadState() or saveState() only
 * invoke the doLoadState() or doStateSave() method of the called instance.
 * This behaviour can be changed with setStateSavingDepth() to automatically
 * call children of the instance. Various modes are supported as documented in
 * StateSavingDepth.
 *
 * @author jwienke
 */
class DIGIKAM_EXPORT StateSavingObject
{
public:

    /**
     * This enum defines the "depth" of the StateSavingObject#loadState() and
     * StateSavingObject#saveState() methods.
     */
    enum StateSavingDepth
    {
        /**
         * Only the instance the saving / restoring was invoked on is
         * saved / restored.
         */
        INSTANCE,

        /**
         * The instance itself and all direct children of this instance implementing
         * StateSavingObject are saved / restored.
         */
        DIRECT_CHILDREN,

        /**
         * The instance and all children in the complete hierarchy are saved /
         * restored.
         */
        RECURSIVE
    };

    /**
     * Constructor. Must be called after any QObject-based constructor.
     *
     * @param host self-reference to access the object name, simply pass "this"
     *             as argument
     */
    explicit StateSavingObject(QObject* const host);

    /**
     * Destructor.
     */
    virtual ~StateSavingObject();

    /**
     * Returns the depth used for state saving or loading. Default is
     * StateSavingDepth::INSTANCE.
     *
     * @return state saving / restoring depth
     */
    StateSavingDepth getStateSavingDepth() const;

    /**
     * Sets the depth used for state saving or loading.
     *
     * @param depth new depth to use
     */
    void setStateSavingDepth(const StateSavingDepth depth);

    /**
     * Sets a dedicated config group that will be used to store and reload
     * the state from. If this method is not called, a group based on the
     * object name is used.
     *
     * You can re-implement this method to pass the group set here to child
     * objects. Don't forget to call this method in your implementation.
     *
     * @param group config group to use for state saving and restoring
     */
    virtual void setConfigGroup(const KConfigGroup& group);

    /**
     * Define a prefix that will be used for every entry in the config group.
     * The default prefix is empty.
     *
     * You can re-implement this method to pass the prefix set here to child
     * objects. Don't forget to call this method in your implementation.
     *
     * @param prefix prefix to use for the config entries
     */
    virtual void setEntryPrefix(const QString& prefix);

    /**
     * Invokes loading the class' state.
     */
    void loadState();

    /**
     * Invokes saving the class' state.
     */
    void saveState();

protected:

    /**
     * Implement this hook method for state loading. Use getConfigGroup() and
     * entryName() for the implementation.
     */
    virtual void doLoadState() = 0;

    /**
     * Implement this hook method for state saving. Use getConfigGroup() and
     * entryName() for the implementation.
     */
    virtual void doSaveState() = 0;

    /**
     * Returns the config group that must be used for state saving and loading.
     *
     * @return config group for state saving and loading
     */
    KConfigGroup getConfigGroup() const;

    /**
     * Always use this method to create config group entry names. This allows
     * to manipulate the entry keys externally by eg. setting a prefix.
     *
     * @param base original name planned for the config group entry
     * @return entry name after manipulating it with externally set parameters
     */
    QString entryName(const QString& base) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // STATESAVINGOBJECT_H
