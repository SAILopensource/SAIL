/*
 * Fib.cpp
 *
 *  Created on: 2011-4-6
 *      Author: root
 */

#include "3Table.h"
#include <iostream>


#define FIBLEN				sizeof(struct FibTrie)		//size of each Trie node
#define EMPTYHOP			0							//Trie node doesnt't has a next hop
#define HIGHTBIT			2147483648					//Binary: 10000000000000000000000000000000


char * result_compare="result_compare.txt";
char * hop_count="hop_count.txt";
CFib::CFib(void)
{
	//initial the root of the Trie tree
	CreateNewNode(m_pTrie);

	allNodeCount=0;			//to count all the nodes in Trie tree, including empty node
	solidNodeCount=0;		//to count all the solid nodes in Trie tree


	prefix32_num=0;
	lenofchain=0;

	memset(num_level, 0, sizeof(num_level));

	memset(LevelPort,0,sizeof(LevelPort));

	ele16=new element_16[65536];
	ele24=new unsigned char[LENGTH_24];
	currentLenBit24=0;

	for (int i=0;i<65536;i++)
	{
		ele16[i].flag=false;
		ele16[i].nexthop=0;
	}

	for (int i=0;i<LENGTH_24;i++)
	{
		ele24[i]=false;
	}

	//HashTable24=new unsigned char[SIZEOF2_24];
	//for (int i=0;i<SIZEOF2_24;i++)
	//{
	//	HashTable24[i]=0;
	//}

	hTable_1=new IPNode[H1_LEN];
	for (int i=0;i<H1_LEN;i++)
	{
		hTable_1[i].IP=0;
		hTable_1[i].nexthop=0;
	}

	hTable_2=new IPNodeList[H2_LEN];
	for (int i=0;i<H2_LEN;i++)
	{
		hTable_2[i].IP=0;
		hTable_2[i].nexthop=0;
		hTable_2[i].pointer=NULL;
	}
}

CFib::~CFib(void)
{
}

//creat new node 
void CFib::CreateNewNode(FibTrie* &pTrie)
{
	
	pTrie= (struct FibTrie*)malloc(FIBLEN);

	//initial
	pTrie->parent = NULL;
	pTrie->lchild = NULL;
	pTrie->rchild = NULL;
	pTrie->newPort = EMPTYHOP;
	pTrie->oldPort = EMPTYHOP;
	pTrie->ifpushed= false;
}



unsigned int CFib::btod(char *bstr)
{
	unsigned int d = 0;
	unsigned int len = strlen(bstr);
	if (len > 32)
	{
		printf("too long\n");
		return -1; 
	}
	len--;

	unsigned int i = 0;
	for (i = 0; i <= len; i++)
	{
		d += (bstr[i] - '0') * (1 << (len - i));
	}

	return d;
}
 

bool CFib::IsLeaf(FibTrie * pNode)
{
	if (pNode->lchild==NULL && pNode->rchild==NULL)return true;
	else return false;	
}


void CFib::Pretraversal(FibTrie* pTrie)
{
	if (NULL==pTrie)return;

	allNodeCount++;
	if (pTrie->newPort!=0)solidNodeCount++;


	Pretraversal(pTrie->lchild);
	Pretraversal(pTrie->rchild);
}
void CFib::ytGetNodeCounts()
{
	allNodeCount=0;
	solidNodeCount=0;

	Pretraversal(m_pTrie);
}

void CFib::OutputTrie(FibTrie* pTrie,string sFileName,string oldPortfile)
{
	ofstream fout(sFileName.c_str());
	GetTrieHops(pTrie,0,0,&fout,true);
	fout<<flush;
	fout.close();

	ofstream fout1(oldPortfile.c_str());
	GetTrieHops(pTrie,0,0,&fout1,false);
	fout1<<flush;
	fout1.close();
}

void CFib::OutputTrie_32(FibTrie* pTrie)
{
	ofstream fout("Prefixes_32.txt");
	GetTrieHops_32(pTrie,0,0,&fout,true);
	fout<<flush;
	fout.close();
}



