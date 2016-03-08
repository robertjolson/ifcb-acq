//-----------------------------------------------------------------------------
//  IFCB Project
//	Analysis.cpp
//	Martin Cooper
//
//	Blob analysis routines
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "config.h"
#include "Analysis.h"
#include "IfcbDlg.h"
#include "GraphTab.h"
#include "Process.h"

#include <vector>
#include <list>

using namespace std;

// user config variables from config.h
int						binarizeThresh;			
int						syringeSize;			
int						blobXGrow, blobYGrow;
int						blobJoinGap;
int						resizeFactor;
uint16					MinBlobArea;
//int						BinarizeThresh;

static BYTE				bThresh;
static CString			debugStr;				
static LARGE_INTEGER	startTime, now, then;	// used for code speed measurements
static LARGE_INTEGER	counterFrequency;		// ditto
static GRAPH_BUFFER		tBuffer = {NULL, 0, 0, 0};
static CCameraTab			*This;
static CRect			*displayRect;
static int				dx[8], dy[8];
static list<RECT_T>		blobs;
static int				sourceWidth, sourceHeight;

//-----------------------------------------------------------------------------
static unsigned int XYtoI(int x, int y) {
	
	return x + y * tBuffer.width;
}

//-----------------------------------------------------------------------------
//	Copies This->Buffer to CImage, ready for displaying
//-----------------------------------------------------------------------------
static void BufferToImage(int width, int height) {

	// resize Image object
	if (!This->Image.IsNull())
		This->Image.Destroy();
	This->Image.Create(width, -static_cast<INT32_TYPE>(height), 24);			// RGB

	BYTE *d;
	int pitch = This->Image.GetPitch();

	for (int y = 0, i = 0; y < height; y++) {
		d = (BYTE *)This->Image.GetBits() + y * pitch;
		for (int x = 0; x < width; x++, i++) {
			*d++ = This->Buffer[i];							// blue
			*d++ = This->Buffer[i];							// green
			*d++ = This->Buffer[i];							// red
		}
	}
}

//-----------------------------------------------------------------------------
//	Copies tBuffer to CImage, ready for displaying
//-----------------------------------------------------------------------------
static void TBufferToImage(void) {

	// resize Image object
	if (!This->Image.IsNull())
		This->Image.Destroy();
	This->Image.Create(tBuffer.width, -static_cast<INT32_TYPE>(tBuffer.height), 24);			// RGB

	BYTE *d, *s;
	int y;
	int pitch = This->Image.GetPitch();

	for (y = 0, s = tBuffer.buf; y < tBuffer.height; y++) {
		d = (BYTE *)This->Image.GetBits() + y * pitch;
		for (int x = 0; x < tBuffer.width; x++) {
			*d++ = *s;										// blue
			*d++ = *s;											// green
			*d++ = *s++;											// red
		}
	}
/*	if (appDebug) {
		DWORD sTime;
//		debugStr.Format(_T("BufferToImage took %d ms\r\n"), GetTickCount() - startTime.QuadPart);
//		DEBUG_MESSAGE_EXT(debugStr);
		startTime.QuadPart += GetTickCount() - sTime;
	}*/
}

//-----------------------------------------------------------------------------
static void Contract(void) {

	int x, y;
	BYTE *d, *s;

#ifdef BLOB_DEBUG
	if (appDebug)
		DEBUG_MESSAGE_EXT(_T("Resizing image"));
#endif

	tBuffer.width =  sourceWidth / resizeFactor;
	tBuffer.height = sourceHeight / resizeFactor;
	tBuffer.size = tBuffer.height * tBuffer.width;

	if (tBuffer.buf)
		free(tBuffer.buf);

	if (!(tBuffer.buf = (BYTE *)malloc(tBuffer.size))) {
		ERROR_MESSAGE_EXT(_T("malloc error \r\n"));
		return;
	}

	s = This->Buffer;
	d = tBuffer.buf;

/*	// just take the top left corner
	int dy = tBuffer.width * (resizeFactor - 1);
	for (y = 0; y < tBuffer.height; y += resizeFactor) {
		for (x = 0; x < tBuffer.width; x += resizeFactor) {
			*d++ = (BYTE)(*s);
			s += resizeFactor;
		}
		s += dy;
	}*/

	//	Average value of square
	int i, j, pixel;
	for (y = 0; y < sourceHeight; y += resizeFactor) {
		for (x = 0; x < sourceWidth; x += resizeFactor) {
			pixel = 0;
			for (i = 0; i < resizeFactor; i++) {
				s = This->Buffer + (y + i) * sourceWidth + x;
				for (j = 0; j < resizeFactor; j++) {
					pixel += *s++;
				}
			}
			*d++ = (BYTE)(pixel / (resizeFactor * resizeFactor));
		}
	}
	
	// minimum value in square - slower than average (19 vs 11) but better
/*	memset(tBuffer.buf, 0xFF, tBuffer.size);						// zero the buffer
	for (y = 0; y < sourceHeight; y++) {
		d = tBuffer.buf + (y / resizeFactor) * tBuffer.width;
		for (x = 0; x < sourceWidth; ) {
			for (int i = 0; i < resizeFactor; i++, x++) {
				*d = min(*d, *s);
				s++;
			}
			d++;
		}
	}*/

#ifdef BLOB_DEBUG
	if (appDebug || viewImages)
		QueryPerformanceCounter(&then);
	if (appDebug) {
		debugStr.Format(_T(" - took %d ms\r\n"), (then.QuadPart - startTime.QuadPart) / counterFrequency.QuadPart);
		DEBUG_MESSAGE_EXT(debugStr);
	}
	if (viewImages) {
		TBufferToImage();
		This->InvalidateRect(*displayRect, FALSE);
		if (appDebug)
			AfxMessageBox(_T("Resized Image"));
	}
	if (appDebug || viewImages) {
		QueryPerformanceCounter(&now);
		startTime.QuadPart += now.QuadPart - then.QuadPart;
	}
#endif
}

