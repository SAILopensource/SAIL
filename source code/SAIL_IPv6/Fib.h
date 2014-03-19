#include <queue>
#include "sTable.h"

#define	SUCCEED					0
#define	FAIL									-1

#define	FIBLEN							sizeof(struct FibNode)		//size of each node in FibTrie
#define	EMPTYHOP					0											//Trie node doesnt't has a next hop
#define	TRIE4								0											// trie for ipv4
#define	TRIE6								1											// trie for ipv6
#define	MAX_BUF_LEN			1024									// buffer length

// Error Definition
#define	AddNodeTypeError		0x1110
#define	AddNodePrefixError		0x1111
#define	FileOpenError				-1
#define	TrieBuildError				0x1112

// Verify type
#define COMM								0x2220								// basic trie
#define LEVELPUSH					0x2221								// level pushed trie
#define SAILTABLE						0x2222								// sail table for level trie

// Size
#define KB										(1024 * 8)
#define MB									(KB * 1024)
#define GB										(MB * 1024)

//node in Fib Trie
struct FibNode
{
	//FibNode*			parent;				// point to father node
	FibNode*			lchild;					// point to the left child(0)
	FibNode*			rchild;					// point to the right child(1)
	unsigned char		newPort;				// nexthop info
	//unsigned char						oldPort;

	//bool						ifpushed;
	//bool						isRoute;
	unsigned char						nodeLevel;
};

class FibTrie {
public:
	FibNode* root4;							// root node for IPv4
	FibNode* root6;							// root node for IPv6

	bool levelVisited[65];				// for test
	
	unsigned int maxPreLen;			// test for levelpushing

	unsigned int entryTotal;				// the number of leaf nodes in original trie
	unsigned int origLevelStat[6][3];	// record the original level statistics in trie

	unsigned int levelStat[65];		// the total nodes in level_xxx
	unsigned int levelLeafNodeStat[65];	// the total leaf nodes in level_xxx
	unsigned int levelInterNodeStat[65];	// the total internal nodes in level_xxx

	unsigned short levelIndex[6];
	unsigned int levelMap[65];		// level_16	0; level_24	1; level_32	2; level_40	3
	
	struct sailLevelTable levelTables[TABLE_NUM];	// 4 level tables for fast lookup
	levelSplitTable splitTables[2];		// level_48	0; level_64 1; The last two levels' tables for fast lookup

	//sailHopTable lastLevelTable;	// the last level arr for next hop info

	FibTrie();
	~FibTrie();

	struct FibNode* createNewNode();
	bool isLeaf(FibNode * pNode);
	int addNode(char prefix[],unsigned int preLen,unsigned int nextHop, unsigned int type);
	int buildTrieFromFile(const char* path, unsigned int type);

	int lookup(char prefix[], unsigned int len, unsigned int type, unsigned int &nextHop);
	void ytLevelPushing(FibNode* pTrie, unsigned int level,unsigned int default_port);

	void traverse(FibNode* fnode, int level);
	void estimateMem(unsigned int trieType);
	void buildSailTables(unsigned int type);

	void levelArrLookup(unsigned long long prefix, unsigned int& nextHop);

	void verify(const char *inPath, const char *outPath, unsigned int vType, unsigned int trieType);
	void FibTrie::LookupspeedTest(const char * tracefile, unsigned int trieType);

	unsigned long long memStat();
	void performance();

	void generateReport(char* trace, char* route);

	void trieDestroy(FibNode* root);
	void fibTrieDestroy();

private:
	int nodeNumber;
	unsigned long long totalMem; 
	bool ifLevelPushed;
	bool ifSailTableBuild;

	FILE* report;

	char* itobs(unsigned long long n,char* ps);
	void preVisit(FibNode* fnode);
};