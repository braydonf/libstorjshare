/**
 * @file storjshare.h
 * @brief Storjshare library.
 *
 * Implements functionality to host files from the Storj network.
 */

#ifndef STORJSHARE_H
#define STORJSHARE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && defined(STORJDLL)
  #if defined(DLL_EXPORT)
    #define STORJ_API __declspec(dllexport)
  #else
    #define STORJ_API __declspec(dllimport)
  #endif
#else
  #define STORJ_API
#endif

#include <assert.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <curl/curl.h>

#include <inttypes.h>

#ifdef __cplusplus
}
#endif

#endif /* STORJSHARE_H */
