//=========================================================================
// FILENAME	: tagutils-mp3.h
// DESCRIPTION	: MP3 metadata reader
//=========================================================================
// Copyright (c) 2008- NETGEAR, Inc. All Rights Reserved.
//=========================================================================

/*
 * This program is free software; you can redistribute it and/or modify
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
#ifndef __TAGUTILS_MP3_H__
#define __TAGUTILS_MP3_H__

struct mp3_frameinfo {
	int layer;                              // 1,2,3
	int bitrate;                            // unit=kbps
	int samplerate;                         // samp/sec
	int stereo;                             // flag

	int frame_length;                       // bytes
	int crc_protected;                      // flag
	int samples_per_frame;                  // calculated
	int padding;                            // flag
	int xing_offset;                        // for xing hdr
	int number_of_frames;

	int frame_offset;

	short mpeg_version;
	short id3_version;

	int is_valid;
};

extern int _get_mp3tags(char *file, struct song_metadata *psong);
extern int _get_mp3fileinfo(char *file, struct song_metadata *psong);
//static int _decode_mp3_frame(unsigned char *frame, struct mp3_frameinfo *pfi);



#endif

