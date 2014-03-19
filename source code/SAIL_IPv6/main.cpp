#include <iostream>
#include <string.h>
#include <windows.h>
#include "Fib.h"

using namespace std;

extern const char *test;
extern const char *output;
extern const char *ytOutput;
extern const char *sailOutput;

int main(int argc, char** argv) {

	char trace[MAX_BUF_LEN];
	char route[MAX_BUF_LEN];

	memset(trace, 0, sizeof(trace));
	memset(route, 0, sizeof(route));

	if (argc >= 3)
	{
		if (strlen(argv[1]) >= MAX_BUF_LEN || strlen(argv[2]) >= MAX_BUF_LEN)
		{
			cout << "The parameters are too long!!!" << endl;
			exit(1);
		}

		strcpy(trace, argv[1]);
		strcpy(route, argv[2]);
		//strcpy(trace, " rrc16_201203012359_v6.prefix_100K.tr");
		//strcpy(route, "rrc16_201203012359_v6.prefix");

		int count = 0;
		int level = 0;

		FibTrie ytFtrie = FibTrie();
		count = ytFtrie.buildTrieFromFile(route, TRIE6);

		cout << "Out buildTrieFromFile, number of entries read: " << count<< endl;

		ytFtrie.verify(trace, output, COMM, TRIE6);
		cout << "Out verify COMM, enter ytLevelPushing..." << endl;

		ytFtrie.ytLevelPushing(ytFtrie.root6, level, 0);
		cout << "Out ytLevelPushing...enter verify LEVELPUSH" << endl;
		ytFtrie.verify(trace, ytOutput, LEVELPUSH, TRIE6);

		//level = 0;
		//ytFtrie.traverse(ytFtrie.root6, level);

		//for (int i = 0; i < 65; i++)
		//{
		//	cout << i << "th level has " << ytFtrie.levelStat[i] << " nodes" << endl;
		//	cout << i << "th level has " << ytFtrie.levelNonLeafStat[i] << " non leaf nodes" << endl;
		//	cout << endl;
		//}

		cout << "Out verify LEVELPUSH, enter buildSailTables..." << endl;
		ytFtrie.buildSailTables(TRIE6);
		ytFtrie.verify(trace, sailOutput, SAILTABLE, TRIE6);

		ytFtrie.LookupspeedTest(trace,TRIE6);
		cout << "Out verify SAILTABLE, enter generateReport..." << endl;

		ytFtrie.generateReport(trace, route);
	}
	else
	{
		cout << "The program needs 2 parameters. e.g., Ftrie.exe tracePath routeTablePath" << endl;
		exit(2);
	}



	system("pause");
	return 0;
}