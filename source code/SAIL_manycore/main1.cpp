#include "3Table.h"
#include <iostream>
#include <tmc/cpus.h>
#include <tmc/task.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <math.h>
#include "Rib.h"

#define IP_LEN		32
#define FLOWS_EACH_TILE 10000 * 100000

char * trace_path ="trace(100000).integer";//"rand_trace(100000).integer";//
char * ribfileName="rib.txt.port";

#define TOTAL_IP 10000000
#define MAX_TILES  32
#define RUN_TIME  100
unsigned int traffic[TOTAL_IP];
unsigned int buffer[MAX_TILES][TOTAL_IP];
unsigned int number[MAX_TILES]; 



int coreTraffic[32];
unsigned int splitTrace(unsigned int IPInteger,bool ifbalance)
{
	unsigned int numberID=0;
	numberID=(IPInteger>>24)%32;
	if (ifbalance)
	{
		if (32*coreTraffic[numberID]/10000000.0>1.01)
		{
			for (int i=0;i<32;i++)
			{
				if (32*coreTraffic[i]/10000000.0<1)
				{
					numberID=i;
					break;
				}
			}
		}
	}
	coreTraffic[numberID]++;

	return numberID;
}


void generateData(char *trace_path)
{
	// read traffic from file
	int return_value=-1;
	unsigned int traceNum=0;

	//first read the trace...
	ifstream fin(trace_path);
	if (!fin)return;
	fin>>traceNum;

	int TraceLine=0;
	unsigned int IPtmp=0;
	int coreID = 0;
	memset( number, 0, sizeof(number));

	while (!fin.eof() && TraceLine<TOTAL_IP )
	{
		fin>>IPtmp;
		coreID = splitTrace( IPtmp, true);
		//traffic[TraceLine]=IPtmp;
		buffer[coreID][ number[coreID]++ ] = IPtmp;
		TraceLine++;
	}
	fin.close();
	for (int i = 0 ; i < 32; ++i) {
		printf("%d\n", number[i]);
	}
	printf("trace read complete...\n");
}


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

void detectForFullIp(){
	int nonRouteStatic=0;

	FILE *fp_rib;
	if ( (fp_rib = fopen(ribfileName, "r" )) !=0) 
	{
		printf("\tSource rib file doesn't exist£¬press any key to quit...\n");
		getchar();
	}
	fclose(fp_rib);

	CRib tRib= CRib();		//define a rib struct
	int iEntryCount=tRib.BuildRibFromFile(ribfileName);
	unsigned int iNodeCount = tRib.GetNodeCount(tRib.m_pTrie);
	printf("\n\n\t\tthe entries number of file1 is:\t%u\n\t\tthe nodes number of trie tree1 is:\t%u\n",iEntryCount,iNodeCount);


	ThreeTable OHTable;
	OHTable.PrepareToLookup(ribfileName);

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
		hop2=OHTable.LookupAnIP(IPInteger);
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
			getchar();

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
	getchar();
}


void help () {
	printf ("#######################################\n");
	printf ("##  *-*-*-*-OH algorithm-*-*-*-*-*   ##\n");
	printf ("#   {para} = [trace name] [rib name]  #\n");
	printf ("##       trace_path   ribfileName    ##\n");
	printf ("#######################################\n");
}

//
// Test parameters and their defaults.
//

double max_time = 0;

//
// Communication between worker threads and master thread.
//
pthread_mutex_t master_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t master_cond = PTHREAD_COND_INITIALIZER;
int tile_counter = 0;
cpu_set_t desired_cpus;
char *dipfile, *portfile;
ThreeTable tt;

//
// Function run by each thread, taking a tile number as its argument.
//
void *thread_func(void *arg)
{
    int number = (int)(intptr_t)arg;
    int cpu = tmc_cpus_find_nth_cpu(&desired_cpus, number);

    // Bind to one tile.
    if (tmc_cpus_set_my_cpu(cpu) != 0)
        tmc_task_die("tmc_cpus_set_my_cpu() failed.");
    //double eachTime = tt. lookupKernelTable (table_16.data, table_24.data,  hash_table32.data1,hash_table32.data2, result, traffic, size);


    double eachTime = tt.lookupbyhth( dipfile, 1, portfile); 

    pthread_mutex_lock(&master_lock);
    tile_counter++;
    max_time = max( max_time, eachTime );
    pthread_mutex_unlock(&master_lock);
    pthread_cond_signal(&master_cond);
    return (void*)NULL;
}

void *thread_func1(void *arg)
{
    int num= (int)(intptr_t)arg;
    int cpu = tmc_cpus_find_nth_cpu(&desired_cpus, num);

    // Bind to one tile.
    if (tmc_cpus_set_my_cpu(cpu) != 0)
        tmc_task_die("tmc_cpus_set_my_cpu() failed.");
    //double eachTime = tt. lookupKernelTable (table_16.data, table_24.data,  hash_table32.data1,hash_table32.data2, result, traffic, size);

    unsigned int* result = new unsigned int[1];
    double eachTime = tt.lookupKernelTableByHth ( tt.table_16.data, tt.table_24.data,  tt.hash_table32.data1,tt.hash_table32.data2, result, buffer[num], number[num]);
    // check result

    delete[] result;

    pthread_mutex_lock(&master_lock);
    tile_counter++;
    max_time = max( max_time, eachTime );
    pthread_mutex_unlock(&master_lock);
    pthread_cond_signal(&master_cond);
    return (void*)NULL;
}


void parallel_exp( int n_tiles ) {
    //
    // Get desired tiles and make sure we have enough.
    //
    if (tmc_cpus_get_my_affinity(&desired_cpus) != 0)
        tmc_task_die("tmc_cpus_get_my_affinity() failed.");
    int avail = tmc_cpus_count(&desired_cpus);
    if (avail < n_tiles)
        tmc_task_die("Need %d cpus, but only %d specified in affinity!",
                n_tiles, avail);

    max_time = 0;
    tile_counter = 0;
    //
    // Spawn a thread on each tile.
    //
    pthread_t threads[MAX_TILES];
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    for (int i = 0; i < n_tiles; i++)
    {
        //int cpu = tmc_cpus_find_nth_cpu(&desired_cpus, i);
        pthread_create(&threads[i], &thread_attr, thread_func1, (void*)(intptr_t)i);
    }

    //
    // Wait for all threads to be created and run.
    //
    pthread_mutex_lock(&master_lock);
    while (tile_counter < n_tiles)
        pthread_cond_wait(&master_cond, &master_lock);
    pthread_mutex_unlock(&master_lock);

    printf("%lf\n", 1.0 * 100  * TOTAL_IP / max_time );

}

int main (int argc, char** argv) {

	//detectForFullIp();
	if (argc > 1)
	{
		dipfile = argv[1];
		portfile = argv[2];
		tt.runLookupV4(dipfile, 1, portfile);
		generateData( dipfile );
		parallel_exp( MAX_TILES );
		//for (int i = 1; i < MAX_TILES; ++i) {
		//	parallel_exp(i);
		//}
	} 
	else 	
		help ();
	return 0;
}
