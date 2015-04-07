/**************************************************************************
  exif.cpp  -- A simple ISO C++ library to parse basic EXIF 
               information from a JPEG file.

  Copyright (c) 2010 Mayank Lahiri
  mlahiri@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions are met:

  -- Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
  -- Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS 
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN 
   NO EVENT SHALL THE FREEBSD PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
   OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/
#include"exif.h"
#include<string.h>          // included for memcpy() and memset()

static unsigned int parse32(unsigned char *buf, bool intel) {
  return intel ? 
    (((unsigned)buf[3]<<24) + ((unsigned)buf[2]<<16) + ((unsigned)buf[1]<<8) + buf[0])
    : 
    (((unsigned)buf[0]<<24) + ((unsigned)buf[1]<<16) + ((unsigned)buf[2]<<8) + buf[3]);
}
static unsigned short parse16(unsigned char *buf, bool intel) {
  return intel ? (((unsigned)buf[1]<<8) + buf[0]) : (((unsigned)buf[0]<<8) + buf[1]); 
}
static void copyEXIFString(char **dest, unsigned ncomp, unsigned base, unsigned offs, unsigned char *buf) {
  *dest = new char[ncomp+1];
  memset(*dest, 0, ncomp+1);
  if(ncomp > 4)
    memcpy(*dest, (char*) (buf+base+offs), ncomp);
  else
    memcpy(*dest, (char*) &offs, 4);
}
static float parseEXIFrational(unsigned char *buf, bool intel) {
  double numerator   = 0;
  double denominator = 1;

  numerator  = (double) parse32(buf, intel);
  denominator= (double) parse32(buf+4, intel);
  if(denominator < 1e-20)
    return 0;
  return numerator/denominator;
}

int ParseEXIF(unsigned char *buf, unsigned len, EXIFInfo &result) {
  bool alignIntel = true;     // byte alignment 
  unsigned offs   = 0;        // current offset into buffer
  if(len == 0)
    return PARSE_EXIF_ERROR_NO_EXIF;

  // Prepare return structure
  memset(&result, 0, sizeof(result));

  // Scan for EXIF header and do a sanity check
  for(offs = 0; offs < len-1; offs++) 
    if(buf[offs] == 0xFF && buf[offs+1] == 0xE1) 
      break;
  if(offs == len-1)
    return PARSE_EXIF_ERROR_NO_EXIF;
  offs += 4;
  if(buf[offs] != 0x45 || buf[offs+1] != 0x78 || buf[offs+2] != 0x69) 
    return PARSE_EXIF_ERROR_NO_EXIF;

  // Get byte alignment (Motorola or Intel)
  offs += 6;
  if(buf[offs] == 0x49 && buf[offs+1] == 0x49) 
    alignIntel = true;
  else {
    if(buf[offs] == 0x4d && buf[offs+1] == 0x4d)
      alignIntel = false;
    else 
      return PARSE_EXIF_ERROR_UNKNOWN_BYTEALIGN;
  }
  result.byteAlign = alignIntel;

  // Get offset into first IFD
  offs += 4;
  unsigned x = parse32(buf+offs, alignIntel);
  if(offs + x >= len) {
    return PARSE_EXIF_ERROR_CORRUPT;
  } 

  // Jump to the first IFD, scan tags there.
  offs += x-4;
  int nentries = parse16(buf+offs, alignIntel);
  offs += 2;
  unsigned ifdOffset = offs-10;
  unsigned exifSubIFD = 0;
  unsigned gpsIFD = 0;
  for(int j = 0; j < nentries; j++)  {
    unsigned short tag = parse16(buf+offs, alignIntel);
    unsigned ncomp = parse32(buf+offs+4, alignIntel);
    unsigned coffs = parse32(buf+offs+8, alignIntel);

    switch(tag) {
      case 0x8769:
        // EXIF subIFD offset
        exifSubIFD = ifdOffset + coffs;
        break;

      case 0x10F:
        // Digicam manufacturer
        copyEXIFString(&result.cameraMake, ncomp, ifdOffset, coffs, buf);
        break;

      case 0x110:
        // Digicam model
        copyEXIFString(&result.cameraModel, ncomp, ifdOffset, coffs, buf);
        break;

      case 0x132:
        // EXIF/TIFF date/time of image
        copyEXIFString(&result.dateTimeModified, ncomp, ifdOffset, coffs, buf);
        break;

      case 0x10E:
        // image description 
        copyEXIFString(&result.imgDescription, ncomp, ifdOffset, coffs, buf);
        break;

      case 0x8825:
        gpsIFD = ifdOffset + coffs;
        break;

      case 0x112:
        // Orientation
        result.orientation = parse16(buf+offs+8, alignIntel);
        break;
    }
    offs += 12;
  }
  if(!exifSubIFD)
    return 0;

  // At the EXIF SubIFD, read the rest of the EXIF tags
  offs = exifSubIFD;
  nentries = parse16(buf+offs, alignIntel);
  offs += 2;
  for(int j = 0; j < nentries; j++)  {
    unsigned short tag = parse16(buf+offs, alignIntel);
    unsigned ncomp = parse32(buf+offs+4, alignIntel);
    unsigned coffs = parse32(buf+offs+8, alignIntel);

    switch(tag) {
      case 0x9003:
        // original image date/time string
        copyEXIFString(&result.dateTimeOriginal, ncomp, ifdOffset, coffs, buf);
        break;

      case 0x920a:
        // Focal length in mm
        // result.focalLength = *((unsigned*)(buf+ifdOffset+coffs));
        result.focalLength = parseEXIFrational(buf+ifdOffset+coffs, alignIntel);
        break;

      case 0x829D:
        // F-stop
        result.FStop = parseEXIFrational(buf+ifdOffset+coffs, alignIntel);
        break;

      case 0x829A:
        // Exposure time
        result.exposureTime = parseEXIFrational(buf+ifdOffset+coffs, alignIntel);
        break;
    }
    offs += 12;
  }

	if(gpsIFD) {
		// http://www.exif.org/Exif2-2.PDF see p.54.
		offs = gpsIFD;
		nentries = parse16(buf+offs, alignIntel);
		offs += 2;
		for(int j = 0; j < nentries; j++)  {
			unsigned short tag = parse16(buf+offs, alignIntel);
			unsigned coffs = parse32(buf+offs+8, alignIntel);

			switch(tag) {
        case 2: // Latitude
          result.GPSLatitude.degree = parseEXIFrational(buf+ifdOffset+coffs+0, alignIntel);            
          result.GPSLatitude.minutes = parseEXIFrational(buf+ifdOffset+coffs+8, alignIntel);            
          result.GPSLatitude.seconds = parseEXIFrational(buf+ifdOffset+coffs+16, alignIntel);           
          break;
        case 4: // Longitude
          result.GPSLongitude.degree = parseEXIFrational(buf+ifdOffset+coffs+0, alignIntel);            
          result.GPSLongitude.minutes = parseEXIFrational(buf+ifdOffset+coffs+8, alignIntel);            
          result.GPSLongitude.seconds = parseEXIFrational(buf+ifdOffset+coffs+16, alignIntel);            
          break;
        case 6:	// Altitude
          result.GPSAltitude = parseEXIFrational(buf+ifdOffset+coffs+0, alignIntel); 
          break;
      }
      offs += 12;
		}
	}

  return 0;
}



