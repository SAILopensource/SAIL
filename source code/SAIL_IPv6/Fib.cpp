#include "Fib.h"
#include <iostream>
#include <windows.h>
#define TRACE_READ 100000
using namespace std;

const char *statOutput = "stat.txt";
const char *test = "rrc16_201203012359_v6.prefix_100K.tr";

const char *output = "comm.txt";
const char *ytOutput = "ytLevel.txt";
const char *sailOutput = "sailTable.txt";

// int maxSize[TABLE_NUM] = {LEVEL16, LEVEL24, LEVEL32, LEVEL40, LEVEL48};

FibTrie::FibTrie ()
{
	root4 = createNewNode();
	root6 = createNewNode();

	maxPreLen = 0;
	entryTotal = 0;

	//origLevelStat[6][3] = {0};
	memset(levelIndex, 0, sizeof(levelIndex));
	memset(levelStat, 0, sizeof(levelStat));
	memset(levelLeafNodeStat, 0, sizeof(levelLeafNodeStat));
	memset(levelInterNodeStat, 0, sizeof(levelInterNodeStat));

	memset(levelMap, 0, sizeof(levelMap));			// level_16	0; level_24	1; level_32	2; level_40	3; level_48	4

	for (int t = 0; t < 65; t++)
		levelVisited[t] = false;

	levelMap[16] = 0;
	levelMap[24] = 1;
	levelMap[32] = 2;
	levelMap[40] = 3;
	levelMap[48] = 4;

	levelIndex[0] = 16;
	levelIndex[1] = 24;
	levelIndex[2] = 32;
	levelIndex[3] = 40;
	levelIndex[4] = 48;
	levelIndex[5] = 64;

	nodeNumber = 0;
	totalMem = 0;

	ifLevelPushed = false;
	ifSailTableBuild = false;

	report = fopen (statOutput, "a+");	// To record the generated statistical results
	
	if (!report) {
		cout << "Open file: " << statOutput << " error!" << endl;
	}
}

FibTrie::~FibTrie()
{
	int i = 0;

	for (i = 0; i < TABLE_NUM; i++)
	{
		if (levelTables[i].table)
		{
			free(levelTables[i].table);
		}
	}

	free(splitTables[0].flag);
	free(splitTables[0].offset);
	free(splitTables[1].flag);
	free(splitTables[1].offset);

	fclose(report);

	fibTrieDestroy();
}

//creat new node 
struct FibNode* FibTrie::createNewNode()
{
	struct FibNode* trieNode = NULL;
	trieNode = (struct FibNode*) malloc( FIBLEN );
	
	if (trieNode == NULL)
		return NULL;

	memset(trieNode, 0, sizeof(struct FibNode));

	//initial
	//trieNode->parent = NULL;
	trieNode->lchild = NULL;
	trieNode->rchild = NULL;
	trieNode->newPort = EMPTYHOP;
	//trieNode->oldPort = EMPTYHOP;

	//trieNode->ifpushed = false;
	//trieNode->isRoute = false;

	return trieNode;
}

bool FibTrie::isLeaf(FibNode * pNode)
{
	if (pNode->lchild == NULL && pNode->rchild == NULL)
		return true;
	else 
		return false;	
}

