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
    this->inodeTable = inodeTable;
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
   for(int i = 0; i < tableSize; i++)
    {
        if(inodeTable[i].iNumber == iNumber)
          return &inodeTable[i];
    }
    return nullptr;
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

   for(int i = 0; i < tableSize; i++)
   {
       cout << "Song Name: " << inodeTable[i].songName << endl;
       cout << "iNumber: " << inodeTable[i].iNumber << endl;
       cout << "Free: " << (inodeTable[i].free ? "Yes" : "No") << endl;
       cout << endl;
   }

   cout << endl;
}