int CFib::HashTableInsert( IPNode *InsertIPNode)
{
	unsigned int tmp = InsertIPNode->IP;
	int index1 = tmp%H1_LEN;
	if (0 == hTable_1[index1].IP)
	{
		hTable_1[index1].IP = InsertIPNode->IP;
		hTable_1[index1].nexthop = InsertIPNode->nexthop;
		return 0;
	}
	else if(hTable_1[index1].IP != InsertIPNode->IP)
	{
		int index2 =tmp%H2_LEN;
		if (0 == hTable_2[index2].IP)
		{
			hTable_2[index2].IP = InsertIPNode->IP;
			hTable_2[index2].nexthop = InsertIPNode->nexthop;
			return 1;
		}
		else if(hTable_2[index2].IP != InsertIPNode->IP )
		{
			IPNodeList* pNode = hTable_2+index2;


			while(pNode->pointer != NULL)
			{
				if (pNode->IP==InsertIPNode->IP)
				{
					return 5; 
				}
				pNode = pNode->pointer;
				//printf("\n%s" , pNode);
			}

			IPNodeList* newIPNode = new IPNodeList();//(IPNodeList *)malloc(1,sizeof(IPNodeList));
			newIPNode->IP = InsertIPNode->IP;
			newIPNode->nexthop = InsertIPNode->nexthop;
			newIPNode->pointer = NULL;

			pNode->pointer = newIPNode;
			lenofchain++;
			return 3;
		}
		return 5;
	}
	return 5; 
}


void CFib::HashTable32_Construction(FibTrie* pTrie,unsigned int iVal,int iBitLen)
{
	unsigned char portOut=PORT_MAX;

	if (-1!=pTrie->newPort)
	{
		portOut=pTrie->newPort;
	}

	IPNode NewIPNode;	 
	if(portOut!=EMPTYHOP  && 32==iBitLen )
	{
		NewIPNode.IP = iVal;
		NewIPNode.nexthop = portOut;
		HashTableInsert(&NewIPNode);
		prefix32_num++;
	}

	iBitLen++;

	//try to handle the left sub-tree
	if(pTrie->lchild!=NULL)
	{
		HashTable32_Construction(pTrie->lchild,iVal,iBitLen);
	}
	//try to handle the right sub-tree
	if(pTrie->rchild!=NULL)
	{
		iVal += 1<<(32-iBitLen);
		HashTable32_Construction(pTrie->rchild,iVal,iBitLen);
	}
}

void CFib::GetTrieHops_32(FibTrie* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort)
{
	unsigned short portOut=PORT_MAX;

	if (-1!=pTrie->newPort)
	{
		portOut=pTrie->newPort;
	}

	if(portOut!=EMPTYHOP  && 32==iBitLen )
	{
		*fout<<iVal<<"\t"<<portOut<<endl;
	}

	iBitLen++;

	//try to handle the left sub-tree
	if(pTrie->lchild!=NULL)
	{
		GetTrieHops_32(pTrie->lchild,iVal,iBitLen,fout,ifnewPort);
	}
	//try to handle the right sub-tree
	if(pTrie->rchild!=NULL)
	{
		iVal += 1<<(32-iBitLen);
		GetTrieHops_32(pTrie->rchild,iVal,iBitLen,fout,ifnewPort);
	}
}
//get and output all the nexthop in Trie
void CFib::GetTrieHops(FibTrie* pTrie,unsigned int iVal,int iBitLen,ofstream* fout,bool ifnewPort)
{
	
	int portOut=-1;
	if (true==ifnewPort)
		portOut=pTrie->newPort;
	else				
		portOut=pTrie->oldPort;
	

	//1 00000000  00010000   00000000
	if(portOut!=EMPTYHOP)
	{
		char strVal[50];
		memset(strVal,0,sizeof(strVal));
		//printf("%d.%d.%d.%d/%d\t%d\n",(iVal>>24),(iVal<<8)>>24,(iVal<<16)>>24,(iVal<<24)>>24,iBitLen,portOut);

		sprintf(strVal,"%d.%d.%d.%d/%d\t%d\n",(iVal>>24),(iVal<<8)>>24,(iVal<<16)>>24,(iVal<<24)>>24,iBitLen,portOut);
		*fout<<strVal;
	}
	
	iBitLen++;

	//try to handle the left sub-tree
	if(pTrie->lchild!=NULL)
	{
		GetTrieHops(pTrie->lchild,iVal,iBitLen,fout,ifnewPort);
	}
	//try to handle the right sub-tree
	if(pTrie->rchild!=NULL)
	{
		iVal += 1<<(32-iBitLen);
		GetTrieHops(pTrie->rchild,iVal,iBitLen,fout,ifnewPort);
	}
}

