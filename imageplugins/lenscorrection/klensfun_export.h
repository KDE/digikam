
#ifndef LIBKLENSFUN_EXPORT_H
#define LIBKLENSFUN_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KLENSFUN_EXPORT
# if defined(MAKE_KLENSFUN_LIB)
   /* We are building this library */
#  define KLENSFUN_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KLENSFUN_EXPORT KDE_IMPORT
# endif
#endif

#endif

