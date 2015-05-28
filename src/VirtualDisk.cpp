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
    if(disk->is_open())
    {
       loadSuperBlock();
       loadBitmap();
       loadInodeTable();
    }
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

   disk->seekp( (sb.blockSize * (1+sb.bitmapBlockCount+sb.inodeTableBlockCount)) + (sizeof(_Inode) * inode.iNumber));

   disk->write((char*)&inode, sizeof(_Inode));

}

void VirtualDisk::writeFile(char* fileName)
{
   ifstream* file = new ifstream(fileName, ios::binary | ios::ate);

   int fileSize = file->tellg();

   file->seekg(0);

   _SuperBlock* sb = superBlock->getStruct();

   int blocksNeeded = 0;

   if(checkEnoughSpace(fileSize, &blocksNeeded) && sb->freeInodes > 0 && !isRepeated(fileName))
   {
       //Buscar en InodeTable
       InodeInfo* iInfo = inodeTable->getFreeInodeInfo();

       iInfo->free = false;
       strcpy(iInfo->songName, fileName);

       //Conseguir inode asociado
       loadedInode = loadInode(iInfo->iNumber);
       loadedInode->getStruct()->blocksInUse = blocksNeeded;
       loadedInode->getStruct()->fileSize = fileSize;

       //Alocar bloques para el archivo
       alloc_blocks(loadedInode->getStruct());

       cout << blocksNeeded << endl;
       //Escribir el archivo en los bloques
       write(file, loadedInode->getStruct());

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
       flushInode(*loadedInode->getStruct());
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
       writeToDIBlock(file, inode, &bn);

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

   for(int i = 0; (*bn) > 0 && i < sb.siPerDI; i++)
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
      alloc_sIndirect(inode, &blocksNeeded, &inode->singleIndirectBlock);

   if(blocksNeeded != 0)
      alloc_dIndirect(inode, &blocksNeeded);

}
int hmt = 0;
void VirtualDisk::alloc_sIndirect(_Inode* inode, int* blocksNeeded, int* siDir)
{
    *siDir = alloc_block();

    _SuperBlock sb = *superBlock->getStruct();

    int* pointers = new int[sb.blocksPerSI];

    int bn = *blocksNeeded;

    for(int i = 0; (*blocksNeeded) > 0 && i < sb.blocksPerSI; i++, (*blocksNeeded)--)
        {hmt++; cout << hmt << endl; pointers[i] = alloc_block();}

    writePointersToBlock(pointers, *siDir);

//    if((*blocksNeeded) == 0) //FDI)
//   {
//        disk->seekp(320296960);//disk->tellp()-sb.blockSize);
//
//        disk->read((char*)pointers, sb.blockSize);
//
//        for(int i = 0; bn > 0 && i < sb.blocksPerSI; i++,bn--)
//        cout << "Pointer " << i << " at SI Block: " << pointers[i] << endl;
//   }

    delete pointers;
}

void VirtualDisk::alloc_dIndirect(_Inode* inode, int* blocksNeeded)
{
   inode->doubleIndirectBlock = alloc_block();
//   cout << inode->doubleIndirectBlock << endl;

   _SuperBlock sb = *superBlock->getStruct();

   int* pointers = new int[sb.siPerDI];

   for(int i = 0; (*blocksNeeded) > 0 && i < sb.siPerDI; i++)
   {
       alloc_sIndirect(inode, blocksNeeded, &pointers[i]); //Alocamos los necesarios directos
       cout << "SI Block Dir en la Pos " << i << " del DI: " << pointers[i] << endl;
   }

   writePointersToBlock(pointers, inode->doubleIndirectBlock);

   delete pointers;
}

void VirtualDisk::writePointersToBlock(int* pointers, int blockPos)
{
    _SuperBlock sb = *superBlock->getStruct();

    disk->seekp(blockPos*sb.blockSize);
//    cout << "Block Pos: " << blockPos << "\tDisk Pos: " <<blockPos*sb.blockSize << endl;

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
   disk->seekp( (sb.blockSize*(1+sb.bitmapBlockCount+sb.inodeTableBlockCount))+(sizeof(_Inode)*iNumber));

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

   if(!data) return;

   ofstream* exportedFile = new ofstream(destination, ios::binary);

   exportedFile->write(data, loadedInode->getStruct()->fileSize);

//   cout << data  << endl;

   exportedFile->close();

//   flushInode(*loadedInode->getStruct());
   delete exportedFile;
   delete loadedInode;
}

void VirtualDisk::deleteFile(char* fileName)
{
   _SuperBlock* sb = superBlock->getStruct();

   InodeInfo* iInfo = inodeTable->getInodeInfo(fileName);

   if(!iInfo) return;

   iInfo->free = true;

   loadedInode = loadInode(iInfo->iNumber);
   _Inode* inode = loadedInode->getStruct();
   inode->deleteTime = time(NULL);

   int blocksNeeded = inode->blocksInUse;

   //Liberar bloques del bitmap(todo el rollo de conseguir los pointers, recorrer en el bitmap y apagar)

   //Update and flush
       sb->filesOnDisk--;
       sb->freeBlocks += blocksNeeded;
       sb->freeDataBlocks += blocksNeeded;
       sb->freeDataSpace += (blocksNeeded*sb->blockSize);
       sb->freeInodes++;
       sb->freeSpace += (blocksNeeded*sb->blockSize);
       sb->usedBlocks -= blocksNeeded;
       sb->usedDataBlocks -= blocksNeeded;
       sb->usedDataSpace -= (blocksNeeded*sb->blockSize);
       sb->usedInodes--;
       sb->usedSpace -= (blocksNeeded*sb->blockSize);

       flushSuperBlock();
       flushBitmap();
       flushInodeTable();
       flushInode(*loadedInode->getStruct());
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
   InodeInfo* iInfo = inodeTable->getInodeInfo(iNumber);

   if(!iInfo) return;

   _SuperBlock sb = *superBlock->getStruct();

   disk->seekp( (sb.blockSize* (1+sb.bitmapBlockCount+sb.inodeTableBlockCount)) + (sizeof(_Inode)*iNumber));

   _Inode inode;

   disk->read((char*)&inode, sizeof(_Inode));

   Inode fileInode(inode);

   fileInode.printInode();
}

char* VirtualDisk::loadFile(char* fileName)
{
   _SuperBlock sb = *superBlock->getStruct();

   InodeInfo* iInfo = inodeTable->getInodeInfo(fileName);

   if(!iInfo) return nullptr;

   loadedInode = loadInode(iInfo->iNumber);
   _Inode inode = *loadedInode->getStruct();

   char* fileData = new char[inode.fileSize];

   int bn = inode.blocksInUse;
   int blockIndex = 0;

//   int initCount = 0, siCount =  0, diCount = 0, lastBlockCount = 0;
//
//   calculateBlocksNeededByCat(bn, &initCount, &siCount, &diCount, &lastBlockCount);

   for(int i = 0; bn > 0 && i < sb.initBlocks; i++, blockIndex++, bn--)
   {
        char* blockData = new char[sb.blockSize];

        disk->seekp(sb.blockSize*inode.blocks[i]);

        disk->read(blockData, sb.blockSize);

        loadBlockData(fileData, blockData, blockIndex);
   }

   if(bn > 0)
      loadSI(sb, inode, inode.singleIndirectBlock, fileData, &blockIndex, &bn);

   if(bn > 0)
   {
      int* diPointers = getDIndirectPointers(inode.doubleIndirectBlock);

      for(int i = 0; bn > 0 && i < sb.siPerDI; i++)
          loadSI(sb, inode, diPointers[i], fileData, &blockIndex, &bn);
   }

   return fileData;
}

void VirtualDisk::loadBlockData(char* fileData, char* blockData, int blockIndex)
{
    _SuperBlock sb = *superBlock->getStruct();

    for(int i = 0; i < sb.blockSize; i++)
    {
       int pos = (sb.blockSize * blockIndex)+i;

       if(pos >= loadedInode->getStruct()->fileSize) return;

       fileData[pos] = blockData[i];
    }//Se pega un poco de basura asi que hay que cancelar cuando el
       //archivo se termine
}
int ctr = 1;
void VirtualDisk::loadSI(_SuperBlock sb, _Inode inode, int siDir, char* fileData, int* blockIndex, int* bn)
{
   int* siPointers = getSIndirectPointers(siDir);

      for(int i = 0; (*bn) > 0 && i < sb.blocksPerSI; i++, (*blockIndex)++, (*bn)--)
      {
        char* blockData = new char[sb.blockSize];

        disk->seekp(sb.blockSize*siPointers[i]);

        disk->read(blockData, sb.blockSize);

        loadBlockData(fileData, blockData, (*blockIndex));
//        if(ctr == 2 && i == 0)for(int i = 0; i < 300; i++) cout << blockData[i] << endl;
//        cout << "Block Index: " << *blockIndex << "\tBlock Dir: " << siPointers[i] << "\tBlocks Needed: " << *bn << endl;

        delete blockData;
      }
ctr++;
}

int* VirtualDisk::getSIndirectPointers(int siBlockDir)
{
    _SuperBlock sb = *superBlock->getStruct();

    disk->seekp(sb.blockSize*siBlockDir);

    int* siPointers = new int[sb.blocksPerSI];

    disk->read((char*)siPointers, sb.blockSize);

    return siPointers;
}

int* VirtualDisk::getDIndirectPointers(int diBlockDir)
{
    return getSIndirectPointers(diBlockDir);
}

void VirtualDisk::calculateBlocksNeededByCat(int blocksNeeded, int* initCount, int* siCount, int* diCount, int* lastSIBlockCount)
{
   cout << "Blocks Needed: " << blocksNeeded << endl;

   int blockSize = superBlock->getStruct()->blockSize;
   int initBlocks = superBlock->getStruct()->initBlocks, siBlocks = superBlock->getStruct()->blocksPerSI,
   diSIs = superBlock->getStruct()->siPerDI;

   *initCount = 0, *siCount = 0, *diCount = 0, *lastSIBlockCount = 0;

    if(blocksNeeded >= initBlocks)
    {
       (*initCount) = 10;
       blocksNeeded -= (*initCount);

       if(blocksNeeded >= siBlocks)
       {
          (*siCount)++;
          blocksNeeded -= siBlocks;

          if(blocksNeeded > 0)
          {
             (*diCount) = 1;

             int siNeeded = blocksNeeded / siBlocks;
             (*lastSIBlockCount) = blocksNeeded % siBlocks;
             if((*lastSIBlockCount) != 0) siNeeded++;

             siCount += siNeeded;

             blocksNeeded = 0;

          }

       }

       else (*siCount) = blocksNeeded;

    }
    else (*initCount) = blocksNeeded;

//    cout << "Init Needed: " << (*initCount) << endl;
//    cout << "SI Needed: " << (*siCount) << endl;
//    cout << "DI Needed: " << (*diCount) << endl;
//    cout << "REM: " << (*lastSIBlockCount) << endl;
}

int VirtualDisk::getFileSize(char* fileName)
{
   InodeInfo info = *inodeTable->getInodeInfo(fileName);

   Inode* inode = loadInode(info.iNumber);

   int fileSize = inode->getStruct()->fileSize;

   delete inode;

   return fileSize;
}

void VirtualDisk::printInodeTableOccupied()
{
   inodeTable->printOccupied();
}

void VirtualDisk::testOut()
{
  _SuperBlock sb = *superBlock->getStruct();
  int* pointers = new int[sb.blocksPerSI];
   disk->seekp(320296960);//disk->tellp()-sb.blockSize);

        disk->read((char*)pointers, sb.blockSize);

        for(int i = 0; i < 1110; i++)
        cout << "Pointer " << i << " at SI Block: " << pointers[i] << endl;
}
