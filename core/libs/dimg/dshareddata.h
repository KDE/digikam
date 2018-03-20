/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-21
 * Description : Shared data with reference counting and explicit sharing
 *
 * Copyright (C) 1992-2006 Trolltech ASA.
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DSHAREDDATA_H
#define DSHAREDDATA_H

// Qt includes

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DSharedData
{
    /**
     * Classes that are used with a DSharedDataPointer shall inherit from
     * this class.
     */
public:

    QAtomicInt ref;

    inline DSharedData()
        : ref(0)
    {
    }

    inline DSharedData(const DSharedData&)
        : ref(0)
    {
    }

    /**
     * Returns true if the reference count is not 0.
     * For the normal use case, you do not need this method.
     */
    inline bool isReferenced() const
    {
        return (int)ref > 0;
    }

    inline bool hasMoreReferences() const
    {
        return (int)ref != 1;
    }

private:
    // using the assignment operator would lead to corruption in the ref-counting
    DSharedData& operator=(const DSharedData&);
};

// --------------------------------------------------------------------------------------

template <class T> class DSharedDataPointer
{

public:

    /**
     * Use this class to store pointers to a shared data object, which
     * inherits DSharedData.
     * This class is inspired by QSharedDataPointer, but differs in two points:
     * - it provides "explicit sharing": A change to the data affects all classes
     *   keeping a pointer to the shared data. No automatic copying is done.
     * - no method "detach()" is provided, acknowledging the fact that the
     *   copy constructor of class T may not be used.
     */

    /**
     * Various operators for accessing the pointer const and non-const
     */
    inline T& operator*()
    {
        return *d;
    }

    inline const T& operator*() const
    {
        return *d;
    }

    inline T* operator->()
    {
        return d;
    }

    inline const T* operator->() const
    {
        return d;
    }

    inline operator T* ()
    {
        return d;
    }

    inline operator const T* () const
    {
        return d;
    }

    inline T* data()
    {
        return d;
    }

    inline const T* data() const
    {
        return d;
    }

    inline const T* constData() const
    {
        return d;
    }

    /**
     * This method carries out a const_cast, so it returns a non-const pointer
     * from a const DSharedDataPointer.
     * Typically, this should only be used if you know it should be used
     * (to implement a lazy loading caching technique or similar)
     */
    inline T* constCastData() const
    {
        return const_cast<T*>(d);
    }

    inline bool operator==(const DSharedDataPointer<T>& other) const
    {
        return d == other.d;
    }

    inline bool operator!=(const DSharedDataPointer<T>& other) const
    {
        return d != other.d;
    }

    inline DSharedDataPointer()
    {
        d = 0;
    }

    explicit inline DSharedDataPointer(T* const data)
        : d(data)
    {
        if (d)
        {
            d->ref.ref();
        }
    }

    inline ~DSharedDataPointer()
    {
        if (d && !d->ref.deref())
        {
            delete d;
        }
    }

    inline DSharedDataPointer(const DSharedDataPointer<T>& o)
        : d(o.d)
    {
        if (d)
        {
            d->ref.ref();
        }
    }

    inline DSharedDataPointer<T>& operator=(const DSharedDataPointer<T>& o)
    {
        delete assign(o);
        return *this;
    }

    inline DSharedDataPointer& operator=(T* const o)
    {
        delete assign(o);
        return *this;
    }

    /**
     * The assign operator is like operator=,
     * with the difference that the old pointer is not deleted
     * if its reference count is 0, but returned.
     * Use this if you need to do your own deleting, if e.g.
     * the object need to be removed from a list or a cache.
     * @returns A T object with reference count 0, which may be deleted;
     *          or 0 if no object need to be dropped.
     */
    inline T* assign(const DSharedDataPointer<T>& o)
    {
        if (o.d != d)
        {
            // reference new value
            if (o.d)
            {
                o.d->ref.ref();
            }

            // store old value
            T* x = d;
            // assign new value
            d    = o.d;

            // dereference old value,
            // return value and ownership if dereferenced
            if (x && !x->ref.deref())
            {
                return x;
            }
        }

        return 0;
    }

    inline T* assign(T* const o)
    {
        if (o != d)
        {
            // reference new value
            if (o)
            {
                o->ref.ref();
            }

            // store old value
            T* x = d;
            // assign new value
            d    = o;

            // dereference old value,
            // return value and ownership if dereferenced
            if (x && !x->ref.deref())
            {
                return x;
            }
        }

        return 0;
    }

    /**
     * Semantics like assign, but no new pointer is assigned to this.
     */
    inline T* unassign()
    {
        return assign(0);
    }

    inline bool operator!() const
    {
        return !d;
    }

private:

    T* d;
};

}  // namespace Digikam

#endif // DSHAREDDATA_H
