/* pico2play.c
*
* Modication of pico2wave.c
* Modifications copyright (C) 2016 Smanar https://github.com/Smanar/Pico2play
*
* This version play sound directly, without using wav file.
*
*
*
* Copyright (C) 2009 Mathieu Parent <math.parent@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*   Convert text to .wav using svox text-to-speech system.
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef _WIN32
	#include <unistd.h>
#endif


//#define LOCAL
#ifdef LOCAL
	#include "../lib/picoapi.h"
	#include "../lib/picoapid.h"
	#include "../lib/picoos.h"

	#include "../portaudio/portaudio.h"
#else
	#include <picoapi.h>
	#include <picoapid.h>
	#include <picoos.h>

	#include <portaudio.h>
#endif

/* adaptation layer defines */
#define PICO_MEM_SIZE       2500000
#define DummyLen 100000000

/* string constants */
#define MAX_OUTBUF_SIZE     128
#ifdef picolangdir
const char * PICO_LINGWARE_PATH             = picolangdir "/";
#else
const char * PICO_LINGWARE_PATH             = "./lang/";
#endif
const char * PICO_VOICE_NAME                = "PicoVoice";

/* supported voices
Pico does not seperately specify the voice and locale.   */
const char * picoSupportedLangIso3[]        = { "eng",              "eng",              "deu",              "spa",              "fra",              "ita" };
const char * picoSupportedCountryIso3[]     = { "USA",              "GBR",              "DEU",              "ESP",              "FRA",              "ITA" };
const char * picoSupportedLang[]            = { "en-US",            "en-GB",            "de-DE",            "es-ES",            "fr-FR",            "it-IT" };
const char * picoInternalLang[]             = { "en-US",            "en-GB",            "de-DE",            "es-ES",            "fr-FR",            "it-IT" };
const char * picoInternalTaLingware[]       = { "en-US_ta.bin",     "en-GB_ta.bin",     "de-DE_ta.bin",     "es-ES_ta.bin",     "fr-FR_ta.bin",     "it-IT_ta.bin" };
const char * picoInternalSgLingware[]       = { "en-US_lh0_sg.bin", "en-GB_kh0_sg.bin", "de-DE_gl0_sg.bin", "es-ES_zl0_sg.bin", "fr-FR_nk0_sg.bin", "it-IT_cm0_sg.bin" };
const char * picoInternalUtppLingware[]     = { "en-US_utpp.bin",   "en-GB_utpp.bin",   "de-DE_utpp.bin",   "es-ES_utpp.bin",   "fr-FR_utpp.bin",   "it-IT_utpp.bin" };
const int picoNumSupportedVocs              = 6;

/* adapation layer global variables */
void *          picoMemArea         = NULL;
pico_System     picoSystem          = NULL;
pico_Resource   picoTaResource      = NULL;
pico_Resource   picoSgResource      = NULL;
pico_Resource   picoUtppResource    = NULL;
pico_Engine     picoEngine          = NULL;
pico_Char *     picoTaFileName      = NULL;
pico_Char *     picoSgFileName      = NULL;
pico_Char *     picoUtppFileName    = NULL;
pico_Char *     picoTaResourceName  = NULL;
pico_Char *     picoSgResourceName  = NULL;
pico_Char *     picoUtppResourceName = NULL;
int     picoSynthAbort = 0;






int  PortAudioInitialised = 0;
int cPlay_paStreamCallback(const void *input, void *output,unsigned long frameCount,const PaStreamCallbackTimeInfo* timeInfo,PaStreamCallbackFlags statusFlags,void *userData);


PaStream* stream= NULL;
int numChannels = 1; // mono
int sampleRate = 16000; //16000 Hz
int bytesPerSample = 2; //16 bit unsigned

int bitsPerSample;
int start = 0;

typedef struct
{
	unsigned char* data;
	unsigned long cursor;
	unsigned long num_frames;
}
paTestData;

paTestData userData;