//add a node in Rib tree
void CFib::AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop)
{
	
	//get the root of rib
	FibTrie* pTrie = m_pTrie;
	//locate every prefix in the rib tree
	for (unsigned int i=0; i<iPrefixLen; i++){
		//turn right
		if(((lPrefix<<i) & HIGHTBIT)==HIGHTBIT){
			//creat new node
			if(pTrie->rchild == NULL){
				FibTrie* pTChild = (struct FibTrie*)malloc(FIBLEN);
				//insert new node
				pTChild->parent = pTrie;
				pTChild->lchild = NULL;
				pTChild->rchild = NULL;
				pTChild->oldPort=0;
				pTChild->newPort=0;
				pTChild->ifpushed=false;

				pTrie->rchild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->rchild;

		}
		//turn left
		else{
			//if left node is empty, creat a new node
			if(pTrie->lchild == NULL){
				FibTrie* pTChild = (struct FibTrie*)malloc(FIBLEN);
				//insert new node
				pTChild->parent = pTrie;
				pTChild->lchild = NULL;
				pTChild->rchild = NULL;
				pTChild->oldPort=0;
				pTChild->newPort=0;
				pTChild->ifpushed=false;

				pTrie->lchild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->lchild;
		}
	}

	pTrie->newPort = iNextHop;
	pTrie->oldPort = iNextHop;
}

	/*
	*PURPOSE: construct RIB tree from file
	*RETURN VALUES: number of items in rib file
	*/
unsigned int CFib::BuildFibFromFile(string sFileName)
{
	unsigned int	iEntryCount=0;		//the number of items from file

	char			sPrefix[100];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of PREFIX
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file

	
	ifstream fin(sFileName.c_str());
	while (!fin.eof()) {

		
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		memset(sPrefix,0,sizeof(sPrefix));
		
		fin >> sPrefix>> iNextHop;

		int iStart=0;				//the start point of PREFIX
		int iEnd=0;					//the start point of PREFIX
		int iFieldIndex = 3;		
		int iLen=(int)strlen(sPrefix);	//The length of PREFIX

		if (iLen>20)
		{
			continue;//maybe IPv6 address
		}
		
		if(iLen>0){
			iEntryCount++;
			for ( int i=0; i<iLen; i++ ){
				//get the first three sub-items
				if ( sPrefix[i] == '.' ){
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex);
					iFieldIndex--;
					iStart = i+1;
					i++;
				}
				if ( sPrefix[i] == '/' ){
					//get the prefix length
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str());
					iStart = i+1;

					
					i++;
					strVal= string(sPrefix+iStart,iLen-1);
					iPrefixLen=atoi(strVal.c_str());
				}
			}
		
			AddNode(lPrefix,iPrefixLen,iNextHop);
		}
	}
	fin.close();
	return iEntryCount;
}



void CFib::LeafPush(FibTrie* pTrie, int depth)
{

	if (NULL==pTrie)return;

	if (NULL==pTrie->lchild && NULL==pTrie->rchild)return;

	if (0==pTrie->newPort)
	{
		LeafPush(pTrie->lchild,depth);
		LeafPush(pTrie->rchild,depth);
		return;
	}

	if (NULL!=pTrie->lchild && NULL==pTrie->rchild)
	{
		FibTrie* pTChild = (struct FibTrie*)malloc(FIBLEN);
		pTChild->parent = pTrie;
		pTChild->lchild = NULL;
		pTChild->rchild = NULL;
		pTChild->oldPort= 0;
		pTChild->newPort= pTrie->newPort;
		pTChild->ifpushed= true;

		pTrie->rchild=pTChild;

		if (0==pTrie->lchild->newPort)pTrie->lchild->newPort=pTrie->newPort;

		LeafPush(pTrie->lchild,depth);

		pTrie->newPort=0;
	}

	else if (NULL!=pTrie->rchild && NULL==pTrie->lchild)
	{
		FibTrie* pTChild = (struct FibTrie*)malloc(FIBLEN);
		pTChild->parent = pTrie;
		pTChild->lchild = NULL;
		pTChild->rchild = NULL;
		pTChild->oldPort=0;
		pTChild->newPort=pTrie->newPort;
		pTChild->ifpushed= true;

		pTrie->lchild=pTChild;

		if (0==pTrie->rchild->newPort)pTrie->rchild->newPort=pTrie->newPort;

		LeafPush(pTrie->rchild,depth);

		pTrie->newPort=0;
	}

	else 
	{
		if (0==pTrie->rchild->newPort)pTrie->rchild->newPort=pTrie->newPort;
		if (0==pTrie->lchild->newPort)pTrie->lchild->newPort=pTrie->newPort;

		LeafPush(pTrie->lchild,depth);
		LeafPush(pTrie->rchild,depth);

		pTrie->newPort=0;
	}
}


void CFib::LevelStatistic(FibTrie* pTrie, unsigned int level)
{
	if(NULL == pTrie)return;
	if(pTrie->newPort != 0)num_level[level]++;

	LevelStatistic(pTrie->lchild, level+1);
	LevelStatistic(pTrie->rchild, level+1);
}