//-----------------------------------------------------------------------------
static void FindMinMax(unsigned int &min, unsigned int &max) {

	int i;
	BYTE *p;

#ifdef BLOB_DEBUG
	if (appDebug)
		DEBUG_MESSAGE_EXT(_T("Scanning image for min & max"));
#endif

	min = 0xFF;
	max = 0;
	for (i = 0, p = tBuffer.buf; i < tBuffer.size; i++, p++) {
		max = max(max, *p);
		min = min(min, *p);
	}
#ifdef BLOB_DEBUG
	if (appDebug) {
		QueryPerformanceCounter(&then);
		debugStr.Format(_T(" = %d,%d - took %d ms\r\n"), min, max, (then.QuadPart - startTime.QuadPart) / counterFrequency.QuadPart);
		DEBUG_MESSAGE_EXT(debugStr);
		QueryPerformanceCounter(&now);
		startTime.QuadPart += now.QuadPart - then.QuadPart;
	}
#endif
}

//-----------------------------------------------------------------------------
static void HistogramStretch(void) {

#ifdef BLOB_DEBUG
	if (appDebug)
		DEBUG_MESSAGE_EXT(_T("Doing histogram stretch"));
#endif

	int i, y, m, c, min, max;
	BYTE *p;

	// find min and max
	min = 0xFF;
	max = 0;
	for (i = 0, p = tBuffer.buf; i < tBuffer.size; i++, p++) {
		max = max(max, *p);
		min = min(min, *p);
	}

	// now rescale
	if ((max != min) && (max - min != 0xFF)) {			// exit on divide by zero or no stretch required
		m = 0xFF0000 / (max - min);
		c = min * m;
		for (i = 0, p = tBuffer.buf; i < tBuffer.size; i++, p++) {
			y = *p;
			y *= m;
			y -= c;
			*p = y >> 16;
		}
	}

#ifdef BLOB_DEBUG
	if (appDebug || viewImages)
		QueryPerformanceCounter(&then);
	if (appDebug) {
		debugStr.Format(_T(" - took %d ms\r\n"), (then.QuadPart - startTime.QuadPart) / counterFrequency.QuadPart);
		DEBUG_MESSAGE_EXT(debugStr);
	}
	if (viewImages) {
		TBufferToImage();
		This->InvalidateRect(*displayRect, FALSE);
		if (appDebug)
			AfxMessageBox(_T("Histogram stretched"));
	}
	if (appDebug || viewImages) {
		QueryPerformanceCounter(&now);
		startTime.QuadPart += now.QuadPart - then.QuadPart;
	}
#endif
}

