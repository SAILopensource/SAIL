#include "3Table.h"
#include <iostream>


#include <stdio.h>
#include <fstream>
//#include <math.h>
#include "Rib.h"

#define IP_LEN		32

char * trace_path ="trace(100000).integer";//"rand_trace(100000).integer";//
char * ribfileName="rib.txt.port";

//given the point pTrie, create a empty node for it
void CreateNewNode(RibTrie* &pTrie)
{
	//allocate space
	pTrie= (struct RibTrie*)malloc(sizeof(struct RibTrie));
	//init parameter
	pTrie->iNextHop=0;
	pTrie->pLeftChild=NULL;
	pTrie->pRightChild=NULL;
	pTrie->pParent=NULL;
}

//given the root of rib trie---m_trie and the prefix---insert_C, return nexthop
int FindNextHop(RibTrie * m_trie,char * insert_C)
{
	int nextHop=-1;//init the return value
	RibTrie *insertNode=m_trie;

	bool IfNewBornNode=false;
	if (insertNode->iNextHop!=0)nextHop=insertNode->iNextHop;
	int len=(int)strlen(insert_C);
	for (int i=0;i<len+1;i++)
	{		
		if ('0'==insert_C[i])
		{//if 0, turn left
			if (NULL!=insertNode->pLeftChild)	insertNode=insertNode->pLeftChild;
			else								break;			
		}
		else
		{//if 1, turn right
			if (NULL!=insertNode->pRightChild)	insertNode=insertNode->pRightChild;
			else								break;			
		}
		if (insertNode->iNextHop!=0)			nextHop=insertNode->iNextHop;
	}
	return	nextHop;
}

char ret[IP_LEN+1];

//given a ip in binary---str and its length---len, return the next ip in binary
char * GetStringIP(char *str, int len)
{
	memset(ret,0,sizeof(ret));
	memcpy(ret,str,IP_LEN);
	int i;
	for (i=0;i<len;i++)
	{
		if ('0'==ret[i])
		{
			ret[i]='1';
			break;
		}
		else if ('1'==ret[i])
		{
			ret[i]='0';
		}
	}
	//printf("%s*\n",ret);
	return ret;
}

unsigned int btod(char *bstr)
{
	unsigned int d = 0;
	unsigned int len = (unsigned int)strlen(bstr);
	if (len > 32)
	{
		printf("too long\n");
		return -1; 
	}
	len--;
	for (unsigned int i = 0; i <= len; i++)
	{
		d += (bstr[i] - '0') * (1 << (len - i));
	}

	return d;
}

ThreeTable *fib_build_fibv4(const char *filename)
{
	ThreeTable *OHTable=new ThreeTable();


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
	tFib.HashTable32_Construction(tFib.m_pTrie,0,0);
	printf("lenofchain is %u\n The prefixes32 num is %u\n",tFib.lenofchain,tFib.prefix32_num);

	OHTable->initTree(tFib.ele16,tFib.ele24,tFib.currentLenBit24,tFib.hTable_1,tFib.hTable_2);
	return OHTable;
}

unsigned int fib_lookupv4(ThreeTable *ohTable, unsigned int ip)
{
	register unsigned char nexthop24=0;
	unsigned int LPMPort;
	/////////////////////////
	if(true==ohTable->table_16.data[ip>>16].flag)LPMPort=ohTable->table_16.data[ip>>16].nexthop;
	else if (LPMPort=ohTable->table_24.data[(ohTable->table_16.data[ip>>16].nexthop<<8)+(ip<<16>>24)]){}
	else 	 LPMPort=ohTable->HashTableLookup(ip);
	return   LPMPort;
}

void detectForFullIp(){
	int nonRouteStatic=0;

	FILE *fp_rib;
	if (fopen_s(&fp_rib,ribfileName, "r" ) !=0) 
	{
		printf("\tSource rib file doesn't exist£¬press any key to quit...\n");
		//_getch();
	}
	fclose(fp_rib);

	CRib tRib= CRib();		//define a rib struct
	int iEntryCount=tRib.BuildRibFromFile(ribfileName);
	unsigned int iNodeCount = tRib.GetNodeCount(tRib.m_pTrie);
	printf("\n\n\t\tthe entries number of file1 is:\t%u\n\t\tthe nodes number of trie tree1 is:\t%u\n",iEntryCount,iNodeCount);


	ThreeTable *OHTable=fib_build_fibv4(ribfileName);
	//OHTable.PrepareToLookup(ribfileName);
	//OHTable.

	int hop1=0;
	int hop2=0;

	char strIP00[IP_LEN+1];
	memset(strIP00,0,sizeof(strIP00));
	for (int tmp=0;tmp<IP_LEN;tmp++)
	{
		strIP00[tmp]='0';
	}
	int len88=strlen(strIP00);

	char new_tmp[IP_LEN+1];
	char old_tmp[IP_LEN+1];
	memset(new_tmp,0,sizeof(new_tmp));
	memset(new_tmp,0,sizeof(new_tmp));
	memcpy(new_tmp,strIP00,IP_LEN);

	double zhishuI=pow((double)2,(double)IP_LEN);

	bool ifhalved=false;
	printf("\t\ttotal\t%.0f\t\n",zhishuI);
	printf("\t\tlength\tcycles\t\tpercent\tnexthop\n");
	for (long long k=0;k<zhishuI;k++)
	{
		memcpy(old_tmp,new_tmp,IP_LEN);
		memcpy(new_tmp,GetStringIP(old_tmp,IP_LEN),IP_LEN);
		hop1=FindNextHop(tRib.m_pTrie,new_tmp);

	
		unsigned int IPInteger=btod(new_tmp);
		//hop2=OHTable.LookupAnIP(IPInteger);
		hop2=fib_lookupv4(OHTable,IPInteger);
		//new_tmp = 0x00000000001ffd48 "10000000 00000000 00000000 00000000"

		if (hop1==-1 && hop2!=hop1)
		{
			nonRouteStatic++;
			continue;
		}
		double ratio=0;
		if (hop2!=hop1)
		{
			printf("%d:%d",hop1,hop2);
			printf("\n\n\t\tNot Equal!!!\n");
			//_getch();

		}
		else 
		{
			//if (-1==hop1)nonRouteNum++;

			if (k%100000==0)
			{
				ratio=k/(double)(zhishuI/100);
				printf("\r\t\t%d\t%lld\t%.2f%%\t%d             ",IP_LEN,k,ratio,hop1);
			}
		}
	}
	printf("\n\t\tTotal number of garbage roaming route£º%d",nonRouteStatic);
	//printf("\n\t\tTotal number of Non-Route: %d\n",nonRouteNum);
	printf("\t\tEqual!!!!\n");
	printf("\t\tPress any key to Exit...");
	//_getch();
}


void help () {
	printf ("#######################################\n");
	printf ("##  *-*-*-*-OH algorithm-*-*-*-*-*   ##\n");
	printf ("#   {para} = [trace name] [rib name]  #\n");
	printf ("##       trace_path   ribfileName    ##\n");
	printf ("#######################################\n");
	system ("pause");
}



void main (int argc, char** argv) {

	//detectForFullIp();
	//return;
	if (argc > 1)
	{
		ThreeTable tt;
		tt.runLookupV4(argv[1],1,argv[2]);
	} 
	else 	
		help ();
}

