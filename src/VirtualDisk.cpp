#include "VirtualDisk.h"

#include <iostream>
#include <string.h>
#include <ctime>
#include <cmath>

#define INODE_BLOCK_PERCENTAGE 0.07

VirtualDisk::VirtualDisk(char diskName[30], char partitionChar, int blockSize, int blockCount)
{
    superBlock = nullptr;
    bitmap = nullptr;
    inodeTable = nullptr;

    initialize(diskName, partitionChar, blockSize, blockCount);
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
     disk = new fstream(diskName, ios::binary | ios::out);

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
      if(byteCount % blockSize != 0) bitmapBlockCount++;

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
      sb.metadataBlockCount = 1 + bitmapBlockCount + inodeTableBlockCount + inodeBlockCount;

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

    for(int i = 0; i < sb.metadataBlockCount; i++)
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
    InodeInfo* table = new InodeInfo[inodeCount];

    for(int i = 0; i < inodeCount; i++)
    {
       InodeInfo* iInfo = &table[i];
       iInfo->iNumber = i;
       iInfo->free = true;
    }
    inodeTable = new InodeTable(table, inodeCount);

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

void VirtualDisk::writeFile(char* fileName)
{
   ifstream* file = new ifstream(fileName, ios::binary | ios::ate);

   int fileSize = file->tellg();

   file->seekg(0);

   _SuperBlock* sb = superBlock->getStruct();

   int blocksNeeded = 0;

   if(checkEnoughSpace(fileSize, &blocksNeeded) && !isRepeated(fileName))
   {
       //Buscar en InodeTable
       InodeInfo* iInfo = inodeTable->getFreeInodeInfo();

       iInfo->free = false;
       strcpy(iInfo->songName, fileName);

       //Conseguir inode asociado
       Inode* fileInode = loadInode(iInfo->iNumber);
       fileInode->getStruct()->blocksInUse = blocksNeeded;
       fileInode->getStruct()->fileSize = fileSize;

       //Alocar bloques para el archivo
       alloc_blocks(fileInode->getStruct());

       //Escribir el archivo en los bloques
       write(file, fileInode->getStruct());

       //Update  structs and flush to disk
       sb->filesOnDisk++;
       sb->freeBlocks -= blocksNeeded;
       sb->freeDataBlocks -= blocksNeeded;
       sb->freeDataSpace -= (blocksNeeded*sb->blockSize);
       sb->freeInodes--;
       sb->freeSpace -= (blocksNeeded*sb->blockSize);
       sb->usedBlocks += blocksNeeded;
       sb->usedDataBlocks += blocksNeeded;
       sb->usedDataSpace += (blocksNeeded*sb->blockSize);
       sb->usedInodes++;
       sb->usedSpace += (blocksNeeded*sb->blockSize);

       flushSuperBlock();
       flushBitmap();
       flushInodeTable();
       flushInode(*fileInode->getStruct());
   }

   file->close();
   delete file;
}

void VirtualDisk::write(ifstream* file, _Inode* inode)
{
     int bn = inode->blocksInUse;
     writeToDirectBlocks(file, &bn, inode->blocks, 10);

     if(bn > 0)
       writeToSIBlock(file, inode, &bn, inode->singleIndirectBlock);

     if(bn > 0)
       writeToDIBlock(file, inode, &bn); //509 bloques, 2 600 000 bytes, 2.6 MB,

}

void VirtualDisk::writeToDirectBlocks(ifstream* file, int* bn, int* blocks, int blockCount)
{
   _SuperBlock sb = *superBlock->getStruct();

   for(int i = 0; (*bn) > 0 && i < blockCount; i++, (*bn)--)
   {
       char* data = new char[sb.blockSize];
       int blockPos = blocks[i];

       file->read(data, sb.blockSize);

       disk->seekp(blockPos*sb.blockSize);

       disk->write(data, sb.blockSize);

       delete data;
   }

}

void VirtualDisk::writeToSIBlock(ifstream* file, _Inode* inode, int* bn, int indDir)
{
     _SuperBlock sb = *superBlock->getStruct();
     int* blocks = new int[sb.blocksPerSI];

     disk->seekp(indDir*sb.blockSize);

     disk->read((char*)blocks, sb.blockSize);

     writeToDirectBlocks(file, bn, blocks, sb.blocksPerSI);

     delete blocks;
}

void VirtualDisk::writeToDIBlock(ifstream* file, _Inode* inode, int* bn)
{
   _SuperBlock sb = *superBlock->getStruct();

   disk->seekp(inode->doubleIndirectBlock*sb.blockSize);

   int dib = disk->tellp();

   for(int i = 0; bn > 0 && i < sb.siPerDI; i++, bn-=sb.blocksPerSI)
   {
       int indDir = -1;

       disk->seekp(dib+(i*4));

       disk->read((char*)&indDir, sizeof(int));

       writeToSIBlock(file, inode, bn, indDir);
   }

}

void VirtualDisk::alloc_blocks(_Inode* inode)
{
   int blocksNeeded = inode->blocksInUse;

   for(int i = 0; blocksNeeded != 0 && i < superBlock->getStruct()->initBlocks; i++, blocksNeeded--)
      inode->blocks[i] = alloc_block();

   if(blocksNeeded != 0)
      alloc_sIndirect(inode, &blocksNeeded, FI);

   if(blocksNeeded != 0)
      alloc_dIndirect(inode, &blocksNeeded);

}

void VirtualDisk::alloc_sIndirect(_Inode* inode, int* blocksNeeded, int mode)
{
    int siDir = alloc_block();

    if(mode == FI)
       inode->singleIndirectBlock = siDir;

    _SuperBlock sb = *superBlock->getStruct();

    int* pointers = new int[sb.blocksPerSI];

    int jtc = 0;

    for(int i = 0; (*blocksNeeded) > 0 && i < sb.blocksPerSI; i++, (*blocksNeeded)--)
        pointers[i] = alloc_block();

    writePointersToBlock(pointers, siDir);

    delete pointers;
}

void VirtualDisk::alloc_dIndirect(_Inode* inode, int* blocksNeeded)
{
   inode->doubleIndirectBlock = alloc_block();

   _SuperBlock sb = *superBlock->getStruct();

   int* pointers = new int[sb.siPerDI];

   for(int i = 0; (*blocksNeeded) > 0 && i < sb.siPerDI; i++, (*blocksNeeded)-=sb.blocksPerSI)
   {
       pointers[i] = alloc_block(); //Alocamos uno indirect
       alloc_sIndirect(inode, blocksNeeded, FDI); //Alocamos los necesarios directos
   }

   writePointersToBlock(pointers, inode->doubleIndirectBlock);

   delete pointers;
}

void VirtualDisk::writePointersToBlock(int* pointers, int blockPos)
{
    _SuperBlock sb = *superBlock->getStruct();

    disk->seekp(blockPos*sb.blockSize);

    disk->write((char*)pointers, sb.blockSize);
}

int VirtualDisk::alloc_block()
{
   for(int i = 0; i < bitmap->getBitCount(); i++)
   {
       if(!bitmap->bitIsOn(i))
       {
           bitmap->toggleBit(i);
           return i;
       }
   }

   return -1;
}

Inode* VirtualDisk::loadInode(int iNumber)
{
   _SuperBlock sb = *superBlock->getStruct();
   disk->seekp(1+sb.bitmapBlockCount+sb.inodeTableBlockCount+(sizeof(_Inode)*iNumber));

   _Inode inode;

   disk->read((char*)&inode, sizeof(_Inode));

   inode.creationTime = time(NULL);

   return new Inode(inode);
}

bool VirtualDisk::checkEnoughSpace(int fileSize, int* blocksNeeded)
{
  *blocksNeeded = getBlocksNeeded(fileSize);

  return superBlock->getStruct()->freeDataBlocks >= *blocksNeeded;
}

bool VirtualDisk::isRepeated(char* fileName)
{
    return inodeTable->getInodeInfo(fileName);
}

int VirtualDisk::getBlocksNeeded(int fileSize)
{
  int blocksNeeded = fileSize / superBlock->getStruct()->blockSize;
  if(fileSize % superBlock->getStruct()->blockSize != 0) blocksNeeded++;

  return blocksNeeded;
}

void VirtualDisk::exportFile(char* fileName, char* destination)
{
   char* data = loadFile(fileName);
}

void VirtualDisk::readFile(char* fileName)
{
   char* data = loadFile(fileName);
}

void VirtualDisk::deleteFile(char* fileName)
{
   //Buscar en el inode table si existe
   //Marcar como free
   //Liberar bloques del bitmap
   //Update and flush
}

void VirtualDisk::loadSuperBlock()
{
   _SuperBlock sb;

   disk->seekp(0);

   disk->read((char*)&sb, sizeof(_SuperBlock));

   superBlock = new SuperBlock(sb);

}

void VirtualDisk::loadBitmap()
{
  _SuperBlock sb = *superBlock->getStruct();
  int bitmapSize = sb.bitmapBlockCount*sb.blockSize;

  char* bmp = new char[bitmapSize];

  disk->seekp(sb.blockSize);

  disk->read(bmp, bitmapSize);

  bitmap = new Bitmap(bmp, sb.blockCount);
}

void VirtualDisk::loadInodeTable()
{
  _SuperBlock sb = *superBlock->getStruct();

  InodeInfo* iTable = new InodeInfo[sb.inodeCount];

  disk->seekp(sb.blockSize*(1+sb.bitmapBlockCount));

  disk->read((char*)iTable, (sb.inodeCount*sizeof(InodeInfo)));

  inodeTable = new InodeTable(iTable, sb.inodeCount);

}

void VirtualDisk::printSuperBlock()
{
   superBlock->printSuperBlock();
}

void VirtualDisk::printBitmap()
{
  int ons = 0, offs = 0;

  for(int i = 0; i < bitmap->getBitCount(); i++)
  {
    bool isOn = bitmap->bitIsOn(i);

    isOn ? ons++ : offs++;

    cout << isOn;

    if((i+1) % 8 == 0) cout << endl;
  }

  cout << "\nOns: " << ons << "\nOffs: " << offs << endl;
}

void VirtualDisk::printInodeTable()
{
   inodeTable->printTable();
}

void VirtualDisk::printInode(int iNumber)
{
   InodeInfo iInfo = *inodeTable->getInodeInfo(iNumber);
   cout << iInfo.free << endl;
}

char* VirtualDisk::loadFile(char* fileName)
{
   //Buscar en el inode table si existe
     //Ir al inode
     //Leer bloque por bloque y:
    //armar el archivo en un buffer de data
    //De ahi esta listo para manipularse
   InodeInfo* iInfo = inodeTable->getInodeInfo(fileName);
}