//-----------------------------------------------------------------------------
//	Calculates graph of pixel value populations, then sets bThresh
//	to be at the bottom of the LHS of the peak
//-----------------------------------------------------------------------------
static void HistogramThreshold(void) {

	int histogram[256], hmax;
	int i, yThresh;
	BYTE *p;
	int *q;

#ifdef BLOB_DEBUG
	if (appDebug)
		DEBUG_MESSAGE_EXT(_T("Calculating histogram threshold"));
#endif

	memset(histogram, 0, 256 * sizeof(int));						// zero the histogram
	for (i = 0, p = tBuffer.buf; i < tBuffer.size; i++, p++)		// fill the histogram
		histogram[*p]++;

	q = histogram;
	for (i = 0, hmax = 0; i < 256; i++, q++)						// find the histogram peak
		hmax = max(hmax, *q);

	// now find the bottom of the LHS of the peak
	q = histogram;
	yThresh = hmax * binarizeThresh;								// calculate the threshold value
	yThresh /= 1000;
	for (i = 0; i < 255; i++, q++)
		if (*q > yThresh)
			break;
	bThresh = q - histogram;

#ifdef BLOB_DEBUG
	if (appDebug) {
		QueryPerformanceCounter(&now);
		debugStr.Format(_T(" = %d - took %d ms\r\n"), bThresh, (now.QuadPart - startTime.QuadPart) / counterFrequency.QuadPart);
		DEBUG_MESSAGE_EXT(debugStr);
	}
#endif
}

//-----------------------------------------------------------------------------
static void ConvertToGradients(void) {

	int *gBuffer, i;
	BYTE *q, z;
	int *g, *h;
	int x, y;

#ifdef BLOB_DEBUG
	if (appDebug)
		DEBUG_MESSAGE_EXT(_T("Converting to gradients"));
#endif

	if (!(gBuffer = (int *)malloc(tBuffer.size * sizeof(int)))) {
		ERROR_MESSAGE_EXT(_T("malloc error \r\n"));
		return;
	}

	// set first an line values to 0
	g = gBuffer;									// g points to first line
	h = gBuffer + tBuffer.size - tBuffer.width;		// h points to last line
	for (x = 0; x < tBuffer.width; x++)
		*g++ = *h++ = 0;

	// set first and last column values to 0
	g = gBuffer + tBuffer.width;
	h = g + tBuffer.width - 1;
	for (y = 1; y < tBuffer.height - 1; y++) {
		*g = *h = 0;
		g += tBuffer.width;
		h += tBuffer.width;
	}

	// now calc gradients
	g = gBuffer + tBuffer.width + 1;
	for (y = 1; y < tBuffer.height - 1; y++) {
		for (x = 1; x < tBuffer.width - 1; x++) {
			z = tBuffer.buf[XYtoI(x, y)];
			*g = abs(z - tBuffer.buf[XYtoI(x - 1, y)]);
			*g += abs(z - tBuffer.buf[XYtoI(x - 1, y - 1)]);
			*g += abs(z - tBuffer.buf[XYtoI(x, y - 1)]);
			*g += abs(z - tBuffer.buf[XYtoI(x + 1, y - 1)]);
			*g += abs(z - tBuffer.buf[XYtoI(x + 1, y)]);
			*g += abs(z - tBuffer.buf[XYtoI(x + 1, y + 1)]);
			*g += abs(z - tBuffer.buf[XYtoI(x, y + 1)]);
			*g += abs(z - tBuffer.buf[XYtoI(x - 1, y + 1)]);
			g++;
		}
		g += 2;
	}

	// transfer gBuffer to tBuffer
	for (i = 0, q = tBuffer.buf, g = gBuffer; i < tBuffer.size; i++, g++, q++)
		*q = (BYTE)(*g >> 3);

	free(gBuffer);

#ifdef BLOB_DEBUG
	if (appDebug || viewImages)
		QueryPerformanceCounter(&then);
	if (appDebug) {
		debugStr.Format(_T(" - took %d ms\r\n"), (then.QuadPart - startTime.QuadPart) / counterFrequency.QuadPart);
		DEBUG_MESSAGE_EXT(debugStr);
	}
	if (viewImages) {
		TBufferToImage();
		This->InvalidateRect(*displayRect, FALSE);
		if (appDebug)
			AfxMessageBox(_T("Gradient"));
	}
	if (appDebug || viewImages) {
		QueryPerformanceCounter(&now);
		startTime.QuadPart += now.QuadPart - then.QuadPart;
	}
#endif
}

//-----------------------------------------------------------------------------
static void Binarize(void) {

	int i;
	BYTE *q;

#ifdef BLOB_DEBUG
	if (appDebug)
		DEBUG_MESSAGE_EXT(_T("Binarizing image"));
#endif

	for (i = 0, q = tBuffer.buf; i < tBuffer.size; i++, q++) {
		*q = *q > binarizeThresh ? 0 : 0xFF;
	}

#ifdef BLOB_DEBUG
	if (appDebug || viewImages)
		QueryPerformanceCounter(&then);
	if (appDebug) {
		debugStr.Format(_T(" - took %d ms\r\n"), (then.QuadPart - startTime.QuadPart) / counterFrequency.QuadPart);
		DEBUG_MESSAGE_EXT(debugStr);
	}
	if (viewImages) {
		TBufferToImage();
		This->InvalidateRect(*displayRect, FALSE);
		if (appDebug)
			AfxMessageBox(_T("Binarized"));
	}
	if (appDebug || viewImages) {
		QueryPerformanceCounter(&now);
		startTime.QuadPart += now.QuadPart - then.QuadPart;
	}
#endif
}

