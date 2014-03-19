
#pragma once

//#define RUN_ON_GPU

#ifdef RUN_ON_GPU
#include "TreeGPU.h"
#endif


#define TRACE_READ 100000
#define _NOT_DELETE	0
#define _DELETE	2
#define _CHANGE	3
#define LENGTH_24	35535*256
//#define SIZEOF2_24	16777216
#define  H1_LEN 9949801//8677633 49999*199 
#define  H2_LEN 524288*4 //2^19
#define PORT_MAX	100
#include <windows.h>
#include <fstream>

using namespace std;


struct element_16 {
	unsigned short nexthop;
	bool flag;
};

struct Table16 {
	element_16* data;	
	int size;			
	void initWithSize (int n) {
		size = n;
		data = new element_16[size];
	}
	void destroy () {	
		if (!data) return;
		delete[] data;
		data = NULL;
	}

	// write code in this function
	void initWithFile (const char* path) {};
};

//////////////////////////////////
// table_24
/////////////////////////////////
typedef unsigned char element_24;

struct Table24 {
	element_24* data;
	int size;
	void initWithSize (int n) {
		size = n;
		data = new element_24[size];
	}
	void destroy () {
		if (!data) return;
		delete[] data;
		data = NULL;
	}

	// write code in this function
	void initWithFile (const char* path) {};
};

struct IPNode{
	unsigned int IP;
	unsigned char nexthop;
};

struct IPNodeList 
{
	unsigned int IP;
	unsigned char nexthop;
	struct IPNodeList * pointer;
};

typedef int element_hashTable32; // modify this using your definiions

struct HashTable32 {
	
	IPNode *data1;//HashTable_1[H1_LEN];
	IPNodeList *data2;//HashTable_2[H2_LEN];

	int size1;
	int size2;
	void initWithSize (int n1,int n2) {
		size1 = n1;
		size2 = n2;
		data1 = new IPNode[size1];
		data2 = new IPNodeList[size2];
	}
	void destroy () {

		if (!data1) return;
		delete[] data1;
		data1 = NULL;

		if (!data2) return;
		delete[] data2;
		data1 = NULL;
	}

	// write code in this function
	void initWithFile (const char* path) {};
};


struct FibTrie
{
	FibTrie*			parent;					//point to father node
	FibTrie*			lchild;					//point to the left child(0)
	FibTrie*			rchild;					//point to the right child(1)
	int					newPort;					
	int					oldPort;	
	bool				ifpushed;				//if the missing node.
};

class CFib
{
public:
	FibTrie* m_pTrie;				//root node of FibTrie
	int allNodeCount;				//to count all the nodes in Trie tree, including empty node
	int solidNodeCount;				//to count all the solid nodes in Trie tree
	int LevelPort[40][200];


	element_16 *ele16;			
	unsigned char *ele24;				
	//unsigned char *HashTable24;	
	IPNode * hTable_1;			
	IPNodeList *hTable_2;		

	unsigned int currentLenBit24;	
	unsigned int prefix32_num;		
	unsigned int lenofchain;	

	CFib(void);
	~CFib(void);

	// creat a new FIBTRIE ndoe 
	void CreateNewNode(FibTrie* &pTrie);
	//get the total number of nodes in RibTrie  
	void ytGetNodeCounts();
	void Pretraversal(FibTrie* pTrie);
	//output the result
	void OutputTrie(FibTrie* pTrie,string sFileName,string oldPortfile);
	void CFib::OutputTrie_32(FibTrie* pTrie);
	bool IsLeaf(FibTrie * pNode);
private:
	//get and output all the nexthop in Trie
	void CFib::GetTrieHops(FibTrie* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort);
	void CFib::GetTrieHops_32(FibTrie* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort);
public:
	unsigned int BuildFibFromFile(string sFileName);
	void AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop);
	void CFib::LeafPush(FibTrie* pTrie, int depth);

	void CFib::ytLevelPushing(FibTrie* pTrie, unsigned int level,unsigned int default_port);

	int num_level[50];

	void CFib::LevelStatistic(FibTrie* pTrie, unsigned int level);
	//OH algorithm
	void CFib::OHConstruction(FibTrie* pTrie, int level, char *prefix);
	void CFib::Buildbitmap24(FibTrie* pTrie,int level, char *prefix);
	unsigned int CFib::btod(char *bstr);
	//int CFib::dtob(unsigned int d, char *bstr);
	void CFib::outputTwoArray();
	//int CFib::OHLookup();
	unsigned int CFib::TWMX( unsigned int a);
	//void CFib::outputHashTable24();
	//void CFib::HashTable24_Construction(FibTrie* pTrie,unsigned int iVal,int iBitLen);
	void CFib::HashTable32_Construction(FibTrie* pTrie,unsigned int iVal,int iBitLen);
	int CFib::HashTableConstructor();
	int CFib::HashTableInsert( IPNode *InsertIPNode);
};


