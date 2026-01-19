#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(DOC_LIB)
#  define DOC_EXPORT Q_DECL_EXPORT
# else
#  define DOC_EXPORT Q_DECL_IMPORT
# endif
#else
# define DOC_EXPORT
#endif