//-----------------------------------------------------------------------------
//	inverted == true implies that the background is black (i.e. tBuffer)
//	inverted == false implies that the background is white (i.e. This->Buffer)
//-----------------------------------------------------------------------------
static void DrawBox(RECT_T blob, bool inverted) {

	int x, y;

	for (x = blob.xmin; x <= blob.xmax; x++) {
		if (inverted) {
			This->Image.SetPixel(x, blob.ymin, This->Image.GetPixel(x, blob.ymin) + 0x000000FF);
			This->Image.SetPixel(x, blob.ymax, This->Image.GetPixel(x, blob.ymax) + 0x000000FF);
		} else {
			This->Image.SetPixel(x, blob.ymin, This->Image.GetPixel(x, blob.ymin) & 0x000000FF);
			This->Image.SetPixel(x, blob.ymax, This->Image.GetPixel(x, blob.ymax) & 0x000000FF);
		}
	}
	for (y = blob.ymin; y <= blob.ymax; y++) {
		if (inverted) {
			This->Image.SetPixel(blob.xmin, y, This->Image.GetPixel(blob.xmin, y) + 0x000000FF);
			This->Image.SetPixel(blob.xmax, y, This->Image.GetPixel(blob.xmax, y) + 0x000000FF);
		} else {
			This->Image.SetPixel(blob.xmin, y, This->Image.GetPixel(blob.xmin, y) & 0x000000FF);
			This->Image.SetPixel(blob.xmax, y, This->Image.GetPixel(blob.xmax, y) & 0x000000FF);
		}
	}
}

//-----------------------------------------------------------------------------
//	initialise the dx[] and dy[] arrays
//	This is a list of vectors arranged as a clockwise circle
//-----------------------------------------------------------------------------
static void InitDi(void) {

	dx[0] = 1;		dy[0] = -1;
	dx[1] = 1;		dy[1] = 0;
	dx[2] = 1;		dy[2] = 1;
	dx[3] = 0;		dy[3] = 1;
	dx[4] = -1;		dy[4] = 1;
	dx[5] = -1;		dy[5] = 0;
	dx[6] = -1;		dy[6] = -1;
	dx[7] = 0;		dy[7] = -1;
}

