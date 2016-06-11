/*****************************************************************************/
// Copyright 2006 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_auto_ptr.h#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/** \file
 * Class to implement std::auto_ptr like functionality even on platforms which do not have a full Standard C++ library.
 */

/*****************************************************************************/

#ifndef __dng_auto_ptr__
#define __dng_auto_ptr__

/*****************************************************************************/

// The following template has similar functionality to the STL auto_ptr,
// without requiring all the weight of STL.

/*****************************************************************************/

/// \brief A class intended to be used in stack scope to hold a pointer from new. The held pointer will be deleted automatically if the scope is left without calling Release on the AutoPtr first.

template<class T>
class AutoPtr
	{

	private:

		T *p_;

	public:

		/// Construct an AutoPtr with no referent.

		AutoPtr () : p_ (0) { }

		/// Construct an AutoPtr which owns the argument pointer.
		/// \param p pointer which constructed AutoPtr takes ownership of. p will be deleted on destruction or Reset unless Release is called first.

		explicit AutoPtr (T *p) :  p_( p ) { }

		/// Reset() is called on destruction.

		~AutoPtr ();

		/// Call Reset with a pointer from new. Uses T's default constructor.

		void Alloc ();

		/// Return the owned pointer of this AutoPtr, NULL if none. No change in ownership or other effects occur.

		T *Get () const { return p_; }

		/// Return the owned pointer of this AutoPtr, NULL if none. The AutoPtr gives up ownership and takes NULL as its value.

		T *Release ();

		/// If a pointer is owned, it is deleted. Ownership is taken of passed in pointer.

		void Reset (T *p);

		/// If a pointer is owned, it is deleted and the AutoPtr takes NULL as its value.

		void Reset ();

		/// Allows members of the owned pointer to be accessed directly. It is an error to call this if the AutoPtr has NULL as its value.

		T *operator-> () const { return p_; }

		/// Returns a reference to the object that the owned pointer points to.  It is an error to call this if the AutoPtr has NULL as its value.

		T &operator* () const { return *p_; }

	private:

		// Hidden copy constructor and assignment operator.  I don't
		// think the STL "feature" of grabbing ownership of the pointer
		// is a good idea.

		AutoPtr (AutoPtr<T> &rhs);

		AutoPtr<T> & operator= (AutoPtr<T> &rhs);

	};

/*****************************************************************************/

template<class T>
AutoPtr<T>::~AutoPtr ()
	{

	delete p_;
	p_ = 0;

	}

/*****************************************************************************/

template<class T>
T *AutoPtr<T>::Release ()
	{
	T *result = p_;
	p_ = 0;
	return result;
	}

/*****************************************************************************/

template<class T>
void AutoPtr<T>::Reset (T *p)
	{

	if (p_ != p)
		{
		if (p_ != 0)
			delete p_;
		p_ = p;
		}

	}

/*****************************************************************************/

template<class T>
void AutoPtr<T>::Reset ()
	{

	if (p_ != 0)
		{
		delete p_;
		p_ = 0;
		}

	}

/*****************************************************************************/

template<class T>
void AutoPtr<T>::Alloc ()
	{
	this->Reset (new T);
	}

/*****************************************************************************/

#endif

/*****************************************************************************/
