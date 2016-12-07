#include "filesystem.h"
#include "command.h"
#define DEBUG 1

void storeDatainBlock(Inode *pInode, Data *pData, char c)
{
	int dbNum;
	dbNum = readDbNuminID(pInode, pData, (pInode -> fileSize) / 128);
	pData[dbNum].file[(pInode -> fileSize) % 128] = c;
	pInode -> fileSize += 1;
}	
void f_mycpto(char *name1, char *name2, SuperBlock *pSB, Inode *ind, Data *pDB, TNode *pwd);

void f_command(char *cmd)
{
	if(!strcmp(cmd, "byebye"))		exit(1);

	system(cmd);
}

void procArgument(SNode *pHead)
{
	int i=0;
	char c;
	SNode *pSNode;

	while((c=getchar()) != '\n')
	{
		if(i==0 && c=='/')
		{
			pSNode=createSNode();
			savedatatoSNode(pSNode, c, i );
			pushSNodetoStack(pHead, pSNode);
			continue;
		}
		if(i==0)
			pSNode=createSNode();
		if(c=='/' || c==' ')	
		{
			pushSNodetoStack(pHead, pSNode);
			i=0;
			continue;
		}

		savedatatoSNode(pSNode, c, i );
		i++;
	}

	pushSNodetoStack(pHead, pSNode);
}

void upsidedownStack(SNode *SHBf, SNode *SHAf)
{
	SNode *pSNode = NULL;

	for(SNode *pSNodeI =SHBf->pNext;pSNodeI!= NULL;pSNodeI=SHBf->pNext)
	{
		pSNode = createSNode();
		for(int i=0;i<20;i++)
			pSNode->data[i] = pSNodeI->data[i];
		deleteSNode(SHBf,pSNodeI);
		pushSNodetoStack(SHAf, pSNode);
	}


}

void initDirectory(Inode *pInode, Data *pData, long long idNum, TNode *Pwd, char *name)
{
	int Dire = (int) pInode[idNum].direct;

	pInode[idNum].fileType = 1;
	pInode[idNum].time = time(NULL);
	pInode[idNum].fileSize = 16; 
	//pData[Dire].directory.idNum[0]= idNum;
	pData[Dire].directory = (struct tagDirectory){{[1]=".."},{[1]=Pwd->idNum}};

}
void initFile(Inode *pInode, Data *pData, long long idNum, TNode *Pwd, char *name)
{
	//int Dire = (int) pInode[idNum].direct;

	pInode[idNum].fileType = 0;
	pInode[idNum].time = time(NULL);
	pInode[idNum].fileSize = 0; 
	//pData[Dire].directory.idNum[0]= idNum;
	//pData[Dire].directory = (struct tagDirectory){{[1]=".."},{[1]=Pwd->idNum}};

}


int confirminfoPwd(Inode *pInode, /*Data *pData,*/TNode *Pwd)
{
	int idNum = Pwd->idNum;
	//int dbNum = pInode->direct;

	int arrNum= ((pInode[idNum].fileSize)/8 == 16) ? 0 : (pInode[idNum].fileSize)/8  ;

	return arrNum;
}

int addinfoPwd(Inode *pInode, Data *pData, TNode *Pwd, long long *idNum)
{
	int idpNum = Pwd->idNum;
	//	int dbNum = readDbNuminID(pInode+(*idNum), pData, (pInode[idpNum].fileSize)/128);
	int index;

	index = confirminfoPwd(pInode, /*pData,*/ Pwd);

	//	pData[dbNum].directory.idNum[index] = (*idNum);
	pData->directory.idNum[index] = (*idNum);

	return index;
}
void preorderTraverse(TNode *pTNode, Inode *pInode, Data *pData, int i)
{
	if(pTNode == NULL)	{ return ; }

	long long idNum = pTNode->idNum;
	long long dbNum = pInode[idNum].direct;

	if(i==0)
		printf("%s\n",pData[dbNum].directory.name[0]);
	else
	{
		for(int j=0;j<i;j++)
			printf("--");
		printf("*%s\n",pData[dbNum].directory.name[0]);
	}

	preorderTraverse(pTNode->pPrev, pInode, pData,i+1);

	if(i!=0)
		preorderTraverse(pTNode->pNext, pInode, pData,i);
}	