//-----------------------------------------------------------------------------
//	searches region for a blob
//	if it finds one, it gets its extents by tracing the perimeter
//	Crops the blob to fit the search area and puts blob extents into blob struct 
//	Then calls itself to check the rest of region - RECURSIVE!
//-----------------------------------------------------------------------------
static void FindBlobs(RECT_T region) {

	int iFirst, iTry, i, w2, y;
	int xCurrent, yCurrent, xTry, yTry;
	BYTE index, j;
	bool found;
	BYTE *p;
	RECT_T tBlob;

	// special case if you get a single pixel spot at the edge of the region or a cropped blob
	if ((region.xmin > region.xmax) || (region.ymin > region.ymax))
		return;

#ifdef BLOB_DEBUG
/*	if (appDebug) {
		debugStr.Format(_T("Searching region x(%d-%d) y(%d-%d)\r\n"), region.xmin, region.xmax, region.ymin, region.ymax);
		DEBUG_MESSAGE_EXT(debugStr);
	}*/
#endif

	// look for first black pixel
	w2 = region.xmax - region.xmin;
	for (found = false, y = region.ymin; y <= region.ymax; y++) {
		p = tBuffer.buf + XYtoI(region.xmin, y);				
		for (i = 0; i <= w2; i++) {
//			This->Image.SetPixel(region.xmin + i, y, This->Image.GetPixel(region.xmin + i, y) | 0x0000FF00);
			if (!*p++) {
				found = true;
				break;
			}
		}
		if (found)
			break;
	}
	if (!found)
		return;

/*	if (appDebug) {
		sprintf_s(str, 80, " - took %d ms\r\n", GetTickCount() - startTime.QuadPart);
		WriteFile(hSerialFluidics, str, strlen(str), &nBytes, NULL);
//		debugStr.Format(_T("Searching region (%d,%d) (%d,%d)"), region.xmin, region.ymin, region.xmax, region.ymax);
		debugStr = str;
		DEBUG_MESSAGE_EXT(debugStr);
	}*/


	// now search round its perimeter
	iFirst = p - tBuffer.buf - 1;
	index = 0;
	xCurrent = tBlob.xmin = tBlob.xmax = iFirst % tBuffer.width;
	yCurrent = tBlob.ymin = tBlob.ymax = iFirst / tBuffer.width;

	do {
		// now look for neighbour
		for (found = false, j = 0; j < 8; j++) {
			index = ++index % 8;
			xTry = xCurrent + dx[index];
			yTry = yCurrent + dy[index];

			// check for neighbour out of region
			if ((xTry > region.xmax) || (xTry < region.xmin) || (yTry > region.ymax) || (yTry < region.ymin))
				continue;												// try the next vector

			iTry = xTry + yTry * tBuffer.width;
			if (!tBuffer.buf[iTry]) {
				found = true;
				break;								// found a neighbour
			}
		}
		if (!found)									// can only happen if blob is 1 pixel big
			break;

		// I have the next neighbour, so update the points
		xCurrent = xTry;
		yCurrent = yTry;
		index = (index + 4) % 8;					// reverse the vector

		// compare this new point with max & min
		tBlob.xmin = min(tBlob.xmin, xTry);
		tBlob.xmax = max(tBlob.xmax, xTry);
		tBlob.ymin = min(tBlob.ymin, yTry);
		tBlob.ymax = max(tBlob.ymax, yTry);

	} while (iTry != iFirst);						// stop when I get back to the beginning

	// compensate for the rounding effect of the contract/expand process
	tBlob.xmax++;
	tBlob.ymax++;

	blobs.push_back(tBlob);							// add this blob to the list
#ifdef BLOB_DEBUG
/*	if (appDebug) {
		debugStr.Format(_T("Found blob x(%d-%d) y(%d-%d)\r\n"), tBlob.xmin, tBlob.xmax, tBlob.ymin, tBlob.ymax);
		DEBUG_MESSAGE_EXT(debugStr);
	}*/
#endif

	// now search the region to the left of the blob
	RECT_T newRegion;
	if (tBlob.xmin > region.xmin) {
		newRegion.xmin = region.xmin;
		newRegion.xmax = tBlob.xmin - 1;
		newRegion.ymin = tBlob.ymin;
		newRegion.ymax = tBlob.ymax;
		FindBlobs(newRegion);							// this will overwrite tBlob
	}

	// now search the region to the right of the blob
	if (tBlob.xmax < region.xmax) {
		newRegion.xmin = tBlob.xmax + 1;
		newRegion.xmax = region.xmax;
		newRegion.ymin = tBlob.ymin;
		newRegion.ymax = tBlob.ymax;
		FindBlobs(newRegion);
	}

	// now search the region below the blob
	if (tBlob.ymax < region.ymax) {
		newRegion.xmin = region.xmin;
		newRegion.xmax = region.xmax;
		newRegion.ymin = tBlob.ymax + 1;
		newRegion.ymax = region.ymax;
		FindBlobs(newRegion);
	}

	return;
}

//-----------------------------------------------------------------------------
static void CalcBlobAreas(void) {
	
	RECT_T blob;
	list<RECT_T>::iterator b;

	for (b = blobs.begin(); b != blobs.end(); b++) {
		blob = *b;
		blob.area = (blob.xmax - blob.xmin) * (blob.ymax - blob.ymin);
		*b = blob;
	}
}

//-----------------------------------------------------------------------------
//	Increases the blob size by blobXGrow and blobYGrow
//-----------------------------------------------------------------------------
static void GrowBlobs(void) {

	RECT_T blob;
	list<RECT_T>::iterator b;

	for (b = blobs.begin(); b != blobs.end(); b++) {
		blob = *b;

		blob.xmin = max(blob.xmin - blobXGrow, 0);
		blob.xmax = min(blob.xmax + blobXGrow, sourceWidth);
		blob.ymin = max(blob.ymin - blobYGrow, 0);
		blob.ymax = min(blob.ymax + blobYGrow, sourceHeight);

		*b = blob;
	}
}

//-----------------------------------------------------------------------------
//	checks to see if point is in a rectangle 1 pixel bigger than b
//-----------------------------------------------------------------------------
static bool PointInBlob(int x, int y, RECT_T b) {

	return ((x >= b.xmin) && (x <= b.xmax) && (y >= b.ymin) && (y <= b.ymax));
}

