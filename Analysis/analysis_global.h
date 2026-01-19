#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(ANALYSIS_LIB)
#  define ANALYSIS_EXPORT Q_DECL_EXPORT
# else
#  define ANALYSIS_EXPORT Q_DECL_IMPORT
# endif
#else
# define ANALYSIS_EXPORT
#endif
