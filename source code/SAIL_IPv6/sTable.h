#include <stdio.h>

#define TABLE_NUM	4

#define LEVEL16			(1 << 16)			// level_16 table size
#define LEVEL24			(1 << 19)			// level_24 table size
#define LEVEL32			(1 << 20)			// level_32 table size
#define LEVEL40			(1 << 20)			// level_40 table size

#define LEVEL48			(1 << 20)			// level_48 table size
#define LEVEL64			(1 << 24)			// last level (level_64) table size


#pragma pack (1)

struct sailTable
{
	bool flag;			// 0 for offset; 1 for hop
	unsigned short offset:16;		// flag = 0 the offset in next level; flag = 1 next hop info
};

struct sailHopTable		// just include next hop info; level 64 tables
{
	unsigned char nextHop[LEVEL64];		// next hop info
	unsigned int total;		// the number of total next hop
};

struct sailLevelTable		// level 16/24/32/40/48 tables
{
	struct sailTable* table;		// table
	unsigned int size;				// table size

	unsigned int total;				// nodes total number
	unsigned int leafCount;		// leaf nodes number
	unsigned int interCount;	// internal nodes number
};

struct levelSplitTable
{
	bool* flag;							// 0 for offset; 1 for hop 
	unsigned short* offset;
	unsigned int size;

	unsigned int total;				// nodes total number
	unsigned int leafCount;		// leaf nodes number
	unsigned int interCount;	// internal nodes number
};