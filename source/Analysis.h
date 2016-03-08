//-----------------------------------------------------------------------------
//  IFCB Project
//	Analysis.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once
#include "atlimage.h"
#include "CameraTab.h"
#include "config.h"

//#define BLOB_DEBUG

typedef struct {
	int xmin;
	int xmax;
	int ymin;
	int ymax;
	unsigned int area;
} RECT_T;

typedef struct {
	BYTE	*buf;						// copy of the This->Buffer to work on
	int		width;
	int		height;
	int		size;
} GRAPH_BUFFER;

unsigned int Analyze(CCameraTab *This, CRect *rect, bool loadedImage);
void StoreBlob(DataStruct *data, unsigned int blobNum);
