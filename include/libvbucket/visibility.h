/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#ifndef LIBVBUCKET_VISIBILITY_H
#define LIBVBUCKET_VISIBILITY_H 1

#if defined (__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#define LIBVBUCKET_PUBLIC_API __global
#elif defined __GNUC__
#define LIBVBUCKET_PUBLIC_API __attribute__ ((visibility("default")))
#else
#define LIBVBUCKET_PUBLIC_API
#endif

#endif /* LIBVBUCKET_VISIBILITY_H */
