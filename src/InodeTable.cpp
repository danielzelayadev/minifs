#include "InodeTable.h"

#include <string.h>

#include <iostream>

using namespace std;

InodeTable::InodeTable(int inodeCount)
{
    tableSize = inodeCount;
    this->inodeTable = new InodeInfo[inodeCount];
}

InodeTable::InodeTable(InodeInfo* inodeTable, int tableSize)
{
    this->tableSize = tableSize;
    this->inodeTable = new InodeInfo[(tableSize*sizeof(InodeInfo))];
    memcpy(this->inodeTable, inodeTable, (tableSize*sizeof(InodeInfo)));
}

InodeTable::~InodeTable()
{
    delete inodeTable;
}

InodeInfo* InodeTable::getInodeInfo(char* fileName)
{
    for(int i = 0; i < tableSize; i++)
    {
        if(strcmp(inodeTable[i].songName, fileName) == 0)
          return &inodeTable[i];
    }
    return nullptr;
}

InodeInfo* InodeTable::getInodeInfo(int iNumber)
{
   return &inodeTable[iNumber];
}

InodeInfo* InodeTable::getFreeInodeInfo()
{
    for(int i = 0; i < tableSize; i++)
    {
        if(inodeTable[i].free)
          return &inodeTable[i];
    }

    return nullptr;
}

int InodeTable::getTableSize()
{
  return tableSize;
}

void InodeTable::printTable()
{
   for(int i = 0; i < tableSize; i++)
     printInodeInfo(i);
}

void InodeTable::printInodeInfo(char* fileName)
{
   InodeInfo* iInfo = getInodeInfo(fileName);
   printInodeInfo(iInfo);
}

void InodeTable::printInodeInfo(int iNumber)
{
   InodeInfo* iInfo = getInodeInfo(iNumber);
   printInodeInfo(iInfo);
}

void InodeTable::printInodeInfo(InodeInfo* iInfo)
{
   if(!iInfo) return;

       cout << "Song Name: " << inodeTable[iInfo->iNumber].songName << endl;
       cout << "iNumber: " << inodeTable[iInfo->iNumber].iNumber << endl;
       cout << "Free: " << (inodeTable[iInfo->iNumber].free ? "Yes" : "No") << endl;
       cout << endl;

   cout << endl;
}
