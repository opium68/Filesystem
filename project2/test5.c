#include "filesystem.h"
#include <string.h>

/*
typedef struct tagNode Node ;

struct tagNode
{
	Data *pData;
	Node *pNext;
};
*/
/*
   long long readIDDire(Inode *pInode)
   {
   long long dbNum = pInode->direct;

   return dbNum;
   }

   long long readDireinSin(Data *pData, long long SinD,int k)
   {
   long long dbNum;

   dbNum = readbitArr(pData+SinD, k);
   }

   long long readIDSinD(Inode *pInode, Data *pData, int k)
   {
   long long SinD = pInode -> sindirect;
   long long dbNum ;

   dbNum = readDireinSin(pData, SinD,k - 2);

   return dbNum;
   }

   long long readSinDinDlin(Data *pData, long long Dlin,int k)
   {
   long long SinD = readbitArr(pData+Dlin, k/103 + 1);
   long long dbNum;

   dbNum = readDireinSin(pData, SinD, k%103);

   return dbNum;

   }

   long long readIDDlin(Inode *pInode, Data *pData, int k)
   {
   long long Dlin = pInode->dlindirect;
   long long dbNum;

   dbNum = readSinDinDlin(pData, Dlin, k - 105 -1);

   return dbNum;
   }
   */
/*
long long readDbNuminID(Inode *pInode ,Data *pData, int allocIndex)
{
	long long dbNum;

	if(allocIndex == 1)
		dbNum = readIDDire(pInode);
	if((allocIndex >= 2) && (allocIndex <=104))
		//dbNum=readIDSinD(pInode, pData, pInode->fileSize/128);
		dbNum=readIDSinD(pInode, pData, allocIndex);
	if((allocIndex >= 105) && (allocIndex <=10611))
		//dbNum=readIDDlin(pInode, pData, pInode->fileSize/128);
		dbNum=readIDDlin(pInode, pData, allocIndex);

	return dbNum;
}

Node *createDBNode(Data *pData)
{
	Node *pNode = (Node *)malloc(sizeof(struct tagNode));

	pNode->pNext = NULL;
	pNode->pData = pData ;

	return pNode;
}

void insertDBNode(Node **pTail, Node *pNode)
{
	pNode->pNext = (*pTail)->pNext;
	(*pTail)->pNext = pNode;

	*pTail = pNode;
}

void deleteDBNode(Node *pPrev, Node **pTail)
{
	Node *pNode = *pTail ;

	pPrev->pNext = pNode->pNext;
	(*pTail) = pPrev;

	free(pNode);
}

Node *findDBPrevNode(Node *pHead, Node *pFind)
{
	Node *pNode = pHead;

	for(pNode;pNode!=NULL;pNode=pNode->pNext)
		if(pNode->pNext == pFind)	{	return pNode ; }
}
//deletNode();
*/

void storeDatainBlock(Inode *pInode,Data *pData, char c)
{
	int dbNum; 

	dbNum=readDbNuminID(pInode, pData, (pInode->fileSize)/128);
	printf("dbNum:%d\n",dbNum);

	pData[dbNum].file[(pInode->fileSize)%128] = c;

	pInode->fileSize += 1;

}


int main(void)
{
	SuperBlock spblock;

	Inode inode;

	Data dataBlock[1024];

	char c;

	int dbNum;

	int flag;
	int i = 0;

	initsuperBlock(&spblock);	

	inode.fileSize = 0 ;

	DNode Head = {NULL, NULL} ;
	DNode *pTail = &Head;
	DNode *pDNode = NULL;

	while((c=getchar()) != '\n')
	{
		if(inode.fileSize % 128 == 0)
		{
			flag=allocdbinIDdirect(&spblock,&inode, dataBlock);

			if(flag == 1)
				flag=allocdbinIDsindirect(&spblock,&inode,dataBlock);

			if(flag == 1)
			{
				flag=allocdbinIDdlindirect(&spblock, &inode, dataBlock);	
				if(isBreak(flag))	{	break;	}
			}
		}

		/*
		   if(inode.fileSize / 128 == 1)
		   dbNum = readIDDire(&inode);
		   if((inode.fileSize / 128 >= 2) && (inode.fileSize / 128 <=104))
		   dbNum=readIDSinD(&inode, dataBlock, inode.fileSize/128);
		   if((inode.fileSize / 128 >= 105) && (inode.fileSize /128 <=10611))
		   dbNum=readIDDlin(&inode, dataBlock, inode.fileSize/128);
		   */

	//	dbNum=readDbNuminID(&inode, dataBlock, i/128+1);

	//	dataBlock[dbNum].file[i%128] = c;
		storeDatainBlock(&inode, dataBlock, c);
		/*
		   printf("i:%d\n",i);
		   for(int j=0;j<i%128;j++)
		   printf("dataBlock[%d].file[%d]:%c\n",dbNum,j,dataBlock[dbNum].file[j]);
		   */
		i++;
	}


		
	printf("inode.sindirect:%d\n",inode.sindirect);
	printf("inode.dlindirect:%d\n",inode.dlindirect);
	printf("inode.fileSize:%d\n",inode.fileSize);

	for(int i=0;i<16;i++)
		printBit(dataBlock[208].dataArr[i]);

/*	
	for(int i=0;i<128;i++)
		   printf("dataBlock[207].file[%d]:%c\n",i,dataBlock[207].file[i]);
*/		   
	/*
	for(int i=0;i<=(inode.fileSize-1)/128;i++)
	{
		if(i == 1 || i ==103)
			i++;
		if(i>103 && (i%103 == 2))
		{
			printf("i:%d\n",i);
			i++;
		}
		dbNum=readDbNuminID(&inode, dataBlock, i+1);
		//printf("dbNum:%d\n",dbNum);
		pDNode = createDBNode(dataBlock + dbNum);
		insertDBNode(&pTail, pDNode);
	}
	*/

/*	
	for(DNode *pDNode = Head.pNext;pDNode != NULL;pDNode = pDNode->pNext)
			printf("[%d]\n",pDNode->pData-dataBlock);
*/			
	/*
	for(DNode *pDNode = Head.pNext;pDNode != NULL;pDNode = pDNode->pNext)
	{
		for(int i=0;i<128;i++)
			printf("[%d]%c\n",pDNode->pData-dataBlock,pDNode->pData->file[i]);
		putchar('\n');
	}
	*/
	  	
/*	int j=0;
	for(DNode *pDNode = &Head; pDNode != NULL;pDNode = pDNode->pNext, j++)
		printf("[%d]:pDNode: %p, pDNode->pNext: %p\n",pDNode->pData - dataBlock,pDNode, pDNode->pNext);
	*/
/*
	for(;pTail != &Head;)
		deleteDBNode(findDBPrevNode(&Head, pTail),&pTail);
*/
/*	
	j=0;
	for(DNode *pDNode = &Head; pDNode != NULL;pDNode = pDNode->pNext, j++)
		printf("[%d]:pNode: %p, pNod->pNext: %p\n",pDNode->pData - dataBlock,pDNode, pDNode->pNext);
*/	
	return 0;
}