void CFib::ytLevelPushing(FibTrie* pTrie, unsigned int level,unsigned int default_port)
{
	if(NULL == pTrie)return;

	if((level == 16 || level == 24 || level == 32) && IsLeaf(pTrie))return;

	if (pTrie->newPort>0)default_port=pTrie->newPort;

	//left child
	if (NULL==pTrie->lchild)
	{
		FibTrie* pTChild  = new FibTrie();//= (struct FibTrie*)malloc(FIBLEN);
		if (NULL==pTChild)
		{
			printf("malloc faild");
		}
		pTChild->parent = pTrie;
		pTChild->lchild = NULL;
		pTChild->rchild = NULL;
		pTChild->oldPort=0;
		pTChild->newPort=pTrie->newPort;
		pTChild->ifpushed= true;
		pTrie->lchild=pTChild;
	}
	else if (0==pTrie->lchild->newPort)pTrie->lchild->newPort=default_port;

	//right child
	if (NULL==pTrie->rchild)
	{
		FibTrie* pTChild = new FibTrie();//(struct FibTrie*)malloc(FIBLEN);
		if (NULL==pTChild)
		{
			printf("malloc faild");
		}
		pTChild->parent = pTrie;
		pTChild->lchild = NULL;
		pTChild->rchild = NULL;
		pTChild->oldPort=0;
		pTChild->newPort=pTrie->newPort;
		pTChild->ifpushed= true;
		pTrie->rchild=pTChild;
	}
	else if (0==pTrie->rchild->newPort)pTrie->rchild->newPort=default_port;

	ytLevelPushing(pTrie->lchild, level+1,default_port);
	ytLevelPushing(pTrie->rchild, level+1,default_port);
}

void CFib::OHConstruction(FibTrie* pTrie, int level, char *prefix)
{
	if (NULL==pTrie || level>16)return;
	
	if (16==level)
	{
		unsigned int num=btod(prefix);
		if (IsLeaf(pTrie))
		{
			ele16[num].flag=true;
			ele16[num].nexthop=pTrie->newPort;
			if (-1==pTrie->newPort)
			{
				ele16[num].nexthop=PORT_MAX;
			}
		}
		else
		{
			ele16[num].flag=false;
			if (0==currentLenBit24)
			{
				int jjj=0;
			}
			ele16[num].nexthop=currentLenBit24;
			Buildbitmap24(pTrie,0,"");
			currentLenBit24++;
		}
	}

	char lPrefix[80],rPrefix[80];
	memset(lPrefix,0,sizeof(lPrefix));
	memset(rPrefix,0,sizeof(rPrefix));
	sprintf(lPrefix,"%s0",prefix);
	sprintf(rPrefix,"%s1",prefix);
	OHConstruction(pTrie->lchild,level+1,lPrefix);
	OHConstruction(pTrie->rchild,level+1,rPrefix);
}

void CFib::Buildbitmap24(FibTrie* pTrie,int level, char *prefix)
{
	if (NULL==pTrie || level>8)return;

	if (8==level)
	{
		if (IsLeaf(pTrie))
		{
			ele24[currentLenBit24*256+btod(prefix)]=pTrie->newPort;
		}
		else
		{
			ele24[currentLenBit24*256+btod(prefix)]=0;
		}
	}

	char lPrefix[50],rPrefix[50];
	memset(lPrefix,0,sizeof(lPrefix));
	memset(rPrefix,0,sizeof(rPrefix));
	sprintf(lPrefix,"%s0",prefix);
	sprintf(rPrefix,"%s1",prefix);
	Buildbitmap24(pTrie->lchild,level+1,lPrefix);
	Buildbitmap24(pTrie->rchild,level+1,rPrefix);
}

void CFib::outputTwoArray()
{
	ofstream fout16("Array_16.txt");
	for (int i=0;i<65536;i++)
	{
		if (true==ele16[i].flag)
		{
			printf("%d\t1\t",i);
			fout16<<"1\t";
		}
		else
		{
			printf("%d\t0\t",i);
			fout16<<"0\t";
		}
		printf("%d\n",ele16[i].nexthop);
		fout16<<ele16[i].nexthop<<endl;
	}
	fout16.close();

	ofstream fout24("Array_24.txt");
	for (int i=0;i<currentLenBit24;i++)
	{
		for (int j=0;j<256;j++)
		{
			if (ele24[i*256+j]>0)
			{
				printf("1");
				fout24<<"1";
			}
			else
			{
				printf("0");
				fout24<<"0";
			}
		}
		printf("\n");
		fout24<<endl;
	}
	fout24.close();
}

unsigned int CFib::TWMX( unsigned int a)
{
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	return a;
}