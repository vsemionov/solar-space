
#include <malloc.h>

#include "NameTree.h"

#define NULL 0








CNameTree::CNameTree()
{
	Init();
}





CNameTree::~CNameTree()
{
	Destroy();
}





void CNameTree::Init()
{
	nodechain.ch=0;
	nodechain.id=-1;
	nodechain.sub=NULL;
	nodechain.prev=nodechain.next=&nodechain;
}





void CNameTree::Destroy()
{
	FreeList(&nodechain);
	Init();
}





bool CNameTree::AddLeaf(char *name, int id)
{
	return AddBranch(&nodechain,name,id);
}





int CNameTree::FindLeaf(char *name)
{
	return SearchBranch(&nodechain,name);
}





bool CNameTree::AddBranch(node_s *chain, char *string, int id)
{
	node_s *newnode;
	node_s *node=FindNode(chain,*string);
	if (!node)
		return false;

	if (node->ch!=*string)
	{
		newnode=NewNode(*string,-1);
		if (!newnode)
			return false;
		InsertNode(newnode,node);
		if (!NewBranch(newnode))
			return false;
		node=newnode;
	}

	if (*string==0)
	{
		node->id=id;
		return true;
	}

	return AddBranch(node->sub,string+1,id);
}





bool CNameTree::NewBranch(node_s *node)
{
	node->sub=NewNode(0,-1);
	return (node->sub!=NULL);
}





void CNameTree::InsertNode(node_s *node, node_s *before)
{
	node->next=before;
	node->prev=before->prev;
	before->prev->next=node;
	before->prev=node;
}





CNameTree::node_s *CNameTree::NewNode(char ch, int id)
{
	node_s *newnode=(node_s*)malloc(sizeof(node_s));

	if (newnode)
	{
		newnode->ch=ch;
		newnode->id=id;
		newnode->sub=NULL;
		newnode->prev=newnode->next=newnode;
	}

	return newnode;
}





CNameTree::node_s *CNameTree::FindNode(node_s *chain, char ch)
{
	node_s *node=chain;

	if (node)
	{
		do
		{
			if (node->ch==ch || node->ch>ch)
				break;
			node=node->next;
		}
		while (node!=chain);
	}


	return node;
}





int CNameTree::SearchBranch(node_s *chain, char *string)
{
	node_s *node=FindNode(chain,*string);
	if (!node || node->ch!=*string)
		return -1;

	if (node->id!=-1)
		return node->id;
	else
		return SearchBranch(node->sub,string+1);
}





void CNameTree::FreeNode(node_s *node)
{
	node->prev->next=node->next;
	node->next->prev=node->prev;
	free(node);
}





void CNameTree::FreeList(node_s *chain)
{
	node_s *node;
	if (chain->sub)
	{
		FreeList(chain->sub);
		FreeNode(chain->sub);
	}
	node=chain->next;
	while (node!=chain)
	{
		node=node->next;
		FreeNode(node->prev);
	}
}
