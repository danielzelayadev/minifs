#include "VirtualDisk.h"

#include <iostream>
#include <string.h>

#define INODE_BLOCK_PERCENTAGE 0.07

VirtualDisk::VirtualDisk(char diskName[30], char partitionChar, int blockSize, int blockCount)
{
    superBlock = nullptr;
    bitmap = nullptr;
    inodeTable = nullptr;

    initialize(diskName, partitionChar, blockCount, blockCount);
}

VirtualDisk::VirtualDisk(char diskName[30])
{
    disk = new fstream(diskName, ios::binary | ios::in | ios::out | ios::ate);
    loadSuperBlock();
    loadBitmap();
    loadInodeTable();
}

VirtualDisk::~VirtualDisk()
{
    disk->close();
    delete superBlock;
    delete bitmap;
    delete inodeTable;
    delete disk;
}

void VirtualDisk::initialize(char diskName[30], char partitionChar, int blockSize, int blockCount)
{
     int diskSize = blockCount*blockSize;

     char* buffer = new char[diskSize];

     disk->write(buffer, diskSize);

     createSuperBlock(diskName, partitionChar, diskSize, blockSize, blockCount);

     _SuperBlock sb = *superBlock->getStruct();

     createBitmap(blockCount);
     createInodeTable(sb.inodeCount);
     createInodes(sb.inodeCount);

     delete buffer;
}

void VirtualDisk::createSuperBlock(char diskName[30], char partitionChar, int diskSize, int blockSize, int blockCount)
{
      int byteCount = (blockCount / 8);
      if(byteCount % 8 != 0) byteCount++;

      int bitmapBlockCount =  byteCount / blockSize;
      if(bitmapBlockCount % blockSize != 0) bitmapBlockCount++;

      int inodeSize = sizeof(_Inode);
      int inodeBlockCount = ( ( diskSize - ( (1+bitmapBlockCount)*blockSize) ) * INODE_BLOCK_PERCENTAGE) / blockSize;
      int inodeCount = (inodeBlockCount*blockSize) / inodeSize;

      int inodeTableBlockCount = (inodeCount*sizeof(InodeInfo)) / blockSize;
      if( (inodeCount*sizeof(InodeInfo)) % blockSize != 0) inodeTableBlockCount++;

      int dataBlockCount = blockCount - 1 - bitmapBlockCount - inodeTableBlockCount - inodeBlockCount;

      _SuperBlock sb;

      strcpy(sb.diskName, diskName);

      sb.blockCount = blockCount;
      sb.blockSize = blockSize;
      sb.filesOnDisk = 0;
      sb.dataBlockCount = dataBlockCount;
      sb.diskSize = diskSize;
      sb.inodeTableBlockCount = inodeTableBlockCount;
      sb.usedSpace = blockSize + (bitmapBlockCount*blockSize) + (inodeTableBlockCount*blockSize) + (inodeBlockCount*blockSize);
      sb.freeSpace = diskSize - sb.usedSpace;
      sb.usedBlocks = 2 + sb.inodeTableBlockCount + inodeBlockCount;
      sb.freeBlocks = blockCount - sb.usedBlocks;
      sb.initBlocks = 10;
      sb.blocksPerSI = blockSize / 4;
      sb.siPerDI = blockSize / 4;
      sb.blocksPerDI = sb.blocksPerSI * sb.siPerDI;
      sb.inodeBlockCount = inodeBlockCount;
      sb.usedInodes = 0;
      sb.freeInodes = inodeCount;
      sb.inodeCount = inodeCount;
      sb.inodeSize = inodeSize;
      sb.partitionChar = partitionChar;
      sb.freeDataBlocks = dataBlockCount;
      sb.usedDataBlocks = 0;
      sb.freeDataSpace = sb.diskSize - sb.usedSpace;
      sb.usedDataSpace = 0;
      sb.bitmapBlockCount = bitmapBlockCount;

      superBlock = new SuperBlock(sb);

      flushSuperBlock();
}

void VirtualDisk::flushSuperBlock()
{
     disk->seekp(0);

     _SuperBlock sb = *superBlock->getStruct();

     disk->write((char*)&sb, sizeof(_SuperBlock));
}

void VirtualDisk::createBitmap(int blockCount)
{
    bitmap = new Bitmap(blockCount);

    _SuperBlock sb = *superBlock->getStruct();

    int metadaBlockCount = 1 + sb.bitmapBlockCount + sb.inodeTableBlockCount + sb.inodeBlockCount;

    for(int i = 0; i < metadaBlockCount; i++)
        bitmap->turnOnBit(i);

    flushBitmap();
}

void VirtualDisk::flushBitmap()
{
   disk->seekp(superBlock->getStruct()->blockSize);

   char* bmp = bitmap->getBitmap();

   disk->write(bmp, bitmap->getSize());
}

void VirtualDisk::createInodeTable(int inodeCount)
{
    inodeTable = new InodeTable(inodeCount);

    for(int i = 0; i < inodeCount; i++)
    {
       InodeInfo* iInfo = inodeTable->getInodeInfo(i);
       iInfo->iNumber = i;
       iInfo->free = true;
    }

    flushInodeTable();
}

void VirtualDisk::flushInodeTable()
{
    _SuperBlock* sb = superBlock->getStruct();
    disk->seekp(sb->blockSize * (1+sb->bitmapBlockCount));

    for(int i = 0; i < sb->inodeCount; i++)
    {
       InodeInfo iInfo = *inodeTable->getInodeInfo(i);
       disk->write((char*)&iInfo, sizeof(InodeInfo));
    }
}

void VirtualDisk::createInodes(int inodeCount)
{
  for(int i = 0; i < inodeCount; i++)
  {
    _Inode inode;
    inode.iNumber = i;
    inode.openMode[0] = 'r';
    flushInode(inode);
  }

}

void VirtualDisk::flushInode(_Inode inode)
{
   _SuperBlock sb = *superBlock->getStruct();

   disk->seekp(sb.blockSize * (1+sb.bitmapBlockCount+sb.inodeTableBlockCount) + (sizeof(_Inode) * inode.iNumber));

   disk->write((char*)&inode, sizeof(_Inode));

}

void VirtualDisk::loadSuperBlock()
{
}

void VirtualDisk::loadBitmap()
{
}

void VirtualDisk::loadInodeTable()
{
}

void VirtualDisk::printSuperBlock()
{
   superBlock->printSuperBlock();
}

void VirtualDisk::printBitmap()
{
  for(int i = 0; i < bitmap->getSize(); i++)
  {
    cout << bitmap->bitIsOn(i);

    if(i % 8 == 0) cout << endl;
  }
}

void VirtualDisk::printInodeTable()
{
   inodeTable->printTable();
}
