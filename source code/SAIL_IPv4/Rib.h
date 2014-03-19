/*
 * Rib.h
 *
 *  Created on: 2011-4-6
 *      Author: root
 */

#ifndef RIB_H_
#define RIB_H_

//#pragma once
#define RIBLEN				sizeof(struct RibTrie)	//the length of ribTrie struct
#define HIGHTBIT			2147483648				//a 32 bit binary number, whose highest bit is 1,and other bits are all 0.
#define EMPTYHOP			0						//use 0 represent empty hop

#include <string>

using namespace std;

//define the Rib trie structure
struct RibTrie
{
	RibTrie*				pParent;				//point to parent
	RibTrie*				pLeftChild;				//point to left child
	RibTrie*				pRightChild;			//point to right child
	int			iNextHop;							//Nexthop
};

//define the Rib trie class
class CRib
{
public:
	CRib(void);
	~CRib(void);

	RibTrie* m_pTrie;				//RibTrie

	// construct a rib trie from a file----sFileName, return the number of entries
	unsigned int BuildRibFromFile(string sFileName);

	//according the root node---ptrie,return the count of trie nodes.
	unsigned int GetNodeCount(RibTrie* pTrie);

	//convert binary file---sBinFile to ip file---sIpFile
	unsigned int CRib::ConvertBinToIP(string sBinFile,string sIpFile);

	//opposite to ConvertBinToIP
	unsigned int CRib::ConvertIpToBin(string sIpFile,string sBinFile);

	//convert a ip address to binary format
	void IpToBinary(string sIP,char sBin[32]);

private:
	//Add a new node to trie, need ip prefix---lPrefix, the length of prefix---iPrefixLen, and nexthop---iNextHop
	void AddNode(unsigned long lPrefix,unsigned int iPrefixLen,unsigned int iNextHop);

};


#endif /* RIB_H_ */
