/*
 * SPDX-License-Identifier: MIT
 *
 * Symbol-visibility macros. Public symbols are tagged with HELLO_API so that
 * shared-library builds can hide everything else with -fvisibility=hidden.
 */
#ifndef HELLO_EXPORT_H
#define HELLO_EXPORT_H

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(HELLO_BUILD_SHARED)
#    define HELLO_API __declspec(dllexport)
#  elif defined(HELLO_USE_SHARED)
#    define HELLO_API __declspec(dllimport)
#  else
#    define HELLO_API
#  endif
#else
#  if defined(HELLO_BUILD_SHARED) && (defined(__GNUC__) || defined(__clang__))
#    define HELLO_API __attribute__((visibility("default")))
#  else
#    define HELLO_API
#  endif
#endif

#endif /* HELLO_EXPORT_H */