//////////////////////////////////
// lookup logic
//////////////////////////////////
#ifdef RUN_ON_GPU
unsigned int __device__ hash (element_hash* hash_table, int key, bool flag) {
	return 0;
}

void __global__ lookupKernelTable (element_16* table_16, element_24* table_24, element_hashTable24* hash_table24, element_hashTable32* hash_table32, unsigned int* result, unsigned int* traffic, int size) {
	int step = gridDim.x * blockDim.x;
	unsigned int address, nexthop;
	unsigned int addr_16, addr_8;

#pragma unroll
	for (int i = blockIdx.x * blockDim.x + threadIdx.x ; i < size ; i += step) {
		address = traffic[i];
		
		addr_16 = address >> 16;
		nexthop = table_16[addr_16].nexthop;
		if (!table_16[addr_16].flag) {
			addr_8 = (address & 0xffff) >> 8;
			nexthop = hash (hash_table, address, table_24[nexthop << 8 | addr_8]);
		}

		result[i] = nexthop;
	} 
}
#else

#endif

////////////////////////////////////////////
/// main class
////////////////////////////////////////////
class ThreeTable{

public:
	Table16 table_16;
	Table24 table_24;
	HashTable32 hash_table32;
	//HashTable24 hash_table24;

	// data on gpu
	element_16* table_16_gpu;
	element_24* table_24_gpu;
	//element_hashTable24* hash_table24_gpu;
	element_hashTable32* hash_table32_gpu;

public:
	ThreeTable () {
		//initTree ();
	}
	ThreeTable (const char* t16_path, const char* t24_path, const char* ht_path) {
		initTableWithFile (t16_path, t24_path, ht_path);
	}
	~ThreeTable () {
		destroy ();
	}

	void initTree (element_16 *ele16, element_24 *ele24,unsigned int currentLenBit24,IPNode* data1,IPNodeList *data2) {
		table_16.data = ele16;
		table_24.data = ele24;
		//hash_table24.data = htable24;
		hash_table32.data1 = data1;
		hash_table32.data2 = data2;

		table_16_gpu = NULL;
		table_24_gpu = NULL;
		//hash_table24_gpu = NULL;
		hash_table32_gpu = NULL;
	}

	void destroy () {
		table_16.destroy ();
		table_24.destroy ();
		//hash_table24.destroy ();
		hash_table32.destroy ();

#ifdef RUN_ON_GPU
		if(table_16_gpu) cudaFree(table_16_gpu);
		if(table_24_gpu) cudaFree(table_24_gpu);
		if(hash_table_gpu) cudaFree(hash_table_gpu);
#endif
	}

	void initTableWithFile (const char* t16_path, const char* t24_path, const char* ht_path) {

		//initTree ();

		table_16.initWithFile (t16_path);
		table_24.initWithFile (t24_path);
		//hash_table24.initWithFile (ht_path);
		hash_table32.initWithFile (ht_path);

		copyDataH2D ();
	}