int InitPortAudio(void)
{
	int fd = 0;

	if (PortAudioInitialised == 1) return 1;


#ifndef _WIN32
	//disable output error
	fd = dup(fileno(stdout));
	freopen("/dev/null", "w", stderr);
#endif


	// Initializes PortAudio.
	PaError pa_init_ans = Pa_Initialize();

	if (pa_init_ans != paNoError) {
		printf("Fail to initialize PortAudio, error message is %s\n", Pa_GetErrorText(pa_init_ans));
		return 0;
	}

#if 0
	{
		int i;

		PaDeviceIndex numDevices = Pa_GetDeviceCount();
		if (numDevices < 0)
		{
			printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
		}

		printf("Number of devices = %d\n", numDevices);
		for (i = 0; i<numDevices; i++)
		{
			const PaDeviceInfo * deviceInfo = Pa_GetDeviceInfo(i);
			printf("[Device] %s", deviceInfo->name);

			if (i == Pa_GetDefaultInputDevice())
			{
				printf("[ Default Input ]");
			}
			if (i == Pa_GetDefaultOutputDevice())
			{
				printf("[ Default Output ]");
			}
			printf("\n");
		}
	}
#endif

	PortAudioInitialised = 1;

#ifndef _WIN32
	//clear and restore it
	fflush(stderr);
	dup2(fd, fileno(stderr));
#endif

	return 1;
}


int portAudioOpen() {

	PaStreamParameters outputParameters;

	outputParameters.device = Pa_GetDefaultOutputDevice();
	if (outputParameters.device == paNoDevice)
	{
		printf("No device found");
		exit(0);
	}

	outputParameters.channelCount = numChannels;
	outputParameters.sampleFormat = paInt16; //Signed 16 bit Little Endian
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;//->defaultHighOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	userData.cursor = 0;
	userData.num_frames = 0;
	userData.data = (unsigned char *)malloc(1);

	PaError ret = Pa_OpenStream(
		&stream,
		NULL, // no input
		&outputParameters,
		16000.0,
		paFramesPerBufferUnspecified, // framesPerBuffer
		0, // flags
		&cPlay_paStreamCallback,
		&userData
		);

	if (ret != paNoError) {
		printf("(SO) Pa_OpenStream failed: (err %i) %s\n", ret, Pa_GetErrorText(ret));
		if (stream)
			Pa_CloseStream(stream);
		return 0;
	}

	return 1;
}
/**********************************************************************************************************/

int cPlay_paStreamCallback(const void *input, void *output,unsigned long frameCount,const PaStreamCallbackTimeInfo* timeInfo,PaStreamCallbackFlags statusFlags,void *userData)
{
	paTestData *data_struct = (paTestData*)userData; 

	size_t numRead = 0;

	int typesize =  bytesPerSample * numChannels;
	unsigned long restant = data_struct->num_frames - data_struct->cursor/typesize;

	(void) input; /* Prevent unused variable warnings. */
	(void) timeInfo; /* Prevent unused variable warnings. */
	(void) statusFlags; /* Prevent unused variable warnings. */

	//printf("Current %ld Remaining %ld\n",data_struct->cursor, restant);

	//securitee
	//if ((data_struct->cursor + typesize * frameCount) > (data_struct->num_frames * typesize))
	//{
	//	printf(">> %ld %ld",(data_struct->cursor + typesize * frameCount) , (data_struct->num_frames * typesize));
	//	return paContinue;
	//}

	if (frameCount < restant)
	{
		memcpy(output,(void *)&data_struct->data[data_struct->cursor], typesize * frameCount);

		numRead = frameCount;
		data_struct->cursor += typesize * frameCount;

	}
	else
	{
		memcpy(output,(void *)&data_struct->data[data_struct->cursor],restant);
		numRead = restant / typesize;
		data_struct->cursor += restant;
	}

	frameCount -= numRead;

	if (frameCount > 0) {
		memset((char *)output + numRead * typesize , 0, frameCount * typesize);
		return paComplete;
	}

	return paContinue;
}



static void usage(void)
{
    fprintf(stderr,"\nUsage:\n\n" \
           "pico2play [-l language] \"Text to speak\"\n\n");
    exit(0);
}



