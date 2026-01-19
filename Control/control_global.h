#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(CONTROL_LIB)
#  define CONTROL_EXPORT Q_DECL_EXPORT
# else
#  define CONTROL_EXPORT Q_DECL_IMPORT
# endif
#else
# define CONTROL_EXPORT
#endif
