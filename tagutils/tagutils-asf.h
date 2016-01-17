//=========================================================================
// FILENAME	: tagutils-asf.h
// DESCRIPTION	: ASF (wma/wmv) metadata reader
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
#ifndef __TAGUTILS_ASF__
#define __TAGUTILS_ASF__

#define __PACKED__  __attribute__((packed))

#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#else
#include <endian.h>
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

typedef struct _GUID {
	uint32_t l;
	uint16_t w[2];
	uint8_t b[8];
} __PACKED__ GUID;

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	GUID name = { l, { w1, w2 }, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#define IsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SWAP32(l) (l)
#define SWAP16(w) (w)
#else
#define SWAP32(l) ( (((l) >> 24) & 0x000000ff) | (((l) >> 8) & 0x0000ff00) | (((l) << 8) & 0x00ff0000) | (((l) << 24) & 0xff000000) )
#define SWAP16(w) ( (((w) >> 8) & 0x00ff) | (((w) << 8) & 0xff00) )
#endif

typedef struct _BITMAPINFOHEADER {
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} __PACKED__ BITMAPINFOHEADER;

typedef struct _WAVEFORMATEX {
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
} __PACKED__ WAVEFORMATEX;

typedef struct _asf_stream_object_t {
	GUID ID;
	uint64_t Size;
	GUID StreamType;
	GUID ErrorCorrectionType;
	uint64_t TimeOffset;
	uint32_t TypeSpecificSize;
	uint32_t ErrorCorrectionSize;
	uint16_t StreamNumber;
	uint32_t Reserved;
} __PACKED__ asf_stream_object_t;

typedef struct _asf_media_stream_t {
	asf_stream_object_t Hdr;
	GUID MajorType;
	GUID SubType;
	uint32_t FixedSizeSamples;
	uint32_t TemporalCompression;
	uint32_t SampleSize;
	GUID FormatType;
	uint32_t FormatSize;
} __PACKED__ asf_media_stream_t;

typedef struct _avi_audio_format_t {
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
} __PACKED__ avi_audio_format_t;

typedef struct _asf_extended_stream_object_t {
	GUID ID;
	uint64_t Size;
	uint64_t StartTime;
	uint64_t EndTime;
	uint32_t DataBitrate;
	uint32_t BufferSize;
	uint32_t InitialBufferFullness;
	uint32_t AltDataBitrate;
	uint32_t AltBufferSize;
	uint32_t AltInitialBufferFullness;
	uint32_t MaximumObjectSize;
	uint32_t Flags;
	uint16_t StreamNumber;
	uint16_t LanguageIDIndex;
	uint64_t AvgTimePerFrame;
	uint16_t StreamNameCount;
	uint16_t PayloadExtensionSystemCount;
} __PACKED__ asf_extended_stream_object_t;

typedef struct _asf_stream_name_t {
	uint16_t ID;
	uint16_t Length;
} __PACKED__ asf_stream_name_t;

typedef struct _asf_payload_extension_t {
	GUID ID;
	uint16_t Size;
	uint32_t InfoLength;
} __PACKED__ asf_payload_extension_t;



typedef struct _asf_object_t {
	GUID ID;
	uint64_t Size;
} __PACKED__ asf_object_t;

typedef struct _asf_codec_entry_t {
	uint16_t Type;
	uint16_t NameLen;
	uint32_t Name;
	uint16_t DescLen;
	uint32_t Desc;
	uint16_t InfoLen;
	uint32_t Info;
} __PACKED__ asf_codec_entry_t;

typedef struct _asf_codec_list_t {
	GUID ID;
	uint64_t Size;
	GUID Reserved;
	uint64_t NumEntries;
	asf_codec_entry_t Entries[2];
	asf_codec_entry_t VideoCodec;
} __PACKED__ asf_codec_list_t;

typedef struct _asf_content_description_t {
	GUID ID;
	uint64_t Size;
	uint16_t TitleLength;
	uint16_t AuthorLength;
	uint16_t CopyrightLength;
	uint16_t DescriptionLength;
	uint16_t RatingLength;
	uint32_t Title;
	uint32_t Author;
	uint32_t Copyright;
	uint32_t Description;
	uint32_t Rating;
} __PACKED__ asf_content_description_t;

typedef struct _asf_file_properties_t {
	GUID ID;
	uint64_t Size;
	GUID FileID;
	uint64_t FileSize;
	uint64_t CreationTime;
	uint64_t TotalPackets;
	uint64_t PlayDuration;
	uint64_t SendDuration;
	uint64_t Preroll;
	uint32_t Flags;
	uint32_t MinPacketSize;
	uint32_t MaxPacketSize;
	uint32_t MaxBitrate;
} __PACKED__ asf_file_properties_t;

typedef struct _asf_header_extension_t {
	GUID ID;
	uint64_t Size;
	GUID Reserved1;
	uint16_t Reserved2;
	uint32_t DataSize;
} __PACKED__ asf_header_extension_t;

typedef struct _asf_video_stream_t {
	asf_stream_object_t Hdr;
	uint32_t Width;
	uint32_t Height;
	uint8_t ReservedFlags;
	uint16_t FormatSize;
	BITMAPINFOHEADER bmi;
	uint8_t ebih[1];
} __PACKED__ asf_video_stream_t;

typedef struct _asf_audio_stream_t {
	asf_stream_object_t Hdr;
	WAVEFORMATEX wfx;
} __PACKED__ asf_audio_stream_t;

typedef struct _asf_payload_t {
	uint8_t StreamNumber;
	uint8_t MediaObjectNumber;
	uint32_t MediaObjectOffset;
	uint8_t ReplicatedDataLength;
	uint32_t ReplicatedData[2];
	uint32_t PayloadLength;
} __PACKED__ asf_payload_t;

typedef struct _asf_packet_t {
	uint8_t TypeFlags;
	uint8_t ECFlags;
	uint8_t ECType;
	uint8_t ECCycle;
	uint8_t PropertyFlags;
	uint32_t PacketLength;
	uint32_t Sequence;
	uint32_t PaddingLength;
	uint32_t SendTime;
	uint16_t Duration;
	uint8_t PayloadFlags;
	asf_payload_t Payload;
} __PACKED__ asf_packet_t;

typedef struct _asf_data_object_t {
	GUID ID;
	uint64_t Size;
	GUID FileID;
	uint64_t TotalPackets;
	unsigned short Reserved;
} __PACKED__ asf_data_object_t;

typedef struct _asf_padding_object_t {
	GUID ID;
	uint64_t Size;
} __PACKED__ asf_padding_object_t;

typedef struct _asf_simple_index_object_t {
	GUID ID;
	uint64_t Size;
	GUID FileID;
	uint32_t IndexEntryTimeInterval;
	uint32_t MaximumPacketCount;
	uint32_t IndexEntriesCount;
} __PACKED__ asf_simple_index_object_t;

typedef struct _asf_header_object_t {
	GUID ID;
	uint64_t Size;
	uint32_t NumObjects;
	uint16_t Reserved;
	asf_header_extension_t HeaderExtension;
	asf_content_description_t ContentDescription;
	asf_file_properties_t FileProperties;
	asf_video_stream_t *        VideoStream;
	asf_audio_stream_t *        AudioStream;
	asf_codec_list_t CodecList;
	asf_padding_object_t PaddingObject;
} __PACKED__ asf_header_object_t;


#define ASF_VT_UNICODE          (0)
#define ASF_VT_BYTEARRAY        (1)
#define ASF_VT_BOOL             (2)
#define ASF_VT_DWORD            (3)
#define ASF_VT_QWORD            (4)
#define ASF_VT_WORD             (5)

extern int _get_asffileinfo(char *file, struct song_metadata *psong);

#ifndef offsetof
#define offsetof(STRUCTURE,FIELD) ((int)((char*)&((STRUCTURE*)0)->FIELD))
#endif

#endif