int main(int argc, char *argv[]) {

	char * lang = "en-US";
	int langIndex = -1, langIndexTmp = -1;
	char * text = NULL;
	int8_t * buffer;
	size_t bufferSize = 256;

	//For command line
	int currentOption;


	InitPortAudio();

	if (portAudioOpen() == 0)
	{
		printf("Can't initialise sound output\n");
		exit(0);
	}


	//Command line parser
	while ( (currentOption = getopt(argc, argv, "l:")) != -1)
    {
        switch (currentOption)
        {
        case 'l':
            lang = optarg;
            break;
        default:
            usage();
        }
	}

	if (optind < argc)
        text = argv[optind];

    if (!text)
    {
        fprintf(stderr, "Error: no input string\n");
        usage();
	}


	/* Language selection */
	for(langIndexTmp =0; langIndexTmp<picoNumSupportedVocs; langIndexTmp++) {
		if(!strcmp(picoSupportedLang[langIndexTmp], lang)) {
			langIndex = langIndexTmp;
			break;
		}
	}
	if(langIndex == -1) {
		fprintf(stderr, "Unknown language: %s\nValid languages:\n", lang);
		for(langIndexTmp =0; langIndexTmp<picoNumSupportedVocs; langIndexTmp++) {
			fprintf(stderr, "%s\n", picoSupportedLang[langIndexTmp]);
		}
		lang = "en-US";
		fprintf(stderr, "\n");
		exit(1);
	}


	buffer = (int8_t *)malloc( bufferSize );

	int ret, getstatus;
	pico_Char * inp = NULL;
	pico_Char * local_text = NULL;
	short       outbuf[MAX_OUTBUF_SIZE/2];
	pico_Int16  bytes_sent, bytes_recv, text_remaining, out_data_type;
	pico_Retstring outMessage;

	picoSynthAbort = 0;

	picoMemArea = malloc( PICO_MEM_SIZE );
	if((ret = pico_initialize( picoMemArea, PICO_MEM_SIZE, &picoSystem ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot initialize pico (%i): %s\n", ret, outMessage);
		goto terminate;
	}

	/* Load the text analysis Lingware resource file.   */
	picoTaFileName      = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );
	strcpy((char *) picoTaFileName,   PICO_LINGWARE_PATH);
	strcat((char *) picoTaFileName,   (const char *) picoInternalTaLingware[langIndex]);
	if((ret = pico_loadResource( picoSystem, picoTaFileName, &picoTaResource ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot load text analysis resource file (%i): %s\n", ret, outMessage);
		goto unloadTaResource;
	}

	/* Load the signal generation Lingware resource file.   */
	picoSgFileName      = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );
	strcpy((char *) picoSgFileName,   PICO_LINGWARE_PATH);
	strcat((char *) picoSgFileName,   (const char *) picoInternalSgLingware[langIndex]);
	if((ret = pico_loadResource( picoSystem, picoSgFileName, &picoSgResource ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot load signal generation Lingware resource file (%i): %s\n", ret, outMessage);
		goto unloadSgResource;
	}

	/* Load the utpp Lingware resource file if exists - NOTE: this file is optional
	and is currently not used. Loading is only attempted for future compatibility.
	If this file is not present the loading will still succeed.                      //
	picoUtppFileName      = (pico_Char *) malloc( PICO_MAX_DATAPATH_NAME_SIZE + PICO_MAX_FILE_NAME_SIZE );
	strcpy((char *) picoUtppFileName,   PICO_LINGWARE_PATH);
	strcat((char *) picoUtppFileName,   (const char *) picoInternalUtppLingware[langIndex]);
	ret = pico_loadResource( picoSystem, picoUtppFileName, &picoUtppResource );
	pico_getSystemStatusMessage(picoSystem, ret, outMessage);
	printf("pico_loadResource: %i: %s\n", ret, outMessage);
	*/

	/* Get the text analysis resource name.     */
	picoTaResourceName  = (pico_Char *) malloc( PICO_MAX_RESOURCE_NAME_SIZE );
	if((ret = pico_getResourceName( picoSystem, picoTaResource, (char *) picoTaResourceName ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot get the text analysis resource name (%i): %s\n", ret, outMessage);
		goto unloadUtppResource;
	}

	/* Get the signal generation resource name. */
	picoSgResourceName  = (pico_Char *) malloc( PICO_MAX_RESOURCE_NAME_SIZE );
	if((ret = pico_getResourceName( picoSystem, picoSgResource, (char *) picoSgResourceName ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot get the signal generation resource name (%i): %s\n", ret, outMessage);
		goto unloadUtppResource;
	}


	/* Create a voice definition.   */
	if((ret = pico_createVoiceDefinition( picoSystem, (const pico_Char *) PICO_VOICE_NAME ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot create voice definition (%i): %s\n", ret, outMessage);
		goto unloadUtppResource;
	}

	/* Add the text analysis resource to the voice. */
	if((ret = pico_addResourceToVoiceDefinition( picoSystem, (const pico_Char *) PICO_VOICE_NAME, picoTaResourceName ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot add the text analysis resource to the voice (%i): %s\n", ret, outMessage);
		goto unloadUtppResource;
	}

	/* Add the signal generation resource to the voice. */
	if((ret = pico_addResourceToVoiceDefinition( picoSystem, (const pico_Char *) PICO_VOICE_NAME, picoSgResourceName ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot add the signal generation resource to the voice (%i): %s\n", ret, outMessage);
		goto unloadUtppResource;
	}

	/* Create a new Pico engine. */
	if((ret = pico_newEngine( picoSystem, (const pico_Char *) PICO_VOICE_NAME, &picoEngine ))) {
		pico_getSystemStatusMessage(picoSystem, ret, outMessage);
		fprintf(stderr, "Cannot create a new pico engine (%i): %s\n", ret, outMessage);
		goto disposeEngine;
	}

	local_text = (pico_Char *) text ;
	text_remaining = strlen((const char *) local_text) + 1;

	inp = (pico_Char *) local_text;

	size_t bufused = 0;

	/* synthesis loop   */
	while (text_remaining) {
		/* Feed the text into the engine.   */
		if((ret = pico_putTextUtf8( picoEngine, inp, text_remaining, &bytes_sent )))
		{
			pico_getSystemStatusMessage(picoSystem, ret, outMessage);
			fprintf(stderr, "Cannot put Text (%i): %s\n", ret, outMessage);
			goto disposeEngine;
		}

		text_remaining -= bytes_sent;
		inp += bytes_sent;

		do {
			if (picoSynthAbort)
			{
				goto disposeEngine;
			}
			/* Retrieve the samples and add them to the buffer. */
			getstatus = pico_getData( picoEngine, (void *) outbuf, MAX_OUTBUF_SIZE, &bytes_recv, &out_data_type );
			if((getstatus !=PICO_STEP_BUSY) && (getstatus !=PICO_STEP_IDLE))
			{
				pico_getSystemStatusMessage(picoSystem, getstatus, outMessage);
				fprintf(stderr, "Cannot get Data (%i): %s\n", getstatus, outMessage);
				goto disposeEngine;
			}
			if (bytes_recv)
			{
				if ((bufused + bytes_recv) <= bufferSize)
				{
					memcpy(buffer+bufused, (int8_t *) outbuf, bytes_recv);
					bufused += bytes_recv;
				}
				else
				{
					//copy data to buffer
					userData.data = (unsigned char *)realloc(userData.data,userData.num_frames * (bytesPerSample * numChannels) + bufused);
					memcpy(userData.data + userData.num_frames * (bytesPerSample * numChannels),buffer,bufused );
					userData.num_frames = userData.num_frames + ( bufused /  (bytesPerSample * numChannels) ) ;


					bufused = 0;
					memcpy(buffer, (int8_t *) outbuf, bytes_recv);
					bufused += bytes_recv;

					start++;

					if (start == 2)
					{
						if (Pa_StartStream(stream) != paNoError)
						{
							printf("can't start sound");
							exit(0);
						}
						start ++;
					}
				}
			}
		} while (PICO_STEP_BUSY == getstatus);

		/* This chunk of synthesis is finished; pass the remaining samples. */
		if (!picoSynthAbort)
		{

			//copy data to buffer
			userData.data = (unsigned char *)realloc(userData.data,userData.num_frames * (bytesPerSample * numChannels) + bufused);
			memcpy(userData.data + userData.num_frames * (bytesPerSample * numChannels),buffer,bufused );
			userData.num_frames = userData.num_frames + ( bufused /  (bytesPerSample * numChannels) ) ;

		}
		picoSynthAbort = 0;
	}

disposeEngine:
	if (picoEngine) {
		pico_disposeEngine( picoSystem, &picoEngine );
		pico_releaseVoiceDefinition( picoSystem, (pico_Char *) PICO_VOICE_NAME );
		picoEngine = NULL;
	}
unloadUtppResource:
	if (picoUtppResource) {
		pico_unloadResource( picoSystem, &picoUtppResource );
		picoUtppResource = NULL;
	}
unloadSgResource:
	if (picoSgResource) {
		pico_unloadResource( picoSystem, &picoSgResource );
		picoSgResource = NULL;
	}
unloadTaResource:
	if (picoTaResource) {
		pico_unloadResource( picoSystem, &picoTaResource );
		picoTaResource = NULL;
	}
terminate:
	if (picoSystem) {
		pico_terminate(&picoSystem);
		picoSystem = NULL;
	}


	//printf("Nbre de frames %ld\n",userData.num_frames);

	//CHECK(Pa_StartStream(stream) == paNoError);
	//printf("Debut de l'attente\n");

	// wait until stream has finished playing
	while (Pa_IsStreamActive(stream) > 0)
	{
		Pa_Sleep(100);
	}

	free(userData.data);

	if (stream)
	{
		Pa_StopStream(stream);
		Pa_CloseStream(stream);
	}

	exit(ret);
}

