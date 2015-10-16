/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_loc.h -- private sound functions


#ifndef SND_LOC_H
#define SND_LOC_H


#include "../qcommon/qcommon.h"

#include "client.h"


typedef struct sfx_s sfx_t;


#ifdef USE_OPENAL

#if defined(_WIN32)
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#if ! (defined(_WIN32) || defined(EMSCRIPTEN))
#include <AL/alext.h>
#endif


qboolean	QAL_Init ();
void		QAL_Shutdown (void);


typedef struct {
	ALCdevice	*hDevice;
	ALCcontext	*hALC;
} alState_t;

extern alState_t	alState;

extern qboolean			openal_active;

qboolean	AL_Init (void);
void		AL_Shutdown (void);


byte *S_Alloc (int size);


qboolean S_OpenAL_LoadSound (sfx_t *sfx);
extern cvar_t	*s_openal_device;

//OpenAL
/*#define	MAX_OPENAL_BUFFERS 1024
#define MAX_OPENAL_SOURCES 128

#define OPENAL_SCALE_VALUE 0.010f

qboolean OpenAL_Init (void);

#ifdef USE_OPENAL
extern uint32 openAlMaxSources;
extern uint32 openAlMaxBuffers;

qboolean AL_Attenuated (int i);
void OpenAL_Shutdown (void);

void OpenAL_DestroyBuffers (void);
ALint OpenAL_GetFreeBuffer (void);
ALint OpenAL_GetFreeSource (void);

void OpenAL_FreeAlIndexes (int index);
int OpenAL_GetFreeAlIndex (void);

void OpenAL_CheckForError (void);

typedef struct OpenALBuffer_s
{
	ALuint buffer;
	ALboolean inuse;
} OpenALBuffer_t;

typedef struct alindex_s
{
	qboolean	inuse;
	qboolean	loopsound;
	qboolean	fixed_origin;
	vec3_t		origin;
	int			entnum;
	int			sourceIndex;
	int			lastloopframe;
	float		attenuation;
	char		soundname[MAX_QPATH];
} alindex_t;

extern OpenALBuffer_t	g_Buffers[MAX_OPENAL_BUFFERS];
extern ALuint			g_Sources[MAX_OPENAL_SOURCES];
extern alindex_t		alindex[MAX_OPENAL_SOURCES];*/

extern	cvar_t	*s_openal_extensions;
extern	cvar_t	*s_openal_eax;


#endif  // USE_OPENAL


typedef struct
{
	int 		length;
	int 		loopstart;
	int 		speed;			// not needed, because converted on load?
	int 		width;
	int 		stereo;
	byte		data[1];		// variable sized
} sfxcache_t;

struct sfx_s
{
	char 		name[MAX_QPATH];
	int			registration_sequence;
	sfxcache_t	*cache;
	char 		*truename;

	qboolean			loaded;
	int					samples;
	int					rate;
	unsigned			format;
	unsigned			bufferNum;
};


typedef struct
{
	int					rate;
	int					width;
	int					channels;
	int					samples;
} wavInfo_t;

typedef struct openal_sfx_s
{
	char				name[MAX_QPATH];
	qboolean			defaulted;
	qboolean			loaded;

	int					samples;
	int					rate;
	unsigned			format;
	unsigned			bufferNum;

	struct openal_sfx_s		*nextHash;
} openal_sfx_t;

/*typedef struct {
	char				introName[MAX_QPATH];
	char				loopName[MAX_QPATH];
	qboolean			looping;
	fileHandle_t		file;
	int					start;
	int					rate;
	unsigned			format;
	void				*vorbisFile;
} bgTrack_t;*/

// A playSound will be generated by each call to S_StartSound.
// When the mixer reaches playSound->beginTime, the playSound will be
// assigned to a channel.
/*typedef struct openal_playSound_s
{
	struct openal_playSound_s	*prev, *next;
	openal_sfx_t		*asfx;
	int					entNum;
	int					entChannel;
	qboolean			fixedPosition;	// Use position instead of fetching entity's origin
	vec3_t				position;		// Only use if fixedPosition is set
	float				volume;
	float				attenuation;
	int					beginTime;		// Begin at this time
} openal_playSound_t;*/

typedef struct
{
	qboolean			streaming;
	sfx_t				*sfx;			// NULL if unused
	//openal_sfx_t		*asfx;			// NULL if unused
	int					entNum;			// To allow overriding a specific sound
	int					entChannel;
	int					startTime;		// For overriding oldest sounds
	qboolean			loopSound;		// Looping sound
	int					loopNum;		// Looping entity number
	int					loopFrame;		// For stopping looping sounds
	qboolean			fixedPosition;	// Use position instead of fetching entity's origin
	vec3_t				position;		// Only use if fixedPosition is set
	float				volume;
	float				distanceMult;
	unsigned			sourceNum;		// OpenAL source
} openal_channel_t;

