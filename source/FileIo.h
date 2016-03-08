//-----------------------------------------------------------------------------
//  IFCB Project
//	FileIo.h
//	Martin Cooper
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

typedef struct {
	int	index;
	char			type;
	void			*p;
	const char		*description;
} CONFIG_PARAM;


#define LONG_STR_BUF_LEN	1024
#define CFG_FILE_LENGTH		10000			// # characters in file
#define FREQ_FILE_LENGTH	100			// # characters in file

void	CStringToChar(char *dest, CString *source);
void	CreateOutputFileNames(unsigned int dataDirectory);
bool	WriteHeaderFile(void);
bool	WriteAdcFile(void);
bool	WriteRoiFile(void);
bool	WriteDigFile(unsigned int nRecs);
bool	WriteCfgFile(void);
bool	WriteAcqFileNameFile(void);
bool	ReadCfgFile(void);
bool	ReadFreqFile(void);
bool	WriteFreqFile(void);
CString GetOutputFileName(void);
bool	CheckEmptyFiles(void);


enum {	FILES_NORMAL,		// put files in directory specified by config file
		FILES_BEADS			// put files in special bead subdirectory of that in config file
};