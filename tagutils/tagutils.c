//=========================================================================
// FILENAME	: tagutils.c
// DESCRIPTION	: MP3/MP4/Ogg/FLAC metadata reader
//=========================================================================
// Copyright (c) 2008- NETGEAR, Inc. All Rights Reserved.
//=========================================================================

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* This file is derived from mt-daapd project */

#include <ctype.h>
#include <errno.h>
#include <id3tag.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <FLAC/metadata.h>

#include "config.h"
#ifdef HAVE_ICONV
#include <iconv.h>
#endif
#include <sqlite3.h>
#include "tagutils.h"
#include "metadata.h"
#include "utils.h"
#include "log.h"

/*
 * Prototype
 */
//#include "tagutils-misc.c"
#include "tagutils-mp3.h"
#include "tagutils-aac.h"
#include "tagutils-ogg.h"
#include "tagutils-flc.h"
#include "tagutils-asf.h"
#include "tagutils-wav.h"
#include "tagutils-pcm.h"
//#include "tagutils-plist.c"

static int _get_tags(char *file, struct song_metadata *psong);
static int _get_fileinfo(char *file, struct song_metadata *psong);


/*
 * Typedefs
 */

typedef struct {
	char* type;
	int (*get_tags)(char* file, struct song_metadata* psong);
	int (*get_fileinfo)(char* file, struct song_metadata* psong);
} taghandler;

static taghandler taghandlers[] = {
	{ "aac", _get_aactags, _get_aacfileinfo                                  },
	{ "mp3", _get_mp3tags, _get_mp3fileinfo                                  },
	{ "flc", _get_flctags, _get_flcfileinfo                                  },
	{ "ogg", 0,            _get_oggfileinfo                                  },
	{ "asf", 0,            _get_asffileinfo                                  },
	{ "wav", _get_wavtags, _get_wavfileinfo                                  },
	{ "pcm", 0,            _get_pcmfileinfo                                  },
	{ NULL,  0 }
};

//*********************************************************************************
// freetags()
#define MAYBEFREE(a) { if((a)) free((a)); };
void
freetags(struct song_metadata *psong)
{
	int role;

	MAYBEFREE(psong->path);
	MAYBEFREE(psong->image);
	MAYBEFREE(psong->title);
	MAYBEFREE(psong->album);
	MAYBEFREE(psong->genre);
	MAYBEFREE(psong->comment);
	for(role = ROLE_START; role <= ROLE_LAST; role++)
	{
		MAYBEFREE(psong->contributor[role]);
		MAYBEFREE(psong->contributor_sort[role]);
	}
	MAYBEFREE(psong->grouping);
	MAYBEFREE(psong->mime);
	MAYBEFREE(psong->dlna_pn);
	MAYBEFREE(psong->tagversion);
	MAYBEFREE(psong->musicbrainz_albumid);
	MAYBEFREE(psong->musicbrainz_trackid);
	MAYBEFREE(psong->musicbrainz_artistid);
	MAYBEFREE(psong->musicbrainz_albumartistid);
}

// _get_fileinfo
static int
_get_fileinfo(char *file, struct song_metadata *psong)
{
	taghandler *hdl;

	// dispatch to appropriate tag handler
	for(hdl = taghandlers; hdl->type; ++hdl)
		if(!strcmp(hdl->type, psong->type))
			break;

	if(hdl->get_fileinfo)
		return hdl->get_fileinfo(file, psong);

	return 0;
}


static void
_make_composite_tags(struct song_metadata *psong)
{
	int len;

	len = 1;

	if(!psong->contributor[ROLE_ARTIST] &&
	   (psong->contributor[ROLE_BAND] || psong->contributor[ROLE_CONDUCTOR]))
	{
		if(psong->contributor[ROLE_BAND])
			len += strlen(psong->contributor[ROLE_BAND]);
		if(psong->contributor[ROLE_CONDUCTOR])
			len += strlen(psong->contributor[ROLE_CONDUCTOR]);

		len += 3;

		psong->contributor[ROLE_ARTIST] = (char*)calloc(len, 1);
		if(psong->contributor[ROLE_ARTIST])
		{
			if(psong->contributor[ROLE_BAND])
				strcat(psong->contributor[ROLE_ARTIST], psong->contributor[ROLE_BAND]);

			if(psong->contributor[ROLE_BAND] && psong->contributor[ROLE_CONDUCTOR])
				strcat(psong->contributor[ROLE_ARTIST], " - ");

			if(psong->contributor[ROLE_CONDUCTOR])
				strcat(psong->contributor[ROLE_ARTIST], psong->contributor[ROLE_CONDUCTOR]);
		}
	}

#if 0 // already taken care of by scanner.c
	if(!psong->title)
	{
		char *suffix;
		psong->title = strdup(psong->basename);
		suffix = strrchr(psong->title, '.');
		if(suffix) *suffix = '\0';
	}
#endif
}


/*****************************************************************************/
// _get_tags
static int
_get_tags(char *file, struct song_metadata *psong)
{
	taghandler *hdl;

	// dispatch
	for(hdl = taghandlers ; hdl->type ; ++hdl)
		if(!strcasecmp(hdl->type, psong->type))
			break;

	if(hdl->get_tags)
	{
		return hdl->get_tags(file, psong);
	}

	return 0;
}

/*****************************************************************************/
// readtags
int
readtags(char *path, struct song_metadata *psong, struct stat *stat, char *lang, char *type)
{
	char *fname;

	if(lang_index == -1)
		lang_index = _lang2cp(lang);

	memset((void*)psong, 0, sizeof(struct song_metadata));
	psong->path = strdup(path);
	psong->type = type;

	fname = strrchr(psong->path, '/');
	psong->basename = fname ? fname + 1 : psong->path;

	if(stat)
	{
		if(!psong->time_modified)
			psong->time_modified = stat->st_mtime;
		psong->file_size = stat->st_size;
	}

	// get tag
	if( _get_tags(path, psong) == 0 )
	{
		_make_composite_tags(psong);
	}
	
	// get fileinfo
	return _get_fileinfo(path, psong);
}
