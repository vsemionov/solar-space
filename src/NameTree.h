#ifndef NAMETREE_H
#define NAMETREE_H

#include "defs.h"





class CNameTree
{
public:
	CNameTree();
	virtual ~CNameTree();
	bool AddLeaf(char *name, int id);
	void Destroy();
	int FindLeaf(char *name);

private:
	void Init();

	struct node_s
	{
		char ch;
		int id;
		node_s *sub;
		node_s *prev, *next;
	};

	bool AddBranch(node_s *chain, char *string, int id);
	int SearchBranch(node_s *chain, char *string);
	bool NewBranch(node_s *node);
	void InsertNode(node_s *node, node_s *before);
	node_s *NewNode(char ch, int id);
	node_s *FindNode(node_s *chain, char ch);
	void FreeNode(node_s *node);
	void FreeList(node_s *chain);

	node_s nodechain;
};

#endif