//-----------------------------------------------------------------------------
//	returns true if any blobs got joined
//	have to keep calling this until it returns false
//	if (expand), include blobs that are near eachother
//-----------------------------------------------------------------------------
static bool JoinBlobs(bool expand) {

	RECT_T blob1, blob2;
	list<RECT_T>::iterator b1, b2, b3;
	bool joined;

	if (blobs.size() < 2)
		return false;

	// run through the list forwards
	joined = false;
	for (b1 = blobs.begin(); b1 != blobs.end(); b1++) {
		blob1 = *b1;
		b2 = b1;
		b2++;
		for ( ; b2 != blobs.end(); b2++) {
			blob2 = *b2;

			if (expand) {
				// expand blob1 by blobGapJoin
				blob1.xmin -= blobJoinGap;
				blob1.xmax += blobJoinGap;
				blob1.ymin -= blobJoinGap;
				blob1.ymax += blobJoinGap;
			}

			// see if any corner of blob2 is inside blob1
			if (!(	PointInBlob(blob2.xmin, blob2.ymin, blob1) ||
					PointInBlob(blob2.xmin, blob2.ymax, blob1) ||
					PointInBlob(blob2.xmax, blob2.ymin, blob1) ||
					PointInBlob(blob2.xmax, blob2.ymax, blob1) ))
				continue;

			// expanded b1 and b2 intersect
			joined = true;

			blob1 = *b1;								// restore blob1's original size
			// increase blob1 to encompass both blobs
			blob1.xmin = min(blob1.xmin, blob2.xmin);
			blob1.xmax = max(blob1.xmax, blob2.xmax);
			blob1.ymin = min(blob1.ymin, blob2.ymin);
			blob1.ymax = max(blob1.ymax, blob2.ymax);

			b3 = b2;
			b3--;
			blobs.erase(b2);							// delete the second blob
			*b1 = blob1;								// update the first
			b2 = b3;									// reassign b2
		}
	}

	if (blobs.size() < 2)
		return false;

	// now run through the list backwards
	for (b1 = blobs.end(); ; ) {
		if (b1 == blobs.begin())
			break;
		b1--;
		blob1 = *b1;
		b2 = b1;
		for ( ; b2 != blobs.begin(); ) {
			b2--;
			blob2 = *b2;

			if (expand) {
				// expand blob1 by blobGapJoin
				blob1.xmin -= blobJoinGap;
				blob1.xmax += blobJoinGap;
				blob1.ymin -= blobJoinGap;
				blob1.ymax += blobJoinGap;
			}

			// see if any corner of blob2 is inside blob1
			if (!(	PointInBlob(blob2.xmin, blob2.ymin, blob1) ||
					PointInBlob(blob2.xmin, blob2.ymax, blob1) ||
					PointInBlob(blob2.xmax, blob2.ymin, blob1) ||
					PointInBlob(blob2.xmax, blob2.ymax, blob1) ))
				continue;
			
			// expanded b1 and b2 intersect
			joined = true;

			blob1 = *b1;								// restore blob1's original size
			// increase blob1 to encompass both blobs
			blob1.xmin = min(blob1.xmin, blob2.xmin);
			blob1.xmax = max(blob1.xmax, blob2.xmax);
			blob1.ymin = min(blob1.ymin, blob2.ymin);
			blob1.ymax = max(blob1.ymax, blob2.ymax);

			b3 = b2;
			b3++;
			blobs.erase(b2);							// delete the second blob
			*b1 = blob1;								// update the first
			b2 = b3;									// reassign b2
		}
	}
	return joined;
}

//-----------------------------------------------------------------------------
static void DrawBlobs(bool inverted) {

	RECT_T blob;
	list<RECT_T>::iterator b;

	for (b = blobs.begin(); b != blobs.end(); b++) {
		blob = *b;
		DrawBox(blob, inverted);
	}
}

//-----------------------------------------------------------------------------
//	resizes blobs to match the original image
//-----------------------------------------------------------------------------
static void ResizeBlobs(void) {

	RECT_T blob;
	list<RECT_T>::iterator b;

	for (b = blobs.begin(); b != blobs.end(); b++) {
		blob = *b;
		blob.xmin *= resizeFactor;
		blob.xmax *= resizeFactor;
		blob.ymin *= resizeFactor;
		blob.ymax *= resizeFactor;

		*b = blob;
	}
}

