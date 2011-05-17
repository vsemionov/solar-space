/*
 * Copyright (C) 2003-2011 Victor Semionov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the copyright holder nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <malloc.h>

#include "NameTree.h"

#ifdef _MSC_VER
#define NULL 0
#endif








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
		node=newnode;
	}

	if (*string==0)
	{
		node->id=id;
		return true;
	}

	if (node->sub==NULL)
		NewBranch(node);
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





void CNameTree::RemoveNode(node_s *node)
{
	node->prev->next=node->next;
	node->next->prev=node->prev;
}





void CNameTree::FreeNode(node_s *node)
{
	RemoveNode(node);
	if (node->sub)
	{
		FreeList(node->sub);
		FreeNode(node->sub);
	}
	free(node);
}





void CNameTree::FreeList(node_s *chain)
{
	while (chain->prev!=chain)
	{
		FreeNode(chain->prev);
	}
}
