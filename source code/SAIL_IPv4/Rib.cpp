/*
 * Rib.cpp
 *
 *  Created on: 2011-4-6
 *      Author: root
 */

#include "Rib.h"
#include <iostream>
#include <fstream>

CRib::CRib(void)
{
	//init the root node
	m_pTrie = (struct RibTrie*)malloc(RIBLEN);
	m_pTrie->pLeftChild = NULL;
	m_pTrie->pRightChild = NULL;
	m_pTrie->iNextHop = EMPTYHOP;

}

CRib::~CRib(void)
{
}

// construct a rib trie from a file----sFileName, return the number of entries
unsigned int CRib::BuildRibFromFile(string sFileName)
{
	unsigned int	iEntryCount=0;		//record the number of valid entries
	char			sPrefix[20];		//store prefix in char[]
	unsigned long	lPrefix;			//store prefix in integer
	unsigned int	iPrefixLen;			//the lenght of prefix
	unsigned int	iNextHop;			//nexthop

	//open rib file and read
	ifstream fin(sFileName.c_str());
	while (!fin.eof()) {

		//initiation
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		//read prefix and nexthop
		fin >> sPrefix>> iNextHop;
		
		int iStart=0;				//define the start point to divide 
		int iEnd=0;					//define the end point to divide 
		int iFieldIndex = 3;		//define the index of division
		int iLen=strlen(sPrefix);	//the length of prefix

		//start analyze
		if(iLen>0)
		{
			iEntryCount++;
			for ( int i=0; i<iLen; i++ )
			{
				//the first third divisions
				if ( sPrefix[i] == '.' )
				{
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex); //left shift to hight order
					iFieldIndex--;
					iStart = i+1;
					i++;
				}
				if ( sPrefix[i] == '/' )
				{
					//the fourth division
					iEnd = i;
					string strVal(sPrefix+iStart,iEnd-iStart);
					lPrefix += atol(strVal.c_str());
					iStart = i+1;

					//the length of prefix
					i++;
					strVal= string(sPrefix+iStart,iLen-1);
					iPrefixLen=atoi(strVal.c_str());
				}
			}
			//printf("hop:%d\n",iNextHop);
			AddNode(lPrefix,iPrefixLen,iNextHop);
		}
	}
	//close the file
	fin.close();
	return iEntryCount;
}

//convert binary file---sBinFile to ip file---sIpFile
unsigned int CRib::ConvertBinToIP(string sBinFile,string sIpFile)
{

	char			sBinPrefix[32];	
	string			strIpPrefix;
	unsigned int	iPrefixLen;
	unsigned int	iNextHop;
	unsigned int	iEntryCount=0;

	ofstream fout(sIpFile.c_str());
	ifstream fin(sBinFile.c_str());
	while (!fin.eof()) 
	{
		iNextHop = 0;
		fin >> sBinPrefix>> iNextHop;

		if(iNextHop != 0){
			string strBin(sBinPrefix);
			iPrefixLen=strBin.length();
			strBin.append(32-iPrefixLen,'0');

			strIpPrefix="";
			for(int i=0; i<32; i+=8)
			{
				int iVal=0;
				string strVal=strBin.substr(i,8);

				for(int j=7;j>=0;j--)
				{
					if(strVal.substr(j,1)=="1")
					{
						iVal+=(1<<(7-j));
					}
				}

				char buffer[5];
				//sprintf(buffer,"%d",iVal);
				strVal=string(buffer);

				strIpPrefix += strVal;
				if(i<24)
				{
					strIpPrefix += ".";
				}
				strVal="";
			}
			fout<<strIpPrefix<<"/"<<iPrefixLen<<" "<<iNextHop<<endl;
		}
	}

	fin.close();
	fout<<flush;
	fout.close();

	return iEntryCount;
}