//add a node in Fib T	rie
int FibTrie::addNode(char prefix[],unsigned int preLen,unsigned int nextHop, unsigned int type)
{
	FibNode *pTrie;
	//get the root of rib
	if (type == TRIE4 && preLen <= 32) 
	{
		pTrie = root4;
	}
	else if (type == TRIE6 && preLen <= 64)
	{
		pTrie = root6;
	}
	else
	{
		return AddNodeTypeError;
	}

	//locate every prefix in the FIB Trie
	for (unsigned int i = 0; i < preLen; i++){
		//turn right
		if(prefix[i] == '1')
		{
			//creat new node
			if(pTrie->rchild == NULL)
			{
				FibNode* pTChild = createNewNode();
				//insert new node
				//pTChild->parent = pTrie;
				pTChild->lchild = NULL;
				pTChild->rchild = NULL;
				//pTChild->oldPort = 0;
				pTChild->newPort = 0;
				pTChild->nodeLevel = pTrie->nodeLevel + 1; // the level of the node

				pTrie->rchild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->rchild;

		}
		//turn left
		else if (prefix[i] == '0')
		{
			//if left node is empty, creat a new node
			if( pTrie->lchild == NULL )
			{
				FibNode* pTChild = createNewNode();
				//insert new node
				//pTChild->parent = pTrie;
				pTChild->lchild = NULL;
				pTChild->rchild = NULL;
				//pTChild->oldPort = 0;
				pTChild->newPort = 0;
				pTChild->nodeLevel = pTrie->nodeLevel + 1;

				pTrie->lchild = pTChild;
			}

			pTrie = pTrie->lchild;
		}
		else 
		{
			return AddNodePrefixError;
		}
	}

	pTrie->newPort = nextHop;
	//pTrie->oldPort = nextHop;
	//pTrie->isRoute = true;

	return SUCCEED;
}

int FibTrie::buildTrieFromFile(const char *path, unsigned int type)
{
	if (type == TRIE6 && !root6)
		return TrieBuildError;
	if (type == TRIE4 && !root4)
		return TrieBuildError;

	FILE* file = fopen (path, "r");
	
	if (!file) {
		cout << "Open file: " << path << " error!" << endl;
		return FileOpenError;           
	}

	int ret = FAIL;

	char buf[MAX_BUF_LEN];
	char prefix[MAX_BUF_LEN];

	unsigned int firstSeg;
	unsigned int secSeg;
	unsigned int thirdSeg;
	unsigned int nextHop;

	while (fgets (buf, MAX_BUF_LEN, file)) {	// read a line

		sscanf (buf, "%s %u:%u:%u::%u", prefix, &firstSeg, &secSeg, &thirdSeg, &nextHop);
		++ nodeNumber;
		//cout << "The " << entry_num << "th entry: " << prefix << "	" << first_seg << "	" << second_seg << "	" << third_seg << "	" << next_hop << endl;
		//cout << "The " << nodeNumber << "th entry: " << prefix <<" " <<  strlen(prefix) <<"	" << nextHop << endl;
		if (strlen(prefix) > 0)
		{
			if (maxPreLen < strlen(prefix))
				maxPreLen = strlen(prefix);
			//ret = addNode(prefix, strlen(prefix), nextHop, type);
			ret = addNode(prefix, strlen(prefix), nodeNumber &127, type);
			//cout << prefix << "   " << nodeNumber << endl;

			if (ret != SUCCEED)
				cout << "Add node error!" << endl;
		}
		//cout << "The return value of addig a node: " << ret << endl;
	}

	fclose (file);
	
	int level = 0;

	if (type == TRIE6)
	{
		traverse(root6, level);
	}
	else
	{
		traverse(root4, level);
	}

	int i = 0;
	for (i = 0; i < 6; i++)
	{
		origLevelStat[i][0] = levelStat[levelIndex[i]];
		origLevelStat[i][1] = levelInterNodeStat[levelIndex[i]];
		origLevelStat[i][2] = levelLeafNodeStat[levelIndex[i]];
	}

	//unsigned int totalLeaf = 0;

	//for (i = 0; i < 65; i++)
	//{
	//	totalLeaf += levelLeafNodeStat[i];
	//}

	cout << "The original trie has " << entryTotal << " solid nodes(entries)!!!" << endl;

	return nodeNumber;
}

int FibTrie::lookup(char prefix[], unsigned int len, unsigned int type, unsigned int &nextHop)
{
	FibNode *pTrie;
	//get the root of rib
	if (type == TRIE4)
	{
		pTrie = root4;
	}
	else
	{
		pTrie = root6;
	}

		for (unsigned int i = 0; i < len; i++)
		{
			if(prefix[i] == '1')
			{
				if(pTrie->rchild == NULL)
				{
					return pTrie->nodeLevel;
				}
				
				pTrie = pTrie->rchild;

				if (pTrie->newPort > 0) 
				{
					nextHop = pTrie->newPort;
				}
			}
			else
			{
				if( pTrie->lchild == NULL )
				{
					return pTrie->nodeLevel;
				}

				pTrie = pTrie->lchild;

				if (pTrie->newPort)
				{
					nextHop = pTrie->newPort;
				}
			}
		}

		nextHop = pTrie->newPort;
		return pTrie->nodeLevel;
}

void FibTrie::ytLevelPushing(FibNode* pTrie, unsigned int level,unsigned int default_port)
{
	ifLevelPushed = true;

	if(pTrie == NULL)
		return;

	if((level == 16 || level == 24 || level == 32 || level == 40 || level == 48 || level == 64) && isLeaf(pTrie))
		return;

	if (pTrie->newPort > 0)
		default_port = pTrie->newPort;

	//left child
	if (pTrie->lchild == NULL)
	{
		FibNode* pTChild  = createNewNode();
		
		if (pTChild == NULL)
		{
			printf("create new node failed!\n");
			return;
		}

		//pTChild->parent = pTrie;
		pTChild->lchild = NULL;
		pTChild->rchild = NULL;
		//pTChild->oldPort = 0;
		pTChild->newPort = default_port;
		//pTChild->ifpushed = true;
		pTChild->nodeLevel = pTrie->nodeLevel + 1;

		pTrie->lchild = pTChild;
	}
	else if (pTrie->lchild->newPort == 0)
	{
		pTrie->lchild->newPort = default_port;
	}

	//right child
	if (pTrie->rchild == NULL)
	{
		FibNode* pTChild = createNewNode();

		if (pTChild == NULL)
		{
			printf("create new node failed!\n");
		}

		//pTChild->parent = pTrie;
		pTChild->lchild = NULL;
		pTChild->rchild = NULL;
		//pTChild->oldPort = 0;
		pTChild->newPort = default_port;
		//pTChild->ifpushed = true;
		pTChild->nodeLevel = pTrie->nodeLevel + 1;

		pTrie->rchild = pTChild;
	}
	else if (pTrie->rchild->newPort == 0)
	{
		pTrie->rchild->newPort = default_port;
	}

	pTrie->newPort = 0;

	ytLevelPushing(pTrie->lchild, level+1,default_port);
	ytLevelPushing(pTrie->rchild, level+1,default_port);
}

void FibTrie::traverse(FibNode* fnode, int level)
{
	//if (isLeaf(fnode))
	//{
	//	if (!(level == 16 || level == 24 || level == 32 || level == 40 || level == 48 || level == 64))
	//		printf("The level of this node is %d\n", level);
	//}

	if (level == 0)
	{
		entryTotal = 0;
	}

	//if (fnode->isRoute)
	{
		entryTotal++;
	}
	
	++levelStat[level];

	if (!isLeaf(fnode))
		++levelInterNodeStat[level];
	else
		++levelLeafNodeStat[level];
	

	level++;

	if (fnode->lchild)
		traverse(fnode->lchild, level);
	if (fnode->rchild)
		traverse(fnode->rchild, level);
}

void FibTrie::estimateMem(unsigned int trieType)
{
	FibNode* root = NULL;

	if (trieType == TRIE4)
	{
		root = root4;
	}
	else
	{
		root = root6;
	}
	
	if (!root)
		return;

	int i = 0;
	int level = 0;

	memset(levelStat, 0, sizeof(levelStat));
	memset(levelLeafNodeStat, 0, sizeof(levelLeafNodeStat));
	memset(levelInterNodeStat, 0, sizeof(levelInterNodeStat));

	// Get the node numbers in each level
	traverse(root, level);

	// Initiate level_16, level_24, level_32, level_40 arrays
	memset(levelTables, 0, sizeof(levelTables));
	for (i = 0; i < TABLE_NUM; i++)
	{
		levelTables[i].size = levelStat[levelIndex[i]] + 1;
		levelTables[i].table = (struct sailTable*) calloc(levelTables[i].size, sizeof(sailTable));
		cout << "Level " << levelIndex[i] << ", number of all nodes: " << levelTables[i].size - 1 << endl;
	}

	// Initiate level_48, level_64 split arrays
	memset(&splitTables, 0, sizeof(splitTables));

	splitTables[0].size = levelStat[48] + 1;
	splitTables[0].flag = (bool *) calloc(splitTables[0].size, sizeof(bool));
	splitTables[0].offset = (unsigned short *) calloc(splitTables[0].size, sizeof(unsigned short));

	splitTables[1].size = levelStat[64] + 1;
	splitTables[1].flag = (bool *) calloc(splitTables[1].size, sizeof(bool));
	splitTables[1].offset = (unsigned short *) calloc(splitTables[1].size, sizeof(unsigned short));

	cout << "Level 48, number of all nodes: " << splitTables[0].size - 1 << endl;
	cout << "Level 64, number of all nodes: " << splitTables[1].size -1 << endl;
}



void FibTrie::preVisit(FibNode* node)
{
	if (node == NULL)
		return;

		if (!levelVisited[node->nodeLevel])
		{
			levelVisited[node->nodeLevel] = true;
			//cout << "Visting the " << node->nodeLevel << "th level!!!" << endl;
		}
		
		if (node->nodeLevel == 16 || node->nodeLevel == 24 || node->nodeLevel == 32 || node->nodeLevel == 40)
		{
			if (isLeaf(node))
			{
				levelTables[levelMap[node->nodeLevel]].table[levelTables[levelMap[node->nodeLevel]].total].flag = true;		// leaf node
				levelTables[levelMap[node->nodeLevel]].table[levelTables[levelMap[node->nodeLevel]].total].offset = node->newPort;	// next hop
				levelTables[levelMap[node->nodeLevel]].leafCount++;
			}
			else
			{
				levelTables[levelMap[node->nodeLevel]].table[levelTables[levelMap[node->nodeLevel]].total].flag = false;		// internal node
				levelTables[levelMap[node->nodeLevel]].table[levelTables[levelMap[node->nodeLevel]].total].offset = levelTables[levelMap[node->nodeLevel]].interCount;	// offset
				levelTables[levelMap[node->nodeLevel]].interCount++;
			}

			levelTables[levelMap[node->nodeLevel]].total++;
			if (levelTables[levelMap[node->nodeLevel]].total >= levelTables[levelMap[node->nodeLevel]].size)
				cout << node->nodeLevel << " has " << levelTables[levelMap[node->nodeLevel]].total << " nodes" << endl;
		}
		else if (node->nodeLevel == 48)
		{
			if (isLeaf(node))
			{
				splitTables[0].flag[splitTables[0].total] = true;		// next hop info
				splitTables[0].offset[splitTables[0].total] = node->newPort;
				splitTables[0].leafCount++;
			}
			else
			{
				splitTables[0].flag[splitTables[0].total] = false;		// offset for next level
				splitTables[0].offset[splitTables[0].total] = splitTables[0].interCount;	// offset
				splitTables[0].interCount++;
			}
			
			splitTables[0].total++;
		}
		else if (node->nodeLevel == 64)
		{
			splitTables[1].offset[splitTables[1].total] = node->newPort;
			splitTables[1].total++;
		}

		if (node->lchild)
			preVisit(node->lchild);
		if (node->rchild)
			preVisit(node->rchild);
}

void FibTrie::buildSailTables(unsigned int type)
{
	FibNode* root = NULL;

	if (type == TRIE4)
	{
		root = root4;
	}
	else
	{
		root = root6;
	}
	
	if (!root)
		return;

	ifSailTableBuild = true;

	cout << "Enter estimateMem..." << endl;
	estimateMem(type);

	preVisit(root);
}

void FibTrie::levelArrLookup(unsigned long long IP64, unsigned int & nextHop)
{
	register int level_16_offset = 0;
	register int level_24_offset = 0;
	register int level_32_offset = 0;
	register int level_40_offset = 0;
	register int level_48_offset = 0;
	register int level_64_offset = 0;

	level_16_offset =  IP64 >> 48;	 // level_16 table lookup

	if (levelTables[0].table[level_16_offset].flag)
		nextHop = levelTables[0].table[level_16_offset].offset;
	else
	{
		level_24_offset = (levelTables[0].table[level_16_offset].offset << 8) + (IP64 << 16 >> 56);				// level_24 table lookup

		if (levelTables[1].table[level_24_offset].flag)
			nextHop = levelTables[1].table[level_24_offset].offset;
		else
		{
			level_32_offset = (levelTables[1].table[level_24_offset].offset << 8) + (IP64 << 24 >> 56);			// level_32 table lookup

			if (levelTables[2].table[level_32_offset].flag)
				nextHop = levelTables[2].table[level_32_offset].offset;
			else
			{
				level_40_offset = (levelTables[2].table[level_32_offset].offset << 8) + (IP64 << 32 >> 56);		// level_40 table lookup
				
				if (levelTables[3].table[level_40_offset].flag)
					nextHop = levelTables[3].table[level_40_offset].offset;
				else
				{
					level_48_offset = (levelTables[3].table[level_40_offset].offset << 8) + (IP64 << 40 >> 56);	// level_48 table lookup

					if (splitTables[0].flag[level_48_offset])
						nextHop = splitTables[0].offset[level_48_offset];
					else
					{
						level_64_offset = (splitTables[0].offset[level_48_offset] << 16) + (IP64 << 48 >> 48);	// level_64 table lookup
						nextHop = splitTables[1].offset[level_64_offset];
					}
				}
			}
		}
	}
}

void FibTrie::trieDestroy(FibNode* root)
{
	if (root == NULL)
		return;
	
	trieDestroy(root->lchild);
	trieDestroy(root->rchild);

	free(root);
}

void FibTrie::fibTrieDestroy()
{
	trieDestroy(root4);
	trieDestroy(root6);

	cout << "The trie has been destroyed!" << endl;
}

void FibTrie::LookupspeedTest(const char * tracefile, unsigned int trieType)
{
	int ln = 0;
	int test_num = 0;
	unsigned long long ip = 0;
	unsigned int nextHop = 0;

	char buf[MAX_BUF_LEN];
	char binaryString[MAX_BUF_LEN];

	FILE* inFile = fopen (tracefile, "r");


	if (!inFile) {
		cout << "Open file: " << tracefile << " error!" << endl;
		return;           
	}

	if (fgets(buf, MAX_BUF_LEN, inFile))
	{
		sscanf(buf, "%u", &test_num);
	}

	unsigned int counter=0;
	unsigned int traffic[TRACE_READ];
	while (fgets (buf, MAX_BUF_LEN, inFile)) {	// read a line
		sscanf (buf, "%llu", &traffic[counter]);
		if (counter++>TRACE_READ)break;
	}

	nextHop = 0;

	int cycleNum=10000;
	LARGE_INTEGER frequence,privious,privious1;
	if(!QueryPerformanceFrequency(&frequence))return;
	QueryPerformanceCounter(&privious);
	printf("frequency=%u\n",frequence.QuadPart);//2825683


	for (register int j=0;j<cycleNum;j++)
	{
		for (register int i=0;i<TRACE_READ;i++)
		{
			levelArrLookup(traffic[i], nextHop);
		}
	}


	QueryPerformanceCounter(&privious1);
	long long Lookuptime=1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;

	printf("nextHop=%d\nLookuptime = \t%u\npps is \t%.3f (Mpps)\n",nextHop,Lookuptime, TRACE_READ*cycleNum/(Lookuptime+0.0));
	fclose (inFile);


}

void FibTrie::verify(const char *inPath, const char *outPath, unsigned int vType, unsigned int trieType)
{
	int ln = 0;
	int test_num = 0;
	unsigned long long ip = 0;
	unsigned int nextHop = 0;

	char buf[MAX_BUF_LEN];
	char binaryString[MAX_BUF_LEN];

	FILE* inFile = fopen (inPath, "r");
	FILE* outFile = fopen (outPath, "w");
	
	if (!inFile) {
		cout << "Open file: " << inPath << " error!" << endl;
		return;           
	}

	if (!outFile) {
		cout << "Open file: " << outPath << " error!" << endl;
		return;           
	}

	if (fgets(buf, MAX_BUF_LEN, inFile))
	{
		sscanf(buf, "%u", &test_num);
		fprintf(outFile, "%u\n", test_num);
	}

	while (fgets (buf, MAX_BUF_LEN, inFile)) {	// read a line

		ln = 0;
		nextHop = 0;

		sscanf (buf, "%llu", &ip);

		if ((vType == COMM) || (vType == LEVELPUSH && ifLevelPushed))
		{
			char *prefix = itobs(ip, binaryString);

			if (strlen(prefix) > 0)
			{
				//cout << prefix << "	" << nextHop << endl;
				ln = lookup(prefix, strlen(prefix), trieType, nextHop);
				fprintf(outFile, "%llu : %u\n", ip, nextHop);
			}
		}
		else if (vType == SAILTABLE && ifSailTableBuild)
		{
			levelArrLookup(ip, nextHop);
			fprintf(outFile, "%llu : %u\n", ip, nextHop);
		}
	}

	fclose (inFile);
	fclose (outFile);
}

char* FibTrie::itobs(unsigned long long n, char *ps)
{
    int i;
	int flag = 0;

	int size = 8 * sizeof(unsigned long long);

	for( i = size - 1; i >= 0; i--,n >>= 1)
	{
		ps[i]=(01 & n) + '0';
	}

    ps[size]='\0';
    
	return ps;
}

unsigned long long FibTrie::memStat()
{
	int i = 0;

	totalMem = 0;

	for (i = 0; i < TABLE_NUM; i++)
	{
		totalMem += levelTables[i].total * 17;
	}

	totalMem += splitTables[0].total * 1;	// flag 1 bit for level 48
	//totalMem += splitTables[1].total * 1;	// flag 1 bit for level 64

	return totalMem;
}

void FibTrie::performance()
{
	memStat();
}

void FibTrie::generateReport(char* trace, char* route)
{
	int i = 0;

	if (!report)
		return;

	performance();

	fprintf(report, "****************************************************************\n");
	fprintf(report, "Trace: %s\n", trace);
	fprintf(report, "Routing Table: %s\n", route);
	fprintf(report, "\n\t\t\tStatistical Results\n\n");
	fprintf(report, "\n\t\tOrignal Trie Statistical Results\n\n");

	for (i = 0; i < 6; i ++)
	{
		fprintf(report, "Level %-10u Stat:\t%-10u\t%-10u\t%-10u\n", levelIndex[i], origLevelStat[i][0], origLevelStat[i][1], origLevelStat[i][2]);
	}

	fprintf(report, "\n\n\t\tLevel Pushed Trie Statistical Results\n\n");
	for (i = 0; i < TABLE_NUM; i++)
	{
		fprintf(report, "Level %-10u Stat:\t%-10u\t%-10u\t%-10u\n", levelIndex[i], levelTables[i].total, levelTables[i].interCount, levelTables[i].leafCount);
	}

	int tempL[2] = {48, 64};
	fprintf(report, "Level %-10u Stat:\t%-10u\t%-10u\t%-10u\n", tempL[0], splitTables[0].total, splitTables[0].interCount, splitTables[0].leafCount);
	fprintf(report, "Level %-10u Stat:\t%-10u\t%-10u\t%-10u\n", tempL[1], splitTables[1].total, splitTables[1].interCount, splitTables[1].leafCount);

	cout << "memory:" << totalMem << endl;
	cout << "memory:" <<(1.0 * totalMem)/(1.0 * MB) << " (MB)" << endl;

	fprintf(report, "The sail tables consume: %-5.3f MB!!!\n", ((1.0 * totalMem)/(1.0 * MB)));
	fprintf(report, "****************************************************************\n\n");
}