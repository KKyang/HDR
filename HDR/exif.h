/**************************************************************************
  exif.h  -- A simple ISO C++ library to parse basic EXIF 
             information from a JPEG file.

  Based on the description of th EXIF file format at:
  http://park2.wakwak.com/~tsuruzoh/Computer/Digicams/exif-e.html  

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
#ifndef __EXIF_H
#define __EXIF_H

// 
// Stores all the EXIF information that this library parses
//
// If any values are 0 or NULL (with the exception of byteAlign), then
// those values are NOT present in the EXIF information and should be
// ignored.
//
class EXIFInfo {
public:

	char byteAlign;				// 0 = Motorola byte alignment, 1 = Intel 
	char *cameraMake;			// String with camera manufacturer's name
	char *cameraModel;			// String with camera model
	char *dateTimeModified;		// date/time string of last modification 
								// (may be blank)
	char *dateTimeOriginal;		// date/time string of original image 
								// (may be blank, or not present)
	char *imgDescription;		// String describing the image
	unsigned focalLength;		// Focal length of lens (millimeters)
	float FStop;				// F-number of lens = 1/FStop 
	float exposureTime;			// Exposure time in seconds
	unsigned short orientation; // Orientation
  struct LatLng {
    double degree;
    double minutes;
    double seconds;
    LatLng() : degree(-360), minutes(-360), seconds(-360) {}
  };
  LatLng GPSLatitude;
  LatLng GPSLongitude;
  double GPSAltitude;

	// Destructor
	~EXIFInfo() {
		if(imgDescription)
			delete[] imgDescription;
		if(cameraMake)
			delete[] cameraMake;
		if(cameraModel)
			delete[] cameraModel;
		if(dateTimeModified)
			delete[] dateTimeModified;
		if(dateTimeOriginal)
			delete[] dateTimeOriginal;
	};
	// Default Constructor
	EXIFInfo() {
		cameraMake = cameraModel = dateTimeModified = dateTimeOriginal = imgDescription = (char*)0;
		focalLength = 0;
		FStop = exposureTime = 0;
	};
} ;

//
// Parse basic EXIF information from a JPEG file buffer.
//
// IN: 		const char* to binary JPEG data
// RETURN:	0 on succes with 'result' filled out
//			error code otherwise, as defined by the 
//				PARSE_EXIF_ERROR_* macros
//
int ParseEXIF(unsigned char *JPEGfile, unsigned int length, EXIFInfo &result);

//
// Error codes returned by ParseEXIF
//

// No EXIF header found in JPEG file.
#define PARSE_EXIF_ERROR_NO_EXIF				1983
#define PARSE_EXIF_ERROR_UNKNOWN_BYTEALIGN		1984
#define PARSE_EXIF_ERROR_CORRUPT				1985

#endif