//-----------------------------------------------------------------------------
//	Removes any blobs that are too small
//-----------------------------------------------------------------------------
static void FilterBlobs(void) {

	RECT_T blob;
	list<RECT_T>::iterator b, bmin;
	
#ifdef BLOB_DEBUG
	if (appDebug) {
		debugStr.Format(_T("Filtering blobs: %d"), blobs.size());
		DEBUG_MESSAGE_EXT(debugStr);
	}
#endif

	// if a blob edge is near the image edge, expand it (the gradient process misses the edges out)
	int z = 2 * resizeFactor;
	for (b = blobs.begin(); b != blobs.end(); b++) {
		blob = *b;
		if (blob.xmin < z)
			blob.xmin = 0;
		if (blob.ymin < z)
			blob.ymin = 0;
		if (blob.xmax > sourceWidth - z)
			blob.xmax = sourceWidth - 1;
		if (blob.ymax > sourceHeight - z)
			blob.ymax = sourceHeight - 1;
		*b = blob;
	}

	// calculate areas
	CalcBlobAreas();

	// remove the ones that are too small
	for (b = blobs.begin(); b != blobs.end(); ) {
		blob = *b;
		if (blob.area < minimumBlobArea) {
			b = blobs.erase(b);							// delete the blob: b now points to next item
		} else
			b++;
	}	

#ifdef BLOB_DEBUG
	if (appDebug) {
		debugStr.Format(_T(" -> %d\r\n"), blobs.size());
		DEBUG_MESSAGE_EXT(debugStr);
	}
#endif
}

//-----------------------------------------------------------------------------
// if a blob lies entirely inside the scratch area +- the grow area, then it's the scratch, so remove it
//-----------------------------------------------------------------------------
static void RemoveScratch(void) {

	static const int scratchXmin = 151 - 10;
	static const int scratchXmax = 257 + 10;
	static const int scratchYmin = 639 - 10;
	static const int scratchYmax = 681 + 10;
	RECT_T blob, scratch;
	list<RECT_T>::iterator b;
	
	scratch.xmin = scratchXmin - blobXGrow;
	scratch.xmax = scratchXmax + blobXGrow;
	scratch.ymin = scratchYmin - blobYGrow;
	scratch.ymax = scratchYmax + blobYGrow;

	for (b = blobs.begin(); b != blobs.end(); ) {
		blob = *b;
		if (PointInBlob(blob.xmin, blob.ymin, scratch) && 
			PointInBlob(blob.xmin, blob.ymax, scratch) &&
			PointInBlob(blob.xmax, blob.ymin, scratch) &&
			PointInBlob(blob.xmax, blob.ymax, scratch))
			b = blobs.erase(b);							// delete the blob: b now points to next item
		else
			b++;
	}	
}

//-----------------------------------------------------------------------------
static void PrintBlobStats(void) {

	RECT_T blob;
	list<RECT_T>::iterator b;
	CString str;
	int i;

	for (i = 0, b = blobs.begin(); b != blobs.end(); i++, b++) {
		blob = *b;
		str.Format(_T("blob %d: (%d - %d),(%d - %d),%d\r\n"), i, blob.xmin, blob.xmax, blob.ymin, blob.ymax, blob.area);
		DEBUG_MESSAGE_EXT(str);
		*b = blob;
	}
}

//-----------------------------------------------------------------------------
//	Writes blobs into the ROIArray
//	Writes ifcb_data info into record pointed to by data
//	Cunningly increases the array size as needed until it runs out of memory,
//	at which point it calls WriteArraysToFile()
//-----------------------------------------------------------------------------
void StoreBlob(DataStruct *data, unsigned int blobNum) {

	if (blobNum >= blobs.size())  {								// no blobs or invalid blobNum
		data->LLHxloc = 0;
		data->LLHyloc = 0;
		data->roiSizeX = 0;
		data->roiSizeY = 0;
		data->start_byte = ROIArrayPtr;

		return;
	}

	// there were some blobs
	RECT_T blob;
	list<RECT_T>::iterator b;
	size_t s;

	unsigned int i;
	for (b = blobs.begin(), i = 0; i < blobNum; i++, b++);		// get the required blob
	blob = *b;

	// see if ROIArray has enough space for this roi and increase size if necessary (and possible)
	s = _msize(ROIArray);
	if (blob.area > s - ROIRamPtr) {
		DEBUG_MESSAGE_EXT(_T("Increasing ROI array size\r\n"));
		data->status |= FLAG_ROI_REALLOC;
		void *p = realloc(ROIArray, s + ROI_ARRAY_SIZE_INC);	// ask for more memory
		if (p)
			ROIArray = (uint8 *)p;
		else
			WriteArraysToFile();								// out of ram, so dump to file
	}
	s = _msize(ROIArray);

	// we have enough space, so store the roi and details

	// store ifcb_data stuff
	data->LLHxloc = (long)blob.xmin;						// top left of the camera image is 0,0
	data->LLHyloc = (long)blob.ymax;
	data->roiSizeX = (long)(blob.xmax - blob.xmin);
	data->roiSizeY = (long)(blob.ymax - blob.ymin);
	// the start byte of this ROI in the ROI file: first byte is 1 (ONE) - bad!
	data->start_byte = ROIArrayPtr;

	// write the ROI pixel values from the image buffer, into the roi pixel array
	size_t width = (size_t)(blob.xmax - blob.xmin);
	uint8 *dest = &ROIArray[ROIRamPtr];
	for (int y = blob.ymin; y < blob.ymax; y++) {
		memcpy(dest, This->Buffer + y * sourceWidth + blob.xmin, width);
		dest += width;
	}

	// update pointers
	ROIArrayPtr += blob.area;
	ROIRamPtr += blob.area;
}

