#include "Inode.h"

#include <iostream>

using namespace std;

Inode::Inode(_Inode inode)
{
   this->inode = new _Inode(inode);
}

Inode::~Inode()
{
   delete inode;
}

void Inode::printInode()
{
   cout << "Artist: " << inode->artist << endl;
   cout << "Album: " << inode->album << endl;
   cout << "Duration: " << inode->songTime << endl;
   cout << "iNumber: " << inode->iNumber << endl;
   cout << "File Size: " << inode->fileSize << endl;
   cout << "Owner ID: " << inode->ownerId << endl;
   cout << "Open Mode: " << inode->openMode << endl;
   cout << "Creation Time: " << inode->creationTime << endl;
   cout << "Last Modification Time: " << inode->modTime << endl;
   cout << "Delete Time: " << inode->deleteTime << endl;
   cout << "Last Access Time: " << inode->lastAccessTime << endl;
   cout << "Blocks in Use: " << inode->blocksInUse << endl;
//   cout << "Direct Blocks in Use: " << inode->directsInUse << endl;
//   cout << "Single Indirect Blocks in Use: " << inode->sIndirectsInUse << endl;
//   cout << "Double Indirect Blocks in Use: " << inode->dIndirectsInUse << endl;
   cout << "Flags: " << inode->flags << endl;
   cout << "Direct Blocks: " << endl;
   printDirectBlocks();
   cout << "SI Block: " << inode->singleIndirectBlock << endl;
   cout << "DI Block: " << inode->doubleIndirectBlock << endl;

   cout << endl;
}

void Inode::printDirectBlocks()
{
   for(int i = 0; i < 10; i++)
      cout << inode->blocks[i] << endl;
}
_Inode* Inode::getStruct()
{
   return inode;
}
