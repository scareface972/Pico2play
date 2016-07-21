#ifdef _WIN32
/*
 * $Id: pa_win_hostapis.c 1728 2011-08-18 03:31:51Z rossb $
 * Portable Audio I/O Library Windows initialization table
 *
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 1999-2008 Ross Bencina, Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */

/** @file
 @ingroup win_src

    @brief Win32 host API initialization function table.
*/

/* This is needed to make this source file depend on CMake option changes
   and at the same time make it transparent for clients not using CMake.
*/
#ifdef PORTAUDIO_CMAKE_GENERATED
#include "options_cmake.h"
#endif

#include "pa_hostapi.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

PaError PaSkeleton_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaWinMme_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaWinDs_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaAsio_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaWinWdm_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaWasapi_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );

#ifdef __cplusplus
}
#endif /* __cplusplus */


PaUtilHostApiInitializer *paHostApiInitializers[] =
    {

#if PA_USE_WMME
        PaWinMme_Initialize,
#endif

#if PA_USE_DS
        PaWinDs_Initialize,
#endif

#if PA_USE_ASIO
        PaAsio_Initialize,
#endif

#if PA_USE_WASAPI
		PaWasapi_Initialize,
#endif

#if PA_USE_WDMKS
        PaWinWdm_Initialize,
#endif

#if PA_USE_SKELETON
        PaSkeleton_Initialize, /* just for testing. last in list so it isn't marked as default. */
#endif

        0   /* NULL terminated array */
    };


#else

/*
 * $Id: pa_unix_hostapis.c 1740 2011-08-25 07:17:48Z philburk $
 * Portable Audio I/O Library UNIX initialization table
 *
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 1999-2002 Ross Bencina, Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */

/** @file
 @ingroup unix_src
*/

#include "pa_hostapi.h"

PaError PaJack_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaAlsa_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaOSS_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
/* Added for IRIX, Pieter, oct 2, 2003: */
PaError PaSGI_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
/* Linux AudioScience HPI */
PaError PaAsiHpi_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaMacCore_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );
PaError PaSkeleton_Initialize( PaUtilHostApiRepresentation **hostApi, PaHostApiIndex index );

/** Note that on Linux, ALSA is placed before OSS so that the former is preferred over the latter.
 */

PaUtilHostApiInitializer *paHostApiInitializers[] =
    {
#ifdef __linux__

#if PA_USE_ALSA
        PaAlsa_Initialize,
#endif

#if PA_USE_OSS
        PaOSS_Initialize,
#endif

#else   /* __linux__ */

#if PA_USE_OSS
        PaOSS_Initialize,
#endif

#if PA_USE_ALSA
        PaAlsa_Initialize,
#endif

#endif  /* __linux__ */

#if PA_USE_JACK
        PaJack_Initialize,
#endif
                    /* Added for IRIX, Pieter, oct 2, 2003: */
#if PA_USE_SGI 
        PaSGI_Initialize,
#endif

#if PA_USE_ASIHPI
        PaAsiHpi_Initialize,
#endif

#if PA_USE_COREAUDIO
        PaMacCore_Initialize,
#endif

#if PA_USE_SKELETON
        PaSkeleton_Initialize,
#endif

        0   /* NULL terminated array */
    };


#endif