//-----------------------------------------------------------------------------
//	performs all the roi finding
//	if loadedImage is false, the image came from the camera and exists in This->Buffer
//	if loadedImage is true, the image came from file and exists in This->Image
//-----------------------------------------------------------------------------
unsigned int Analyze(CCameraTab *T, CRect *r, bool loadedImage) {

	This = T;
	displayRect = r;

#ifdef BLOB_DEBUG
	// display raw image
	if (viewImages) {
		BufferToImage(This->Image.GetWidth(), This->Image.GetHeight());
		This->InvalidateRect(*displayRect, FALSE);
		AfxMessageBox(_T("Raw Image"));
	}

	QueryPerformanceFrequency(&counterFrequency);
	counterFrequency.QuadPart /= 1000;
	QueryPerformanceCounter(&startTime);
#endif

	// resize image, copy to tBuffer and adjust tBufferWidth and tBufferHeight
	sourceWidth = This->Image.GetWidth();
	sourceHeight = This->Image.GetHeight();

	Contract();											// resize the image

	ConvertToGradients();								// calculate intensity gradients
//	HistogramStretch();									// histogram stretch
//	HistogramThreshold();								// calculate histogram threshold
	Binarize();											// binarize

	// find blobs
	InitDi();											// setup the di vector array

	RECT_T region = {0, tBuffer.width - 1, 0, tBuffer.height - 1, 0}; 
	blobs.clear();										// clear the blob list
	FindBlobs(region);									// find all the blobs
#ifdef BLOB_DEBUG
	if (appDebug) {
		QueryPerformanceCounter(&then);
		debugStr.Format(_T("FindBlobs - took %d ms\r\n"), (then.QuadPart - startTime.QuadPart) / counterFrequency.QuadPart);
		DEBUG_MESSAGE_EXT(debugStr);
		QueryPerformanceCounter(&now);
		startTime.QuadPart += now.QuadPart - then.QuadPart;
	}
#endif

	if (blobs.size() > 0) {
		ResizeBlobs();									// un-contract them
		GrowBlobs();									// make the blobs bigger
		while (JoinBlobs(false));						// join blobs
		FilterBlobs();									// remove small blobs
		RemoveScratch();
		while (JoinBlobs(true));						// join blobs near to eachother
	}
	CalcBlobAreas();

#ifdef BLOB_DEBUG
	QueryPerformanceCounter(&now);
	now.QuadPart = (now.QuadPart - startTime.QuadPart) / counterFrequency.QuadPart;

	debugStr.Format(_T(" %d blobs found in %d ms\r\n"), blobs.size(), now.QuadPart);
	DEBUG_MESSAGE_EXT(debugStr);

	DEBUG_MESSAGE_EXT(_T("Restoring image"));
	BufferToImage(tBuffer.width * resizeFactor, tBuffer.height * resizeFactor);
	DEBUG_MESSAGE_EXT(_T(" - done\r\n"));
#endif

	if (viewImages)
		DrawBlobs(false);								// draw boxes round all the blobs

//	PrintBlobStats();

	// plot alignment data
	if (((GraphData.GraphType == GraphData.GRAPH_ROI_ADC) || (GraphData.GraphType == GraphData.GRAPH_ROI_PEAK))
				&& GraphData.running && (blobs.size() > 0)) {
		list<RECT_T>::iterator b, bmax;
		RECT_T blob;
		if (blobs.size() > 1) {						// find the biggest blob
			unsigned int amax = 0;
			for (b = blobs.begin(); b != blobs.end(); b++) {
				blob = *b;
				if (blob.area > amax) {
					amax = blob.area;
					bmax = b;
				}
			}
		} else
			bmax = blobs.begin();

		blob = *bmax;
		GraphData.data[0][GraphData.nextIn + GraphData.nPoints] 
			= GraphData.data[1][GraphData.nextIn + GraphData.nPoints]
			= GraphData.data[2][GraphData.nextIn + GraphData.nPoints]
			= GraphData.data[3][GraphData.nextIn + GraphData.nPoints]
			= (double)blob.ymin;

		debugStr.Format(_T("Y = %0.2f\r\n"), GraphData.data[0][GraphData.nextIn + GraphData.nPoints]);
		DEBUG_MESSAGE_EXT(debugStr);
	}

	return blobs.size();				// return # of blobs
}