void makeDirectory(SuperBlock *pSb, Inode *pInode, Data *pData, TNode *Pwd, char *name)
{
	long long idNum;
	int dbNum;
	int arrNum;
	int bitNum;
	int index;
	int i;
	int j;
	int end;
	int start;
	int flag;
	int tmpfileSize ;

	TNode *pTNode = NULL;

	DNode Head = {NULL, NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;

	idNum = Pwd->idNum;									//현재 디렉터리의 Inode
	tmpfileSize = pInode[idNum].fileSize ;

	if(pInode[idNum].fileSize % 128 == 0)
	{
		flag=allocdbinIDdirect(pSb,pInode+Pwd->idNum, pData);

		if(flag == 1)
			flag=allocdbinIDsindirect(pSb,pInode+Pwd->idNum,pData);

		if(flag == 1)
		{
			flag=allocdbinIDdlindirect(pSb, pInode+Pwd->idNum, pData);	
			if(isBreak(flag))	{	return;   }
		}
	}
	for(int i=0;i<=(pInode[idNum].fileSize)/128;i++)
	{
		dbNum=readDbNuminID(pInode+Pwd->idNum, pData, i);
		pDNode = createDBNode(pData + dbNum);
		insertDBNode(&pTail, pDNode);
	}

	for(DNode *pIndex=Head.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
	{
		if(pIndex == Head.pNext)
			start = 2;
		else 
			start = 1;
		end = ((tmpfileSize)>128) ? 128/8 : (tmpfileSize)/8 ;
		if(tmpfileSize>=128)
			tmpfileSize -= 128;
		for(int i=start;i<end;i++)
		{
			if(strcmp(pIndex->pData->directory.name[i],name) == 0)
			{
				printf("이미 같은 디렉터리가 존재 합니다.\n");
				return;
			}
		}
	}

	findusableInode(pSb->usableInode, &arrNum, &bitNum);
	markInode(pSb->usableInode, arrNum, bitNum);
	idNum = arrNum*64 + bitNum - 1;										//새로 할당된 디렉터리의 Inode
	allocdbinIDdirect(pSb, pInode + idNum, pData);						//Inode에 데이터 블럭 할당 

	pTNode = createTNode(idNum);										//트리구조에 새로 만든 디렉터리 추가
	insertleftTNode(Pwd, pTNode);

	initDirectory(pInode, pData, idNum, Pwd, name);						//새로 만든 디렉터리 초기화

	strcpy(pData[pInode[idNum].direct].directory.name[0], name);
	pData[pInode[idNum].direct].directory.idNum[0] = idNum;

	index = addinfoPwd(pInode,pTail->pData/*pData,*/, Pwd, &idNum);						//현재 디렉터리 정보 갱신
	strcpy(pTail->pData->directory.name[index],name);
	pTail->pData->directory.idNum[index] = idNum;
	for(; &Head != pTail;)
		deleteDBNode(findDBPrevNode(&Head,pTail),&pTail);
	pInode[Pwd->idNum].fileSize += 8;


}
void rm(SuperBlock *pSb,Inode *pInode, Data *pData,TNode *Pwd, char *name)
{
	long long idNum = Pwd->idNum;
	long long tmpidNum;
	long long Remo;
	long long dbNum;
	long long predbNum;
	char tmp[4];
	int i;
	int j;
	int start;
	int end;
	int flag=0;
	int l;
	int tmpfileSize ;
	int index = 0;
	tmpfileSize = pInode[idNum].fileSize ;

	TNode *pTNode = NULL ;
	TNode *pPrev = NULL;

	DNode Head = {NULL,NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;
	DNode *pDPrev = NULL;

	for(int i=0;i<=(pInode[idNum].fileSize)/128;i++)
	{
		dbNum=readDbNuminID(pInode+idNum, pData, i);
		pDNode = createDBNode(pData + dbNum);
		insertDBNode(&pTail, pDNode);
	}
	for(DNode *pIndex=Head.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
	{
		if(pIndex == Head.pNext)
			start = 2;
		else 
			start = 0;

		end = ((tmpfileSize)>128) ? 128/8 : (tmpfileSize)/8 ;
		printf("idNum:%d\n",idNum);
		for(i=start;i<end;i++)
		{
			
			printf("name:%s\n",name);
			if(strcmp(pIndex->pData->directory.name[i],name) == 0)
			{
				tmpidNum = pIndex->pData->directory.idNum[i];
				if(pInode[tmpidNum].fileType == 1)
				{
					flag = 2;
					break;
					//printf("디렉터리입니다.\n");
					//return ;
				}

				flag = 1;
				break;
			}
		}
		if((flag == 1) || (flag == 2))
			break;
		if(tmpfileSize>=128)
			tmpfileSize -= 128;
	}

	if(flag == 0)	
	{
		printf("존재 하지 않는 파일입니다.\n");
		return ;
	}
	if(flag == 2)
	{
		printf("디렉터리 입니다.\n");
		return ;
	}
	//pInode[idNum].fileSize -= 8;
	Remo = pData[dbNum].directory.idNum[i];					//지우고자 하는 디렉터리의 아이노드 번호
	pInode[idNum].fileSize -= 8;
//	unmarkInode(pSb->usableInode, Remo/64, (Remo + 1) %64);	//지우고자 하는 디렉터리에 할당된 데이터블럭 할당 해제

	for(DNode *pIndex=Head.pNext;pIndex!=NULL;pIndex=pIndex->pNext,index++)
	{
		if(pIndex == Head.pNext)
			pDPrev = pIndex ;

		if(index < ((pInode[idNum].fileSize+8)-tmpfileSize)/128)
			continue;
		else if(index == ((pInode[idNum].fileSize+8)-tmpfileSize)/128)
			j=i;
		else 
			j=0;

		end = (((pInode[idNum].fileSize+8)-128*index)>128) ? 128/8 : ((pInode[idNum].fileSize+8)-128*index)/8 ;
		for(j;j<end;j++)
		{
			if(index==0 && j<=2)
				continue;
			else if(index>=1 && j==0)
				strcpy(pDPrev->pData->directory.name[15], pIndex->pData->directory.name[j]);
			else if(j == end-1)
			{
				for(int i=0;i<4;i++)
					pIndex->pData->directory.name[j][i]=' ';
			}
			else 
				strcpy(pIndex->pData->directory.name[j], pIndex->pData->directory.name[j+1]);


		}
		pDPrev = pIndex ;

	}

//	pPrev = Pwd;
/*	for(pTNode=Pwd->pPrev;pTNode != NULL;pTNode=pTNode->pNext)
	{
		if(pTNode->idNum == idNum)
		{
			if(pTNode == Pwd->pPrev)
				deleteTNodePrev(pPrev, pTNode);
			else
				deleteTNodeNext(pPrev, pTNode);
			break;
		}
		pPrev = pTNode;
	}
*/
}

void removeDirectory(SuperBlock *pSb,Inode *pInode, Data *pData,TNode *Pwd, char *name)
{
	long long idNum = Pwd->idNum;
	long long Remo;
	long long dbNum;
	long long predbNum;
	char tmp[4];
	int i;
	int j;
	int start;
	int end;
	int flag=0;
	int l;
	int tmpfileSize ;
	int index = 0;
	tmpfileSize = pInode[idNum].fileSize ;

	TNode *pTNode = NULL ;
	TNode *pPrev = NULL;

	DNode Head = {NULL,NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;
	DNode *pDPrev = NULL;

	for(int i=0;i<=(pInode[idNum].fileSize)/128;i++)
	{
		dbNum=readDbNuminID(pInode+idNum, pData, i);
		pDNode = createDBNode(pData + dbNum);
		insertDBNode(&pTail, pDNode);
	}
	for(DNode *pIndex=Head.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
	{
		if(pIndex == Head.pNext)
			start = 2;
		else 
			start = 0;

		end = ((tmpfileSize)>128) ? 128/8 : (tmpfileSize)/8 ;
		for(i=start;i<end;i++)
		{
			if(strcmp(pIndex->pData->directory.name[i],name) == 0)
			{
				flag = 1;
				break;
			}
		}
		if(flag == 1)
			break;
		if(tmpfileSize>=128)
			tmpfileSize -= 128;
	}

	if(flag == 0)	
	{
		printf("존재 하지 않는 디렉터리입니다.\n");
		return ;
	}
	//pInode[idNum].fileSize -= 8;
	Remo = pData[dbNum].directory.idNum[i];					//지우고자 하는 디렉터리의 아이노드 번호
	if(pInode[Remo].fileType != 1)		
	{
		printf("%s는 디렉터리가 아닙니다.\n",name);
		return ;
	}
	pInode[idNum].fileSize -= 8;
	unmarkInode(pSb->usableInode, Remo/64, (Remo + 1) %64);	//지우고자 하는 디렉터리에 할당된 데이터블럭 할당 해제

	for(DNode *pIndex=Head.pNext;pIndex!=NULL;pIndex=pIndex->pNext,index++)
	{
		if(pIndex == Head.pNext)
			pDPrev = pIndex ;

		if(index < ((pInode[idNum].fileSize+8)-tmpfileSize)/128)
			continue;
		else if(index == ((pInode[idNum].fileSize+8)-tmpfileSize)/128)
			j=i;
		else 
			j=0;

		end = (((pInode[idNum].fileSize+8)-128*index)>128) ? 128/8 : ((pInode[idNum].fileSize+8)-128*index)/8 ;
		for(j;j<end;j++)
		{
			if(index==0 && j<=2)
				continue;
			else if(index>=1 && j==0)
				strcpy(pDPrev->pData->directory.name[15], pIndex->pData->directory.name[j]);
			else if(j == end-1)
			{
				for(int i=0;i<4;i++)
					pIndex->pData->directory.name[j][i]=' ';
			}
			else 
				strcpy(pIndex->pData->directory.name[j], pIndex->pData->directory.name[j+1]);


		}
		pDPrev = pIndex ;

	}

	pPrev = Pwd;
	for(pTNode=Pwd->pPrev;pTNode != NULL;pTNode=pTNode->pNext)
	{
		if(pTNode->idNum == idNum)
		{
			if(pTNode == Pwd->pPrev)
				deleteTNodePrev(pPrev, pTNode);
			else
				deleteTNodeNext(pPrev, pTNode);
			break;
		}
		pPrev = pTNode;
	}

}

void preorderTraverseP(TNode *pTNode, Inode *pInode, Data *pData, long long idNumP, TNode **pUwd)
{
	if(pTNode == NULL)	{ return ; }

	long long idNum = pTNode->idNum;


	if(idNum == idNumP)
		(*pUwd) = pTNode;
	preorderTraverseP(pTNode->pPrev, pInode, pData, idNumP, pUwd);
	preorderTraverseP(pTNode->pNext, pInode, pData, idNumP, pUwd);
}	

void storePwdName(Inode *pInode, Data *pData, TNode *Pwd, SNode *pHead)
{
	SNode *pStack = NULL;

	long long idNum = Pwd->idNum;
	long long dbNum = pInode[idNum].direct;


	for(int i=0;i<16;i++)
		if(pData[dbNum].directory.idNum[i] == idNum)
		{
			pStack=createSNode();
			strcpy(pStack->data,pData[dbNum].directory.name[i]);
			//printf("name:%s\n",pData[dbNum].directory.name[i]);
			pushSNodetoStack(pHead, pStack);
			return ;
		}
}

bool findUpwd(Inode *pInode,Data *pData,TNode *pRoot,TNode **pUwd, SNode *pHead)
{
	SNode *pStack = NULL ;

	long long idNum = (*pUwd)->idNum;
	long long dbNum = pInode[idNum].direct;

	int i=0;

	if(strcmp(pData[dbNum].directory.name[0],"/")==0)	{ return 1; }

	for(i=0;i<16;i++)
	{
		if(strcmp(pData[dbNum].directory.name[i],"..")==0)
		{
			idNum = pData[dbNum].directory.idNum[i];
			break;
		}
	}
	if(i==16)		{  return 1; }


	preorderTraverseP(pRoot, pInode, pData, idNum, pUwd);
	return 0;
}

void printPwd(Inode *pInode, Data *pData,TNode *pRoot, TNode *Pwd, TNode **pUwd)
{
	SNode Head = {{0},NULL};
	SNode *pStack = NULL;
	bool flag = 0;

	int idNum = Pwd->idNum;
	int dbNum = pInode[idNum].direct ;

	if(strcmp(pData[dbNum].directory.name[0],"/") != 0)
		storePwdName(pInode, pData, Pwd, &Head);

	while(1)
	{
		Pwd = (*pUwd);
		flag=findUpwd(pInode, pData, pRoot, pUwd, &Head);
		if(flag == 1 )		{break;}
		storePwdName(pInode, pData, Pwd, &Head);
	}

	if(Head.pNext == NULL)
		printf("/\n");
	for(pStack = Head.pNext; pStack!=NULL;pStack=pStack->pNext)
	{
		printf("/%s",pStack->data);
		if(pStack->pNext == NULL)
			putchar('\n');
	}
}

void absolutePath(Inode *pInode,Data *pData,TNode *pRoot, TNode **tPwd, TNode **tUwd,SNode *pSNode)
{
	long long idNum = (*tPwd==NULL) ? pRoot->idNum : (*tPwd)->idNum;
	long long dbNum ;

	int i;
	int j;
	int end;
	int flag;
	int tmpfileSize = pInode[idNum].fileSize;

	DNode Head = {NULL,NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;

	for(int i=0;i<=(pInode->fileSize)/128;i++)
	{
		dbNum=readDbNuminID(pInode+idNum, pData, i);
		pDNode = createDBNode(pData + dbNum);
		insertDBNode(&pTail, pDNode);
	}


	for(DNode *pIndex=Head.pNext;pIndex != NULL;pIndex=pIndex->pNext)
	{
		end = ((tmpfileSize)>128) ? 128/8 : ((tmpfileSize)/8) ;
		for(j=0;j<end;j++)
		{
			if(strcmp(pIndex->pData->directory.name[j],pSNode->data)==0)
			{
				idNum = pIndex->pData->directory.idNum[j];
				flag = 1;
				break;
			}
		}
		if(flag == 1)
			break;
		tmpfileSize -= 128;
	}

	if(i==16)		{  return ; }

	preorderTraverseP(pRoot, pInode, pData, idNum, tPwd);

	dbNum = pInode[idNum].direct;
	for(i=0;i<16;i++)
	{
		if(strcmp(pData[dbNum].directory.name[i],"..")==0)
		{
			idNum = pData[dbNum].directory.idNum[i];
			break;
		}	
	}
	if(i==16)		{  return ; }
	preorderTraverseP(pRoot, pInode, pData, idNum, tUwd);

}

void relativePath(Inode *pInode, TNode *pRoot, TNode **tPwd, TNode **tUwd, Data *pData, SNode *pSNode)
{

	long long idNum = (*tPwd)->idNum;
	long long dbNum ;

	int i;
	int j;
	int end;
	int flag;
	int tmpfileSize = pInode[idNum].fileSize;

	DNode Head = {NULL,NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;

	for(int i=0;i<=(pInode->fileSize)/128;i++)
	{
		dbNum=readDbNuminID(pInode+idNum, pData, i);
		pDNode = createDBNode(pData + dbNum);
		insertDBNode(&pTail, pDNode);
	}

	if(strcmp("..",pSNode->data)==0)
	{
		(*tPwd) = (*tUwd); 
		idNum = (*tPwd)->idNum;
	}
	else
	{

		for(DNode *pIndex=Head.pNext;pIndex != NULL;pIndex=pIndex->pNext)
		{
			end = ((tmpfileSize)>128) ? 128/8 : (tmpfileSize)/8 ;
			for(j=0;j<end;j++)
			{
				if(strcmp(pIndex->pData->directory.name[j],pSNode->data)==0)
				{
					idNum = pIndex->pData->directory.idNum[j];
					flag = 1;
					break;
				}	
			}
			if(flag == 1)
				break;
		}
		if(i==16)		{  return ; }
		preorderTraverseP(pRoot, pInode, pData, idNum, tPwd);

	}	


	dbNum = pInode[idNum].direct;

	for(i=0;i<16;i++)
	{
		if(strcmp(pData[dbNum].directory.name[i],"..")==0)
		{
			printf("idNum:%d\n",pData[dbNum].directory.idNum[i]);
			idNum = pData[dbNum].directory.idNum[i];
			break;
		}	
	}
	if(i==16)		{  return ; }
	preorderTraverseP(pRoot, pInode, pData, idNum, tUwd);

	return ;
}

void mycd(TNode **pPwd,TNode **pUwd,TNode *tPwd, TNode *tUwd)
{
	(*pPwd) = tPwd;
	(*pUwd) = tUwd;
}


typedef struct tagNameList NameList;

struct tagNameList
{
	char name[4];
	int idNum;
	NameList *pNext;
};

NameList *createNameNode(char *Name, int idNum)
{
	NameList *pNode = (NameList *)malloc(sizeof(NameList));

	strcpy(pNode->name,Name);
	pNode->idNum=idNum;

	return pNode;
}


void insertNameNode(NameList **pTail, NameList *pNode)
{
	pNode->pNext = (*pTail)->pNext;
	(*pTail)->pNext = pNode;

	*pTail = pNode;
}
void deleteNameNode(NameList *pPrev, NameList **pTail)
{
	NameList *pNode = *pTail ;

	pPrev->pNext = pNode->pNext;
	(*pTail) = pPrev;

	free(pNode);
}



void myls(Inode *pInode, Data *pData, TNode *tPwd, int flag )
{
	long long idNum = tPwd->idNum;
	long long dbNum ;
	long long tidNum = idNum;
	int i=0;
	int j=0;
	int k =0;
	int start;
	int end;
	int tmpfileSize = pInode[idNum].fileSize;
	int num=tmpfileSize/128;
	struct tm *time ;


	NameList NameHead = {{0},0,NULL};
	NameList *pNameList = NULL;
	NameList *pNameTail = &NameHead;
	NameList *tpTail;
	NameList *pPrev;

	char tmpName[4];
	int tmpNum;

	DNode Head = {NULL,NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;
	DNode *pIndex = NULL;

	//	printf("idNum:%d\n",idNum);
	//	printf("pInode[idNum].fileSize:%d\n",tmpfileSize);
	for(i=0;i<=(pInode[idNum].fileSize-1)/128;i++)
	{
		dbNum=readDbNuminID(pInode+idNum, pData, i);
		pDNode = createDBNode(pData + dbNum);
		insertDBNode(&pTail, pDNode);
		end = ((tmpfileSize)>128) ? 128/8 : (tmpfileSize)/8 ;
		for(j=0;j<end;j++)
		{
			//			printf("name:%s\n",pDNode->pData->directory.name[j]);
			pNameList=createNameNode(pDNode->pData->directory.name[j],pDNode->pData->directory.idNum[j]);
			insertNameNode(&pNameTail,pNameList);
		}
		tmpfileSize-=128;
	}
	printf("pTail->pData:%p\n",pTail->pData);
	//	for(NameList *pIndex=NameHead.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
	//		printf("idNum:%d, name:%s\n",pIndex->idNum,pIndex->name);

	tpTail=pNameTail;	
	for(NameList *pIndex=NameHead.pNext->pNext ;NameHead.pNext->pNext->pNext!=tpTail;)
	{
		if(pIndex->pNext== NULL)	{  break; }
		if((strcmp(pIndex->name,"..")==0) )
		{
			pIndex=pIndex->pNext;
			continue;
		}
		for(NameList *pJndex=pIndex;/*pJndex->pNext != NULL*/;pJndex=pJndex->pNext)
		{
			if(pJndex == tpTail)
			{
				tpTail = pPrev;
				break;
			}

			if(strcmp(pJndex->name,pJndex->pNext->name) >0)
			{
				strcpy(tmpName,pJndex->name);
				strcpy(pJndex->name,pJndex->pNext->name);
				strcpy(pJndex->pNext->name,tmpName);

				tmpNum = pJndex->pNext->idNum;
				pJndex->pNext->idNum = pJndex->idNum;
				pJndex->idNum=tmpNum;
			}
			pPrev=pJndex;
		}


	}
	//	printf("end\n");



	if(flag == 0)												//myls 인자 없는 경우
	{
		for(NameList *pIndex=NameHead.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
			if(pIndex==NameHead.pNext)
				printf(".\n");
			else 
				printf("%s\n",pIndex->name);
	}
	if(flag == 1)												//myls -i 아이노드 번호 출력
		for(NameList *pIndex=NameHead.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
			if(pIndex==NameHead.pNext)
				printf("%d .\n",pIndex->idNum);
			else 
				printf("%d %s\n",pIndex->idNum,pIndex->name);
	if(flag == 2)												//myls -l 상세 정보 출력
		for(NameList *pIndex=NameHead.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
		{
			tidNum = pIndex->idNum;
			time = localtime(&(pInode[tidNum].time));
			if(pInode[tidNum].fileType == 1)
				printf("d %4d ",0);
			else 
				printf("- %4d ",pInode[tidNum].fileSize);

			printf("%d/%d/%d ",time->tm_year + 1900, time->tm_mon+1, time->tm_mday);
			printf("%d:%d:%d ",time->tm_hour, time->tm_min, time->tm_sec);

			if(i==0 && j ==0)
				printf(".\n");
			else
				printf("%s\n",pIndex->name);

		}
	if(flag == 3)
		for(NameList *pIndex=NameHead.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
		{
			tidNum = pIndex->idNum;
			time = localtime(&(pInode[tidNum].time));
			if(pInode[tidNum].fileType == 1)
				printf("d %4d ",0);
			else 
				printf("- %4d ",pInode[tidNum].fileSize);

			printf("%d/%d/%d ",time->tm_year + 1900, time->tm_mon+1, time->tm_mday);
			printf("%d:%d:%d ",time->tm_hour, time->tm_min, time->tm_sec);

			if(i==0 && j ==0)
				printf(".\n");
			else
				printf("%s\n",pIndex->name);

		}


}

/*
   void f_mytouch(SNode *pSNode char cmd_line[][10], SuperBlock *pSB, Inode *ind, Data *pDB, TNode *pwd)
   {
   short wd = readDbNuminID(ind+pwd->idNum,pDB,ind[pwd->idNum].fileSize/128);
   int indNum = findInode(pSNode->data, &pDB[wd]);

   if(indNum == 512)
   {
   int check = findemptyDir_line(&pDB[wd]);
   if(check == -1)		return ;
   pDB[wd].directory.idNum[check] = prepareInode(pSB, ind, 0, 0);
   for(int i = 0; i < 4; i++)				
   pDB[wd].directory.name[check][i] = pSNode->data[i];

   ind[indNum].direct = -1;
   ind[indNum].sindirect = -1;
   ind[indNum].dlindirect = -1;
   }
   else
   ind[indNum].time = time(NULL);
   ind[pwd->idNum].fileSize+=8;
   }
   */
void mytouch(SuperBlock *pSb, Inode *pInode, Data *pData, TNode *Pwd, char *name)
{
	long long idNum;
	long long tmpidNum;
	int dbNum;
	int arrNum;
	int bitNum;
	int index;
	int i;
	int j;
	int end;
	int start;
	int flag;
	int tmpfileSize ;

	TNode *pTNode = NULL;

	DNode Head = {NULL, NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;

	idNum = Pwd->idNum;									//현재 디렉터리의 Inode
	tmpfileSize = pInode[idNum].fileSize ;

	printf("Pwd->idNum:%d\n",Pwd->idNum);
	for(int i=0;i<=(pInode[idNum].fileSize)/128;i++)
	{
		dbNum=readDbNuminID(pInode+Pwd->idNum, pData, i);
		pDNode = createDBNode(pData + dbNum);
		insertDBNode(&pTail, pDNode);
		printf("pDNode->pData:%p\n",pDNode->pData);
	}

	for(DNode *pIndex=Head.pNext;pIndex!=NULL;pIndex=pIndex->pNext)
	{
		if(pIndex == Head.pNext)
			start = 2;
		else 
			start = 1;
		end = ((tmpfileSize)>128) ? 128/8 : (tmpfileSize)/8 ;
		if(tmpfileSize>=128)
			tmpfileSize -= 128;
		for(int i=start;i<end;i++)
		{
			if(strcmp(pIndex->pData->directory.name[i],name) == 0)
			{
				tmpidNum = pIndex->pData->directory.idNum[i];
				if(pInode[tmpidNum].fileType == 1)
				{
					printf("같은 이름의 디렉터리가 존재합니다.\n");
					return ;
				}
				pInode[tmpidNum].time = time(NULL); 
				return;
			}
		}
	}

	findusableInode(pSb->usableInode, &arrNum, &bitNum);
	markInode(pSb->usableInode, arrNum, bitNum);
	idNum = arrNum*64 + bitNum - 1;										//새로 할당된 디렉터리의 Inode

	initFile(pInode, pData, idNum, Pwd, name);						//새로 만든 디렉터리 초기화

	index = addinfoPwd(pInode,pTail->pData/*pData,*/, Pwd, &idNum);						//현재 디렉터리 정보 갱신
	printf("pData->directory.name[index]:%s\n",pTail->pData->directory.name[index]);
	printf("index:%d\n",index);
	printf("pTail->pData:%p\n",pTail->pData);
	printf("pData[1]:%p\n",pData+1);
	strcpy(pTail->pData->directory.name[index],name);
	pTail->pData->directory.idNum[index] = idNum;
	printf("pTail->pData->directory.name[index]:%s\n",pTail->pData->directory.name[index]);
	printf("pData->directory.name[index]:%s\n",pData[17].directory.name[index]);
	for(; &Head != pTail;)
	deleteDBNode(findDBPrevNode(&Head,pTail),&pTail);
	pInode[Pwd->idNum].fileSize += 8;
}

void f_mycpfrom(char *name1, char *name2, SuperBlock *pSB, Inode *ind, Data *pDB, TNode *pwd)
{								//mycpfrom orig_fs.file my_fs.file
//	for(int i=0;i<=pInode[idNum].fileSize/128;i++)
//	{
//		dbNum = readDbNuminID(ind+pwd -> idNum, pDB, i);
//		unmarkdataBlock(pSB->usabledataBlock, dbNum/64, (dbNum+1) );
//	}

	printf("'mycpfrom' call\n");

	TNode *pTNode = NULL;
	DNode Head = {NULL, NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;
	int start, end;

	long long idNum = pwd -> idNum;
	int tmpfileSize = ind[idNum].fileSize;
	int dbNum, tmpidNum, indNum;

    FILE *ifp;
    ifp = fopen(name1, "r");
    if(ifp == NULL)
    {
        printf("'%s' is not found.\n", name1);
        fclose(ifp);
        return ;
    }

	for(int i = 0; i <= (ind[idNum].fileSize) / 128; i++)
	{
		dbNum = readDbNuminID(ind+pwd -> idNum, pDB, i);
		pDNode = createDBNode(pDB + dbNum);
		insertDBNode(&pTail, pDNode);
	}

	for(DNode *pIndex = Head.pNext; pIndex != NULL; pIndex = pIndex -> pNext)
	{
		if(pIndex == Head.pNext)
			start = 2;
		else
			start = 1;

		end = (tmpfileSize > 128) ? 128 / 8 : (tmpfileSize) / 8;
		if(tmpfileSize >= 128)
			tmpfileSize -= 128;

		for(int i = start; i < end; i++)
		{
			if(!strncmp(pIndex -> pData -> directory.name[i], name2, 4))
			{
				rm(pSB, ind, pDB, pwd, name2);
				printf("old file\n");
				//이미 파일 존재할 때 초기화 필요
			}
		}
	}

	//파일이 없을 때
	int index = addinfoPwd(ind, pTail -> pData, pwd, &idNum);

	indNum = prepareInode(pSB, ind, 1, 0);
	strncpy(pTail -> pData -> directory.name[index], name2, 4);
	pTail -> pData -> directory.idNum[index] = indNum;
	ind[pwd->idNum].fileSize += 8;

    char c;
	int flag, DBnum, i;	
    while((c = fgetc(ifp)) != EOF)
    {
		printf("ind[indNum].fileSize:%d\n", ind[indNum].fileSize);
		if(((ind + indNum) -> fileSize) % 128 == 0)
		{
			flag = allocdbinIDdirect(pSB, &ind[indNum], pDB);

			if(flag == 1)
				flag = allocdbinIDsindirect(pSB, ind+indNum, pDB);

			if(flag == 1)
			{
				flag = allocdbinIDdlindirect(pSB, &ind[indNum], pDB);
				if(isBreak(flag))	{break;}
			}
		}
		storeDatainBlock(&ind[indNum], pDB, c);
//		putchar(c);

		i++;
	}

	fclose(ifp);
}

void f_mycpto(char *name1, char *name2, SuperBlock *pSB, Inode *ind, Data *pDB, TNode *pwd)
{
	printf("'mycpto' call\n");

	TNode *pTNode = NULL;
	DNode Head = {NULL, NULL};
	DNode *pTail = &Head;
	DNode *pDNode = NULL;
	int start, end;

	long long idNum = pwd -> idNum;
	int tmpfileSize = ind[idNum].fileSize;
	int dbNum, tmpidNum, indNum;

	for(int i = 0; i <= (ind[idNum].fileSize) / 128; i++)
	{
		dbNum = readDbNuminID(ind+pwd -> idNum, pDB, i);
		pDNode = createDBNode(pDB + dbNum);
		insertDBNode(&pTail, pDNode);
	}

	for(DNode *pIndex = Head.pNext; pIndex != NULL; pIndex = pIndex -> pNext)
	{
		if(pIndex == Head.pNext)
			start = 2;
		else
			start = 1;

		end = (tmpfileSize > 128) ? 128 / 8 : (tmpfileSize) / 8;
		if(tmpfileSize >= 128)
			tmpfileSize -= 128;

		for(int i = start; i < end; i++)
		{
			if(strncmp(pIndex -> pData -> directory.name[i], name1, 4))
			{
				printf("'%s' is not found in myfs.\n", name1);
				return ;
			}
			else	//"name1"의 아이노드 저장
				tmpidNum = pIndex -> pData -> directory.idNum[i];
		}
	}
	
	FILE *ofp;
	ofp = fopen(name2, "w");

	for(int i = 0; i < (ind[tmpidNum].fileSize / 128); i++)
	{
		dbNum = readDbNuminID(ind + tmpidNum, pDB, i);
		pDNode = createDBNode(pDB + dbNum);
		insertDBNode(&pTail, pDNode);
	}

	for(DNode *pIndex = Head.pNext; pIndex != NULL; pIndex = pIndex -> pNext)
		for(int i = 0; i < 128; i++)
		{
			putchar(pIndex -> pData -> file[i]);
			putc(pIndex -> pData -> file[i], ofp);
		}

}


int main(void)
{
	SuperBlock spblock ;

	Inode inode[512];
	inode[0].direct = 0;
	inode[0].fileType = 1;
	inode[0].fileSize = 16;
	inode[0].time = time(NULL);

	Data dataBlock[1024] ;

	initDataBlock(dataBlock);

	dataBlock[0].directory.name[0][0] = '/';
	dataBlock[0].directory.name[0][1] = 0;
	dataBlock[0].directory.name[1][0] = '.';
	dataBlock[0].directory.name[1][1] = '.';
	dataBlock[0].directory.name[1][2] = 0;
	dataBlock[0].directory.idNum[0] = 0;
	dataBlock[0].directory.idNum[1] = 0;

	TNode Root ={NULL,0,NULL} ;
	TNode *pTNode = NULL;
	TNode *pwd = &Root;
	TNode *uPwd = &Root;
	TNode *tPwd = pwd;
	TNode *tUwd = uPwd;

	SNode SHAf = {{0},NULL};
	SNode SHBf = {{0},NULL};
	SNode *pSNode = NULL;

	int idNum;
	int dbNum;
	int flag = 0;

			char tmpname[10];
	initsuperBlock(&spblock);
	markInode(spblock.usableInode, 0, 1);
	markdataBlock(spblock.usabledataBlock, 0, 1);


	while(1)
	{
		printf("[%s]$ ",dataBlock[0].directory.name[0]);
		procArgument(&SHBf);
		upsidedownStack(&SHBf,&SHAf);

		if(strcmp("mymkdir",SHAf.pNext->data) == 0)
		{
			for(pSNode =SHAf.pNext; pSNode != NULL ;)
			{
				deleteSNode(&SHAf, pSNode);
				pSNode = SHAf.pNext;
				if(pSNode == NULL)	{ break ; }
				makeDirectory(&spblock, inode, dataBlock, pwd, pSNode->data);
			}
		}
		else if(strcmp("myrmdir",SHAf.pNext->data) == 0)
		{
			for(pSNode =SHAf.pNext; pSNode != NULL ; )
			{
				deleteSNode(&SHAf, pSNode);
				pSNode = SHAf.pNext;
				if(pSNode == NULL)	{ break ; }
				removeDirectory(&spblock, inode, dataBlock, pwd, pSNode->data);
			}
		}
		else if(strcmp("mytree",SHAf.pNext->data) == 0)
		{
			deleteSNode(&SHAf, SHAf.pNext);
			pSNode = SHAf.pNext;
			if(pSNode == NULL)		
			{
				tPwd = &Root;
				tUwd = &Root;
			}
			else 
			{
				if(strcmp(pSNode->data,"/") == 0)
				{
					for(;pSNode != NULL;)
					{
						absolutePath(inode, dataBlock, &Root, &tPwd, &tUwd,pSNode);
						deleteSNode(&SHAf, pSNode);
						pSNode = SHAf.pNext;
					}
				}
				else 
				{
					for(;pSNode!=NULL;)
					{
						relativePath(inode,&Root,&tPwd,&tUwd,dataBlock,pSNode);
						deleteSNode(&SHAf, pSNode);
						pSNode = SHAf.pNext;
					}
				}
			}

			if(tPwd->idNum == Root.idNum)
				preorderTraverse(pwd, inode, dataBlock,0);
			else 
			{
				idNum = tPwd->idNum;
				dbNum = inode[idNum].direct;
				preorderTraverse(tPwd,inode,dataBlock,0);
			}
			tPwd = pwd;
			tUwd = uPwd;
		}
		else if(strcmp("pwd",SHAf.pNext->data)==0)
		{
			for(pSNode =SHAf.pNext; pSNode != NULL ; )
			{
				deleteSNode(&SHAf, pSNode);
				pSNode = SHAf.pNext;
				printPwd(inode, dataBlock, &Root, pwd, &uPwd);
			}
		}
		else if(strcmp("cd",SHAf.pNext->data) == 0)
		{
			deleteSNode(&SHAf, SHAf.pNext);
			pSNode = SHAf.pNext;
			if(pSNode == NULL)		
			{
				tPwd = &Root;
				tUwd = &Root;
			}
			else 
			{
				if(strcmp(pSNode->data,"/") == 0)
				{
					for(;pSNode != NULL;)
					{
						absolutePath(inode, dataBlock, &Root, &tPwd, &tUwd,pSNode);
						deleteSNode(&SHAf, pSNode);
						pSNode = SHAf.pNext;
					}
				}
				else 
				{
					for(;pSNode!=NULL;)
					{
						relativePath(inode,&Root,&tPwd,&tUwd,dataBlock,pSNode);
						deleteSNode(&SHAf, pSNode);
						pSNode = SHAf.pNext;
					}
				}
			}
			mycd(&pwd, &uPwd, tPwd,tUwd);
			//			printf("pwd->idNum:%d\n",pwd->idNum);
			//			printf("uPwd->idNum:%d\n",uPwd->idNum);
		}
		else if(strcmp("myls",SHAf.pNext->data) == 0)
		{
			deleteSNode(&SHAf, SHAf.pNext);
			pSNode = SHAf.pNext;
			if(pSNode != NULL &&strcmp(pSNode->data,"-i") == 0)
			{
				flag = 1;
				deleteSNode(&SHAf, SHAf.pNext);
				pSNode = SHAf.pNext;
			}
			else if(pSNode != NULL && strcmp(pSNode->data,"-l")==0)
			{
				flag = 2;
				deleteSNode(&SHAf, SHAf.pNext);
				pSNode = SHAf.pNext;
			}
			else if(pSNode != NULL && (strcmp(pSNode->data,"-il")==0 || strcmp(pSNode->data,"-li")==0))
			{
				flag = 3;
				deleteSNode(&SHAf, SHAf.pNext);
				pSNode = SHAf.pNext;
			}

			if(pSNode == NULL)		
			{
				tPwd = pwd;
				tUwd = uPwd;
			}
			else 
			{

				if(strcmp(pSNode->data,"/") == 0)
				{


					for(;pSNode != NULL;)
					{
						absolutePath(inode, dataBlock, &Root, &tPwd, &tUwd,pSNode);
						deleteSNode(&SHAf, pSNode);
						pSNode = SHAf.pNext;
					}
				}
				else 
				{
					for(;pSNode!=NULL;)
					{
						relativePath(inode,&Root,&tPwd,&tUwd,dataBlock,pSNode);
						deleteSNode(&SHAf, pSNode);
						pSNode = SHAf.pNext;
					}
				}
			}
			myls(inode,dataBlock, tPwd, flag);
			flag = 0;


		}
		else if(strcmp("mytouch",SHAf.pNext->data)==0)
		{
			deleteSNode(&SHAf, SHAf.pNext);
			pSNode = SHAf.pNext;
			if(pSNode == NULL)		
				flag = 1;
			else 
			{
				if(strcmp(pSNode->data,"/") == 0)
				{
					for(;pSNode->pNext != NULL;)
					{
						absolutePath(inode, dataBlock, &Root, &tPwd, &tUwd,pSNode);
						if(pSNode->pNext != NULL)
						{
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
					}
				}
				else 
				{
					for(;pSNode->pNext!=NULL;)
					{
						relativePath(inode,&Root,&tPwd,&tUwd,dataBlock,pSNode);
						if(pSNode->pNext != NULL)
						{
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
					}
				}
			}
			if(flag == 1)
				printf("인자를 입력하세요\n");
			else
			{
				printf("name:%s\n",pSNode->data);
				mytouch(&spblock,inode,dataBlock,tPwd,pSNode->data);
			}

		}

		else if(strcmp("mycpfrom", SHAf.pNext -> data) == 0)
		{
			deleteSNode(&SHAf, SHAf.pNext);
			pSNode = SHAf.pNext;
			if(pSNode == NULL)		
				flag = 1;
			else 
			{
				if(strcmp(pSNode->data,"/") == 0)
				{
					for(;pSNode->pNext != NULL;)
					{
						absolutePath(inode, dataBlock, &Root, &tPwd, &tUwd,pSNode);
						if(pSNode->pNext != NULL)
						{
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
					}
				}
				else 
				{
					for(;pSNode->pNext!=NULL;)
					{
						relativePath(inode,&Root,&tPwd,&tUwd,dataBlock,pSNode);
						if(pSNode->pNext != NULL)
						{
							strncpy(tmpname, pSNode -> data, 4);
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
						else
						{
						}
					}
				}
			}
			if(flag == 1)
				printf("인자를 입력하세요\n");
			else
			{
				printf("1 : %s\n", tmpname);
				printf("2 :%s\n",pSNode->data);
				f_mycpfrom(tmpname, pSNode -> data, &spblock,inode,dataBlock,tPwd);
			}

		}
		else if(strcmp("mycpto", SHAf.pNext -> data) == 0)
		{
			deleteSNode(&SHAf, SHAf.pNext);
			pSNode = SHAf.pNext;
			if(pSNode == NULL)		
				flag = 1;
			else 
			{
				if(strcmp(pSNode->data,"/") == 0)
				{
					for(;pSNode->pNext != NULL;)
					{
						absolutePath(inode, dataBlock, &Root, &tPwd, &tUwd,pSNode);
						if(pSNode->pNext != NULL)
						{
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
					}
				}
				else 
				{
					for(;pSNode->pNext!=NULL;)
					{
						relativePath(inode,&Root,&tPwd,&tUwd,dataBlock,pSNode);
						if(pSNode->pNext != NULL)
						{
							strncpy(tmpname, SHAf.pNext -> data, 4);
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
						else
						{
						}
					}
				}
			}
			if(flag == 1)
				printf("인자를 입력하세요\n");
			else
			{
				printf("1 : %s\n", tmpname);
				printf("2 :%s\n",pSNode->data);
				f_mycpto(tmpname, pSNode -> data, &spblock,inode,dataBlock,tPwd);
			}

		}

		else if(strcmp("rm",SHAf.pNext->data)==0)
		{
			deleteSNode(&SHAf, SHAf.pNext);
			pSNode = SHAf.pNext;
			if(pSNode == NULL)		
				flag = 1;
			else 
			{
				if(strcmp(pSNode->data,"/") == 0)
				{
					for(;pSNode->pNext != NULL;)
					{
						absolutePath(inode, dataBlock, &Root, &tPwd, &tUwd,pSNode);
						if(pSNode->pNext != NULL)
						{
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
					}
				}
				else 
				{
					for(;pSNode->pNext!=NULL;)
					{
						relativePath(inode,&Root,&tPwd,&tUwd,dataBlock,pSNode);
						if(pSNode->pNext != NULL)
						{
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
					}
				}
			}
			if(flag == 1)
				printf("인자를 입력하세요\n");
			else
			{
				rm(&spblock, inode, dataBlock, tPwd, pSNode->data);
			}


		}
		else if(strcmp("byebye",SHAf.pNext->data) == 0)
		{
			for(pSNode =SHAf.pNext; pSNode != NULL ;pSNode = pSNode->pNext)
				deleteSNode(&SHAf, pSNode);
			break;
		}
		else if(SHAf.pNext -> data[0] != 'm' || SHAf.pNext -> data[1] != 'y')
		{
			strncpy(tmpname, SHAf.pNext -> data, 4);
			deleteSNode(&SHAf, SHAf.pNext);
			pSNode = SHAf.pNext;
			if(pSNode == NULL)		
				flag = 1;
			else 
			{
				if(strcmp(pSNode->data,"/") == 0)
				{
					for(;pSNode->pNext != NULL;)
					{
						absolutePath(inode, dataBlock, &Root, &tPwd, &tUwd,pSNode);
						if(pSNode->pNext != NULL)
						{
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
					}
				}
				else 
				{
					for(;pSNode->pNext!=NULL;)
					{
						relativePath(inode,&Root,&tPwd,&tUwd,dataBlock,pSNode);
						if(pSNode->pNext != NULL)
						{
							deleteSNode(&SHAf, pSNode);
							pSNode = SHAf.pNext;
						}
					}
				}
			}
			if(pSNode -> data != NULL)
			{
				strcat(tmpname, " ");
				strcat(tmpname, pSNode -> data);
			}

			printf("1 : %s\n", tmpname);
			system(tmpname);

		}

		for(pSNode =SHAf.pNext; pSNode != NULL ;pSNode = pSNode->pNext)
			deleteSNode(&SHAf, pSNode);
		/*

		   for(int i=0;i<4;i++)
		   {
		   idNum = Root.idNum;
		   dbNum = inode[idNum].direct;
		   printf("[]name:%s, idNum:%d\n",dataBlock[dbNum].directory.name[i], dataBlock[dbNum].directory.idNum[i]);
		   }
		   putchar('\n');
		   */
#if DEBUG
		{
			printf("			INODEPRINT\n");
			for(int i = 0; i < 10; i++)
			{
				printf("   [#%d IND] ", i);
				printf("Type : %d | ", inode[i].fileType);
				printf("Size : %3d | ", inode[i].fileSize);
				printf("DB : %d", inode[i].direct);
				printf(" %d", inode[i].sindirect);
				printf(" %d |", inode[i].dlindirect);
				printf("Time : %ld\n", inode[i].time);
			}
			printf("	  	      ROOTDIRECTORY\n");
			for(int i = 0; i < 10; i++)
			{
				printf("		    %4d", dataBlock[0].directory.idNum[i]);
				printf("      ");
				for(int j = 0; j < 4; j++)
				{
					printf("%c", dataBlock[0].directory.name[i][j]);
				}
				putchar('\n');
			}
			printBit(spblock.usableInode[0]);
			printBit(spblock.usabledataBlock[0]);
		}
#endif
	}


	for(int i=0;i<16;i++)
		printf("name:%s, idNum:%d\n",dataBlock[0].directory.name[i],dataBlock[0].directory.idNum[i]);

	return 0;
}