//Add a new node to trie, need ip prefix---lPrefix, the length of prefix---iPrefixLen, and nexthop---iNextHop
void CRib::AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop)
{
	RibTrie* pTrie = m_pTrie;
	for (unsigned int i=0; i<iPrefixLen; i++)
	{

		if(((lPrefix<<i) & HIGHTBIT)==HIGHTBIT)
		{//turn right
			//if null, create a new node
			if(pTrie->pRightChild == NULL)
			{
				RibTrie* pTChild = (struct RibTrie*)malloc(RIBLEN);
				//insert the new node
				pTChild->pParent = pTrie;
				pTChild->pLeftChild = NULL;
				pTChild->pRightChild = NULL;
				pTChild->iNextHop= EMPTYHOP;
				pTrie->pRightChild = pTChild;
			}
			//turn right
			pTrie = pTrie->pRightChild;
		}
		else
		{//turn left
			//if null, create a new node
			if(pTrie->pLeftChild == NULL){
				RibTrie* pTChild = (struct RibTrie*)malloc(RIBLEN);
				//insert the new node
				pTChild->pParent = pTrie;
				pTChild->pLeftChild = NULL;
				pTChild->pRightChild = NULL;
				pTChild->iNextHop= EMPTYHOP;
				pTrie->pLeftChild = pTChild;
			}
			//turn left
			pTrie = pTrie->pLeftChild;
		}
	}

	//change nexthop
	pTrie->iNextHop = iNextHop;
}

//get the nodes number of ribtree---pTrie
unsigned int CRib::GetNodeCount(RibTrie* pTrie)
{
	//compute the current node
	unsigned int iCount=0;
	if(pTrie->iNextHop!=EMPTYHOP)	iCount = 1;

	//compute the left tree
	if(pTrie->pLeftChild!=NULL)		iCount += GetNodeCount(pTrie->pLeftChild);

	//compute the right tree
	if(pTrie->pRightChild!=NULL)	iCount += GetNodeCount(pTrie->pRightChild);

	return iCount;
}
//convert binary file---sBinFile to ip file---sIpFile, return the number of entries changed
unsigned int CRib::ConvertIpToBin(string sIpFile,string sBinFile)
{

	char			sBinPrefix[32];		//store the prefix in binary format with char []
	string			strIpPrefix;		//store the prefix in binary format with integer
	unsigned int	iPrefixLen;			//store the length of prefix
	unsigned int	iNextHop;			//next hop
	unsigned int	iEntryCount=0;		//the number of entries changed

	char			sPrefix[20];		//prefix read from file

	//open output file
	ofstream fout(sBinFile.c_str());

	//open input file, start to read route information
	ifstream fin(sIpFile.c_str());
	while (!fin.eof()) 
	{
		//initiation
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		//read prefix and next hop
		fin >> sPrefix>> iNextHop;

		int iLen=strlen(sPrefix);	//length of prefix

		//start analyze
		if(iLen>0){
			iEntryCount++;
			for ( int i=0; i<iLen; i++ ){
				if ( sPrefix[i] == '/' ){
					//the fourth division
					string strVal(sPrefix,i);
					strIpPrefix=strVal;

					//the length of prefix
					strVal= string(sPrefix+i+1,iLen-1);
					iPrefixLen=atoi(strVal.c_str());
					break;
				}
			}

			//change to binary format
			IpToBinary(strIpPrefix,sBinPrefix);
			//handle with root node
			if(iPrefixLen>0)
			{
				strIpPrefix=string(sBinPrefix,iPrefixLen);
			}
			else
			{
				strIpPrefix="*";
			}
			//output to file
			fout<<strIpPrefix<<"\t"<<iNextHop<<endl;
		}
	}

	//close files
	fin.close();
	fout<<flush;
	fout.close();
	return iEntryCount;
}

//convert a ip address to binary format
void CRib::IpToBinary(string sIP,char saBin[32]){
	int iStart=0;				//define the start point to divide 
	int iEnd=0;					//define the end point to divide 
	int iFieldIndex = 3;		//define the index of division
	int iLen=sIP.length();		//define the length of ip
	unsigned long	lPrefix=0;	//the integer corresponding to IP

	//convert ip to integer
	for ( int i=0; i<iLen; i++ )
	{
		//the first third divisions
		if ( sIP.substr(i,1)== "." )
		{
			iEnd = i;
			string strVal=sIP.substr(iStart,iEnd-iStart);
			lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex); //shift to higher bit
			iFieldIndex--;
			iStart = i+1;
			i++;
		}
		if ( iFieldIndex == 0 )
		{
			//the fourth division
			iEnd = iLen;
			string strVal=sIP.substr(iStart,iEnd-iStart);
			lPrefix += atol(strVal.c_str());
			iStart = i+1;
		}
	}

	//convert to binary with char []
	unsigned long	lVal=0x80000000;
	for(int i=0;i<32;i++)
	{
		if(lPrefix&lVal)
		{
			saBin[i]='1';
		}
		else{
			saBin[i]='0';
		}
		lVal=lVal>>1;
	}
}