typedef struct
{
	vec3_t				position;
	vec3_t				velocity;
	float				orientation[6];
} openal_listener_t;


extern cvar_t	*snd_openal_extensions;
extern cvar_t	*snd_openal_eax;

/*
 =======================================================================

 IMPLEMENTATION SPECIFIC FUNCTIONS

 =======================================================================
*/

typedef struct
{
	const char			*vendorString;
	const char			*rendererString;
	const char			*versionString;
	const char			*extensionsString;

	const char			*deviceList;
	const char			*deviceName;

	qboolean			eax;
	unsigned			eaxState;
} alConfig_t;

extern alConfig_t		alConfig;


#define ALimp_Init						AL_Init
#define ALimp_Shutdown					AL_Shutdown


// !!! if this is changed, the asm code must change !!!
typedef struct
{
	int			left;
	int			right;
} portable_samplepair_t;


// a playsound_t will be generated by each call to S_StartSound,
// when the mixer reaches playsound->begin, the playsound will
// be assigned to a channel
typedef struct playsound_s
{
	struct playsound_s	*prev, *next;
	sfx_t		*sfx;
	float		volume;
	float		attenuation;
	int			entnum;
	int			entchannel;
	qboolean	fixed_origin;	// use origin field instead of entnum's origin
	vec3_t		origin;
	uint32		begin;			// begin on this sample
} playsound_t;

typedef struct
{
	int			channels;
	int			samples;				// mono samples in buffer
	int			submission_chunk;		// don't mix less than this #
	int			samplepos;				// in mono samples
	int			samplebits;
	int			speed;
	byte		*buffer;
} dma_t;

// !!! if this is changed, the asm code must change !!!
typedef struct
{
	sfx_t		*sfx;			// sfx number
	int			leftvol;		// 0-255 volume
	int			rightvol;		// 0-255 volume
	int			end;			// end time in global paintsamples
	int 		pos;			// sample position in sfx
	int			looping;		// where to loop, -1 = no looping OBSOLETE?
	int			entnum;			// to allow overriding a specific sound
	int			entchannel;		//
	vec3_t		origin;			// only use if fixed_origin is set
	vec_t		dist_mult;		// distance multiplier (attenuation/clipK)
	int			master_vol;		// 0-255 master volume
	qboolean	fixed_origin;	// use origin instead of fetching entnum's origin
	qboolean	autosound;		// from an entity->sound, cleared each frame
} channel_t;

typedef struct
{
	int			rate;
	int			width;
	int			channels;
	int			loopstart;
	int			samples;
	int			dataofs;		// chunk starts this many bytes from file start
} wavinfo_t;


/*
====================================================================

  SYSTEM SPECIFIC FUNCTIONS

====================================================================
*/

// initializes cycling through a DMA buffer and returns information on it
qboolean SNDDMA_Init(int fullInit);

// gets the current DMA position
int		SNDDMA_GetDMAPos(void);

// shutdown the DMA xfer.
void	SNDDMA_Shutdown(void);

void	SNDDMA_BeginPainting (void);

void	SNDDMA_Submit(void);

//====================================================================

#define	MAX_CHANNELS			32
extern	channel_t   channels[MAX_CHANNELS];

extern	int		paintedtime;
extern	int		s_rawend;
extern	vec3_t	listener_origin;
extern	vec3_t	listener_forward;
extern	vec3_t	listener_right;
extern	vec3_t	listener_up;
extern	dma_t	dma;
extern	playsound_t	s_pendingplays;

#define	MAX_RAW_SAMPLES	8192
extern	portable_samplepair_t	s_rawsamples[MAX_RAW_SAMPLES];

extern cvar_t	*s_volume;
extern cvar_t	*s_nosound;
extern cvar_t	*s_testsound;
extern cvar_t	*s_primary;

wavinfo_t GetWavinfo (char *name, byte *wav, int wavlength);

void S_InitScaletable (void);

sfxcache_t *S_LoadSound (sfx_t *s);

void S_IssuePlaysound (playsound_t *ps);

void S_PaintChannels(int endtime);

// picks a channel based on priorities, empty slots, number of channels
channel_t *S_PickChannel(int entnum, int entchannel);

// spatializes a channel
void S_Spatialize(channel_t *ch);

#define		SOUND_FULLVOLUME	80.0f
#define		SOUND_LOOPATTENUATE	0.003f


#endif  // SND_LOC_H