	void copyDataH2D () {
		printf ("ThreeTable Copy\n");
#ifdef RUN_ON_GPU
		cudaMalloc ((void**)&table_16_gpu, sizeof (element_16) * table_16.size);
		cudaMalloc ((void**)&table_24_gpu, sizeof (element_24) * table_24.size);
		cudaMalloc ((void**)&hash_table_gpu, sizeof (element_hash) * hash_table.size);
		cudaMemcpy (table_16_gpu, table_16.data, sizeof (element_16) * table_16.size, cudaMemcpyHostToDevice);
		cudaMemcpy (table_24_gpu, table_24.data, sizeof (element_24) * table_24.size, cudaMemcpyHostToDevice);
		cudaMemcpy (hash_table_gpu, hash_table.data, sizeof (element_hash) * hash_table.size, cudaMemcpyHostToDevice);
#endif
	}

#ifdef RUN_ON_GPU
	double runLookupV4 (LLNextHopP hostResult, LLTraceV4 hostTrace, LLCUDA cfg, cudaStream_t* gpuStream) {
		int stream = cfg.maxStream;
		int block  = cfg.maxBlock;
		int thread = cfg.maxThread;
		//printf ("Lookup GALE, %d %d %d\n", stream, block, thread);

		IPV4*      deviceTrace;
		LLNextHopP deviceResult;

		cudaMalloc ((void**)&deviceTrace,  hostTrace.size * sizeof(IPV4));
		cudaMalloc ((void**)&deviceResult, hostTrace.size * sizeof(LLNextHop));

		int urlPerStream    = hostTrace.size / stream;
		int bytesPerStreamR = urlPerStream * sizeof (LLNextHop);
		int bytesPerStreamT = urlPerStream * sizeof (IPV4);

		startTimer ();

		for (int i = 0 ; i < stream ; i ++) {
			cudaMemcpyAsync (deviceTrace + i * urlPerStream, hostTrace.data + i * urlPerStream, bytesPerStreamT, cudaMemcpyHostToDevice, gpuStream[i]);
			lookupKernelTable<<<block, thread, 0, gpuStream[i]>>> (m_pGPUTable, deviceResult + i * urlPerStream, deviceTrace + i * urlPerStream, urlPerStream);
			cudaMemcpyAsync (hostResult + i * urlPerStream, deviceResult + i * urlPerStream, bytesPerStreamR, cudaMemcpyDeviceToHost, gpuStream[i]);
		}
		
		float time_used = endTimer ();

		cudaThreadSynchronize();

		cudaFree (deviceTrace);
		cudaFree (deviceResult);

		return time_used;
	}
#else 
	void runLookupV4 (const char* trace_path, int size, const char * ribfileName) {
		
		// read traffic from file
		unsigned int traffic[TRACE_READ];
		int return_value=-1;
		unsigned int traceNum=0;

		//first read the trace...
		ifstream fin(trace_path);
		if (!fin)return;
		fin>>traceNum;

		int TraceLine=0;
		int IPtmp=0;
		while (!fin.eof() && TraceLine<TRACE_READ )
		{
			fin>>IPtmp;
			traffic[TraceLine]=IPtmp;
			TraceLine++;
		}
		fin.close();
		printf("trace read complete...\n");

		if (TraceLine<TRACE_READ)
		{
			printf("not enough\n",TraceLine);
		}


		CFib tFib= CFib();		//build FibTrie
		unsigned int iEntryCount = 0;
		iEntryCount=tFib.BuildFibFromFile(ribfileName);
		tFib.ytGetNodeCounts();
		printf("\nThe total number of routing items in FRib file is \t%u, \nThe total number of solid Trie node is :\t%u,\ntFib.allNodeCount=%d\n",iEntryCount,tFib.solidNodeCount,tFib.allNodeCount);
		//level-pushing
		int default_port=PORT_MAX;
		if (tFib.m_pTrie->newPort>0)default_port=tFib.m_pTrie->newPort;

		tFib.ytLevelPushing(tFib.m_pTrie,0,default_port);

		tFib.OHConstruction(tFib.m_pTrie,0,"");
		//tFib.HashTable24_Construction(tFib.m_pTrie,0,0);
		tFib.HashTable32_Construction(tFib.m_pTrie,0,0);
		//tFib.outputHashTable24();
		printf("lenofchain is %u\n The prefixes32 num is %u\n",tFib.lenofchain,tFib.prefix32_num);

		initTree(tFib.ele16,tFib.ele24,tFib.currentLenBit24,tFib.hTable_1,tFib.hTable_2);

		printf("Lookup table constuction complete!\n");
		/////////////////////////

		unsigned int* result = new unsigned int[size];
		lookupKernelTable (table_16.data, table_24.data,  hash_table32.data1,hash_table32.data2, result, traffic, size);
		// check result

		delete[] result;
	}

