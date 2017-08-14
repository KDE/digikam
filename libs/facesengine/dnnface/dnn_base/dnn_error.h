/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2017-08-08
 * @brief   Errors for dnn module, can be used for face recognition, 
 *          all codes are ported from dlib library (http://dlib.net/)
 *
 * @section DESCRIPTION
 *
 * @author Copyright (C) 2017 by Yingjie Liu
 *         <a href="mailto:yingjiewudi at gmail dot com">yingjiewudi at gmail dot com</a>
 *
 * @section LICENSE
 *
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 *
 * ============================================================ */
#ifndef DLIB_ERROr_ 
#define DLIB_ERROr_ 

#include <string>
#include <new>          // for std::bad_alloc
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <exception>

// -------------------------------
// ------ exception classes ------
// -------------------------------



// ----------------------------------------------------------------------------------------

enum error_type
{       
    EPORT_IN_USE,  
    ETIMEOUT,     
    ECONNECTION, 
    ELISTENER, 
    ERESOLVE,     
    EMONITOR,   
    ECREATE_THREAD,    
    ECREATE_MUTEX,    
    ECREATE_SIGNALER,
    EUNSPECIFIED,   
    EGENERAL_TYPE1,
    EGENERAL_TYPE2,  
    EGENERAL_TYPE3,  
    EINVALID_OPTION,
    ETOO_FEW_ARGS,
    ETOO_MANY_ARGS,
    ESOCKET,
    ETHREAD,
    EGUI,
    EFATAL,
    EBROKEN_ASSERT,
    EIMAGE_LOAD,
    EDIR_CREATE,
    EINCOMPATIBLE_OPTIONS,
    EMISSING_REQUIRED_OPTION,
    EINVALID_OPTION_ARG,
    EMULTIPLE_OCCURANCES,
    ECONFIG_READER,
    EIMAGE_SAVE,
    ECAST_TO_STRING,
    ESTRING_CAST,
    EUTF8_TO_UTF32,
    EOPTION_PARSE
};

// ----------------------------------------------------------------------------------------

// the base exception class
class error : public std::exception
{
    /*!
        WHAT THIS OBJECT REPRESENTS
            This is the base exception class for the dlib library.  i.e. all 
            exceptions in this library inherit from this class.
    !*/

public:
    error(
        error_type t,
        const std::string& a
    ): info(a), type(t) {}
    /*!
        ensures
            - #type == t
            - #info == a
    !*/

    error(
        error_type t
    ): type(t) {}
    /*!
        ensures
            - #type == t
            - #info == ""
    !*/

    error(
        const std::string& a
    ): info(a), type(EUNSPECIFIED) {}
    /*!
        ensures
            - #type == EUNSPECIFIED
            - #info == a
    !*/

    error(
    ): type(EUNSPECIFIED) {}
    /*!
        ensures
            - #type == EUNSPECIFIED
            - #info == ""
    !*/

    virtual ~error(
    ) throw() {}
    /*!
        ensures
            - does nothing
    !*/

    const char* what(
    ) const throw()
    /*!
        ensures
            - if (info.size() != 0) then
                - returns info.c_str()
            - else
                - returns type_to_string(type)
    !*/
    {
        if (info.size() > 0)
            return info.c_str(); 
        else
            return type_to_string();
    }

    const char* type_to_string (
    ) const throw()
    /*!
        ensures
            - returns a string that names the contents of the type member.
    !*/
    {
        if ( type == EPORT_IN_USE) return "EPORT_IN_USE";
        else if ( type == ETIMEOUT) return "ETIMEOUT";
        else if ( type == ECONNECTION) return "ECONNECTION"; 
        else if ( type == ELISTENER) return "ELISTENER"; 
        else if ( type == ERESOLVE) return "ERESOLVE";     
        else if ( type == EMONITOR) return "EMONITOR";   
        else if ( type == ECREATE_THREAD) return "ECREATE_THREAD";    
        else if ( type == ECREATE_MUTEX) return "ECREATE_MUTEX";    
        else if ( type == ECREATE_SIGNALER) return "ECREATE_SIGNALER";
        else if ( type == EUNSPECIFIED) return "EUNSPECIFIED";   
        else if ( type == EGENERAL_TYPE1) return "EGENERAL_TYPE1";
        else if ( type == EGENERAL_TYPE2) return "EGENERAL_TYPE2";  
        else if ( type == EGENERAL_TYPE3) return "EGENERAL_TYPE3";  
        else if ( type == EINVALID_OPTION) return "EINVALID_OPTION";
        else if ( type == ETOO_FEW_ARGS) return "ETOO_FEW_ARGS";
        else if ( type == ETOO_MANY_ARGS) return "ETOO_MANY_ARGS";
        else if ( type == ESOCKET) return "ESOCKET";
        else if ( type == ETHREAD) return "ETHREAD";
        else if ( type == EGUI) return "EGUI";
        else if ( type == EFATAL) return "EFATAL";
        else if ( type == EBROKEN_ASSERT) return "EBROKEN_ASSERT";
        else if ( type == EIMAGE_LOAD) return "EIMAGE_LOAD";
        else if ( type == EDIR_CREATE) return "EDIR_CREATE";
        else if ( type == EINCOMPATIBLE_OPTIONS) return "EINCOMPATIBLE_OPTIONS";
        else if ( type == EMISSING_REQUIRED_OPTION) return "EMISSING_REQUIRED_OPTION";
        else if ( type == EINVALID_OPTION_ARG) return "EINVALID_OPTION_ARG";
        else if ( type == EMULTIPLE_OCCURANCES) return "EMULTIPLE_OCCURANCES";
        else if ( type == ECONFIG_READER) return "ECONFIG_READER";
        else if ( type == EIMAGE_SAVE) return "EIMAGE_SAVE";
        else if ( type == ECAST_TO_STRING) return "ECAST_TO_STRING";
        else if ( type == ESTRING_CAST) return "ESTRING_CAST";
        else if ( type == EUTF8_TO_UTF32) return "EUTF8_TO_UTF32";
        else if ( type == EOPTION_PARSE) return "EOPTION_PARSE";
        else return "undefined error type";
    }

    const std::string info;  // info about the error
    const error_type type; // the type of the error

private:
    const error& operator=(const error&);
};

// ----------------------------------------------------------------------------------------

class fatal_error : public error
{
    /*!
        WHAT THIS OBJECT REPRESENTS
            As the name says, this object represents some kind of fatal error.  
            That is, it represents an unrecoverable error and any program that
            throws this exception is, by definition, buggy and needs to be fixed.

            Note that a fatal_error exception can only be thrown once.  The second
            time an application attempts to construct a fatal_error it will be 
            immediately aborted and an error message will be printed to std::cerr. 
            The reason for this is because the first fatal_error was apparently ignored
            so the second fatal_error is going to make itself impossible to ignore 
            by calling abort.  The lesson here is that you should not try to ignore 
            fatal errors.

            This is also the exception thrown by the DLIB_ASSERT and DLIB_CASSERT macros.
    !*/

public:
    fatal_error(
        error_type t,
        const std::string& a
    ): error(t,a) {check_for_previous_fatal_errors();}
    /*!
        ensures
            - #type == t
            - #info == a
    !*/

    fatal_error(
        error_type t
    ): error(t) {check_for_previous_fatal_errors();}
    /*!
        ensures
            - #type == t
            - #info == ""
    !*/

    fatal_error(
        const std::string& a
    ): error(EFATAL,a) {check_for_previous_fatal_errors();}
    /*!
        ensures
            - #type == EFATAL
            - #info == a
    !*/

    fatal_error(
    ): error(EFATAL) {check_for_previous_fatal_errors();}
    /*!
        ensures
            - #type == EFATAL
            - #info == ""
    !*/

private:

    static inline char* message ()
    { 
        static char buf[2000];
        buf[1999] = '\0'; // just to be extra safe
        return buf;
    }

    static inline void dlib_fatal_error_terminate (
    )
    {
        std::cerr << "\n**************************** FATAL ERROR DETECTED ****************************";
        std::cerr << message() << std::endl;
        std::cerr << "******************************************************************************\n" << std::endl;
    }

    void check_for_previous_fatal_errors()
    {
        // If dlib is being use to create plugins for some other application, like
        // MATLAB, then don't do these checks since it terminates the over arching
        // system.  Just let the errors go to the plugin handler and it will deal with
        // them.
#if defined(MATLAB_MEX_FILE)
        return;
#else
        static bool is_first_fatal_error = true;
        if (is_first_fatal_error == false)
        {
            std::cerr << "\n\n ************************** FATAL ERROR DETECTED ************************** " << std::endl;
            std::cerr << " ************************** FATAL ERROR DETECTED ************************** " << std::endl;
            std::cerr << " ************************** FATAL ERROR DETECTED ************************** \n" << std::endl;
            std::cerr << "Two fatal errors have been detected, the first was inappropriately ignored. \n"
                      << "To prevent further fatal errors from being ignored this application will be \n"
                      << "terminated immediately and you should go fix this buggy program.\n\n"
                      << "The error message from this fatal error was:\n" << this->what() << "\n\n" << std::endl;
            using namespace std;
            assert(false);
            abort();
        }
        else
        {
            // copy the message into the fixed message buffer so that it can be recalled by dlib_fatal_error_terminate
            // if needed.
            char* msg = message();
            unsigned long i;
            for (i = 0; i < 2000-1 && i < this->info.size(); ++i)
                msg[i] = info[i];
            msg[i] = '\0';

            // set this termination handler so that if the user doesn't catch this fatal_error that is being
            // thrown then it will eventually be printed to standard error
            std::set_terminate(&dlib_fatal_error_terminate);
        }
        is_first_fatal_error = false;
#endif
    }
};

// ----------------------------------------------------------------------------------------

class gui_error : public error
{
public:
    gui_error(
        error_type t,
        const std::string& a
    ): error(t,a) {}
    /*!
        ensures
            - #type == t
            - #info == a
    !*/

    gui_error(
        error_type t
    ): error(t) {}
    /*!
        ensures
            - #type == t
            - #info == ""
    !*/

    gui_error(
        const std::string& a
    ): error(EGUI,a) {}
    /*!
        ensures
            - #type == EGUI 
            - #info == a
    !*/

    gui_error(
    ): error(EGUI) {}
    /*!
        ensures
            - #type == EGUI
            - #info == ""
    !*/
};

// ----------------------------------------------------------------------------------------

class socket_error : public error
{
public:
    socket_error(
        error_type t,
        const std::string& a
    ): error(t,a) {}
    /*!
        ensures
            - #type == t
            - #info == a
    !*/

    socket_error(
        error_type t
    ): error(t) {}
    /*!
        ensures
            - #type == t
            - #info == ""
    !*/

    socket_error(
        const std::string& a
    ): error(ESOCKET,a) {}
    /*!
        ensures
            - #type == ESOCKET
            - #info == a
    !*/

    socket_error(
    ): error(ESOCKET) {}
    /*!
        ensures
            - #type == ESOCKET
            - #info == ""
    !*/
};

// ----------------------------------------------------------------------------------------

class thread_error : public error
{
public:
    thread_error(
        error_type t,
        const std::string& a
    ): error(t,a) {}
    /*!
        ensures
            - #type == t
            - #info == a
    !*/

    thread_error(
        error_type t
    ): error(t) {}
    /*!
        ensures
            - #type == t
            - #info == ""
    !*/

    thread_error(
        const std::string& a
    ): error(ETHREAD,a) {}
    /*!
        ensures
            - #type == ETHREAD
            - #info == a
    !*/

    thread_error(
    ): error(ETHREAD) {}
    /*!
        ensures
            - #type == ETHREAD
            - #info == ""
    !*/
};

// ----------------------------------------------------------------------------------------

class impossible_labeling_error : public error
{ 
    /*!
        WHAT THIS OBJECT REPRESENTS
            This is the exception thrown by code that trains object detectors (e.g.
            structural_svm_object_detection_problem) when they detect that the set of
            truth boxes given to the training algorithm contains some impossible to
            obtain outputs.  
            
            This kind of problem can happen when the set of image positions scanned by
            the underlying object detection method doesn't include the truth rectangle
            as a possible output.  Another possibility is when two truth boxes are very
            close together and hard coded non-max suppression logic would prevent two
            boxes in such close proximity from being output.
    !*/
public: 
    impossible_labeling_error(const std::string& msg) : error(msg) {};
};

// ----------------------------------------------------------------------------------------



#endif // DLIB_ERROr_