	void lookupKernelTable (element_16* ele16, element_24* ele24, IPNode * hashtable_1, IPNodeList *hashtable_2, unsigned int* result, unsigned int* traffic, int size) 
	{
		register unsigned char LPMPort=0;
		register unsigned int taffic_Integer=0;

		LARGE_INTEGER frequence,privious,privious1;
		if(!QueryPerformanceFrequency(&frequence))return;
		QueryPerformanceCounter(&privious);
		printf("frequency=%u\n",frequence.QuadPart);//2825683
		
		for (register int j=0;j<10000;j++)
		{
			for (register int i=0;i<TRACE_READ;i++)
			{
				taffic_Integer=traffic[i];
				if(true==ele16[taffic_Integer>>16].flag)LPMPort=ele16[taffic_Integer>>16].nexthop;
				else if (LPMPort=ele24[(ele16[taffic_Integer>>16].nexthop<<8)+(taffic_Integer<<16>>24)]){}
				else LPMPort=HashTableLookup(traffic[i]);
			}
		}
		
		QueryPerformanceCounter(&privious1);
		long long Lookuptime=1000000*(privious1.QuadPart-privious.QuadPart)/frequence.QuadPart;

		printf("Lookup time=%u\n",Lookuptime);
	}


	unsigned char HashTableLookup( unsigned int IncomingIP)
	{
		//unsigned int IncomingIP = TWMX(InsertIPNode.IP);
		int index1 = IncomingIP%H1_LEN;
		if (hash_table32.data1[index1].IP == IncomingIP)return hash_table32.data1[index1].nexthop;
		else 
		{
			//int index2 =IncomingIP%H2_LEN;
			IPNodeList* pNode = hash_table32.data2+IncomingIP%H2_LEN;
			while(pNode!= NULL)
			{
				if (pNode->IP==IncomingIP)return pNode->nexthop;
				pNode = pNode->pointer;
			}
		}
		return 0;
	}


	void PrepareToLookup(const char * ribfileName)
	{

		CFib tFib= CFib();		//build FibTrie
		unsigned int iEntryCount = 0;
		iEntryCount=tFib.BuildFibFromFile(ribfileName);
		tFib.ytGetNodeCounts();
		printf("\nThe total number of routing items in FRib file is \t%u, \nThe total number of solid Trie node is :\t%u,\ntFib.allNodeCount=%d\n",iEntryCount,tFib.solidNodeCount,tFib.allNodeCount);
		
		//level-pushing
		int default_port=PORT_MAX;
		if (tFib.m_pTrie->newPort>0)default_port=tFib.m_pTrie->newPort;
		tFib.ytLevelPushing(tFib.m_pTrie,0,default_port);

		//tFib.LevelPushing(tFib.m_pTrie->rchild,1);
		tFib.OHConstruction(tFib.m_pTrie,0,"");
		//tFib.HashTable24_Construction(tFib.m_pTrie,0,0);
		tFib.HashTable32_Construction(tFib.m_pTrie,0,0);
		//tFib.outputHashTable24();
		printf("lenofchain is %u\n The prefixes32 num is %u\n",tFib.lenofchain,tFib.prefix32_num);

		initTree(tFib.ele16,tFib.ele24,tFib.currentLenBit24,tFib.hTable_1,tFib.hTable_2);
	}


	unsigned int LookupAnIP(unsigned int taffic_Integer)
	{
		register unsigned char nexthop24=0;
		unsigned int LPMPort;
	
		if(true==table_16.data[taffic_Integer>>16].flag){
			LPMPort=table_16.data[taffic_Integer>>16].nexthop;
		}
		else if (nexthop24=table_24.data[(table_16.data[taffic_Integer>>16].nexthop<<8)+(taffic_Integer<<16>>24)]){
			LPMPort=nexthop24;
		}
		else {
			LPMPort=HashTableLookup(taffic_Integer);
		}
		return LPMPort;

	}
#endif
};