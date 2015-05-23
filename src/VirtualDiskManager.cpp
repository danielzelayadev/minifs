#include "VirtualDiskManager.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <ctime>

#define INODE_BLOCK_PERCENTAGE 0.07

VirtualDiskManager::VirtualDiskManager()
{
    this->vdisks = nullptr;
    this->open = false;
}

VirtualDiskManager::~VirtualDiskManager()
{
   delete vdisks;
}

bool VirtualDiskManager::openVDiskManager()
{
    open = true;
}

bool VirtualDiskManager::closeVDiskManager()
{
    open = false;
}

bool VirtualDiskManager::isOpen()
{
   return open;
}

bool VirtualDiskManager::loadVirtualDisks()
{


}

bool VirtualDiskManager::unloadVirtualDisks()
{
}

bool VirtualDiskManager::createDisk(char diskName[30], int blockCount, int blockSize, int diskSize, char partitionChar)
{
    ofstream* newDisk = new ofstream(diskName, ios::binary);

    char* buffer = new char[diskSize];

    if(newDisk->is_open())
    {
       newDisk->write(buffer, diskSize);

       delete buffer;

       int inodeSize = sizeof(Inode), inodeBlockCount = (( diskSize - (3*blockSize) ) * INODE_BLOCK_PERCENTAGE) / blockSize,
           inodeCount = (inodeBlockCount*blockSize) / inodeSize,
           dataBlockCount = blockCount-(3+inodeBlockCount);

       if(createSuperblock(newDisk, diskName, partitionChar, diskSize, blockSize, blockCount, dataBlockCount,
          inodeSize, inodeCount, inodeBlockCount))
       {
            goToBlock(newDisk, 1, blockSize);

            if(createDataBitmap(newDisk, dataBlockCount))
            {
               goToBlock(newDisk, 2, blockSize);

               if(createInodeTable(newDisk, inodeCount))
               {
                    goToBlock(newDisk, getCurrentBlock(newDisk, blockSize)+1, blockSize);

                    if(createInodes(newDisk, inodeCount))
                    {
                       newDisk->close();
                       delete newDisk;
                       return true;

                    }
               }
            }
       }
    }

    return false;
}

bool VirtualDiskManager::createSuperblock(ofstream* newDisk, char diskName[30], char partitionChar, int diskSize, int blockSize,
                                        int blockCount, int dataBlockCount, int inodeSize, int inodeCount, int inodeBlockCount)
{

      newDisk->seekp(0);
//      int freeSpace, usedSpace;

      SuperBlock sp;
      strcpy(sp.diskName, diskName);
      sp.blockCount = blockCount;
      sp.blockSize = blockSize;
      sp.dataBlockCount = dataBlockCount;
      sp.diskSize = diskSize;
      sp.inodeBlockCount = inodeBlockCount;
      sp.inodeCount = inodeCount;
      sp.inodeSize = inodeSize;
      sp.partitionChar = partitionChar;

      newDisk->write((char*)&sp, sizeof(SuperBlock));

      return true;

}

bool VirtualDiskManager::createDataBitmap(ofstream* newDisk, int blockCount)
{
     int byteCount = blockCount / 8;

     char bitmap[byteCount];

     for(int i = 0; i < byteCount; i++)
     {
         bitmap[i] = 0;
         newDisk->write(&bitmap[i], sizeof(char));
     }

     return true;
}

bool VirtualDiskManager::createInodeTable(ofstream* newDisk, int inodeCount)
{

  for(int i = 0; i < inodeCount; i++)
  {
     InodeInfo iInfo;

     iInfo.iNumber = i;
     iInfo.free = true;

     newDisk->write((char*)&iInfo, sizeof(InodeInfo));
  }

  return true;
}

bool VirtualDiskManager::createInodes(ofstream* newDisk, int inodeCount)
{

  for(int i = 0; i < inodeCount; i++)
  {
    Inode inode;
    inode.iNumber = i;
    inode.openMode[0] = 'r';
    newDisk->write((char*)&inode, sizeof(Inode));
  }
  return true;
}

void VirtualDiskManager::goToBlock(ofstream* disk, int blockPos, int blockSize)
{
    disk->seekp(blockSize*blockPos);
}

int VirtualDiskManager::getCurrentBlock(ofstream* disk, int blockSize)
{
    int blockPos = -1;

    int currentPos = disk->tellp();

    blockPos = currentPos / blockSize;

    return blockPos;
}

bool VirtualDiskManager::writeToDisk(char* diskName, char* fileName)
{
    ifstream* file = new ifstream(fileName, ios::binary | ios::ate);

    if(file->is_open())
    {
       fstream* disk = new fstream(diskName, ios::binary | ios::ate | ios::in | ios::out);

       if(disk->is_open())
       {
        disk->seekg(0);

        SuperBlock sb;

        disk->read((char*)&sb, sizeof(SuperBlock));

        //Validar si hay free space y segun eso return false o seguir

        int fileSize = file->tellg();

        goToBlock((ofstream*)disk, 2, sb.blockSize);

        disk->seekg(disk->tellp());

        InodeInfo inodeTable[sb.inodeCount];

        disk->read((char*)&inodeTable, sizeof(inodeTable));


        int freeInodeIndex = -1;

        for(int i = 0; i < sb.inodeCount; i++)
        {
           if(inodeTable[i].free)
           {
              inodeTable[i].free = false;
              freeInodeIndex = i;
              break;
           }
        }


        goToBlock((ofstream*)disk, getCurrentBlock((ofstream*)disk, sb.blockSize)+1, sb.blockSize);

        disk->seekg(disk->tellp()+(freeInodeIndex*sb.inodeSize));

        Inode fileInode;

        disk->read((char*)&fileInode, sizeof(Inode));



        //Conseguir bloques libres necesarios

        int blocksNeeded = fileSize / sb.blockSize;

        if(fileSize % sb.blockSize != 0) blocksNeeded++;

        goToBlock((ofstream*)disk, 1, sb.blockSize);

        //alloc_DirectBlock() buscar un freeBlockIndex en el bitmap y flush para marcar como ocupado
        //alloc_IndirectBlock() llamar a alloc_DirectBlock (blockSize / sizeof(int)) cantidad de veces
        //alloc_DoubleIndirectBlock() llamar a alloc_IndirectBlock (blockSize / sizeof(int)) cantidad de veces

       // alloc_blocks(disk, sb, fileInode, blocksNeeded);

        //Modificar la meta data del disco

        char* data = new char[sb.blockSize];

        file->seekg(0);
        file->read(data, sb.blockSize);

        //cout << data << endl;

        goToBlock((ofstream*)disk, 50, sb.blockSize);

        disk->write(data, sb.blockSize);

        delete disk;
        return true;
       }
       delete disk;
    }

    delete file;

    return false;
}

int VirtualDiskManager::alloc_directBlock(fstream* disk, SuperBlock sb)
{
    int freeBlockIndex = -1;
    int bitIndex = -1;

    int byteCount = sb.blockCount / 8;

    if(sb.blockCount % 8 != 0) byteCount++;

    for(int i = 0; freeBlockIndex == -1 && i < byteCount; i++)
    {
       char c = 0;
       disk->read(&c, sizeof(char));

       for(int k = 0; k < 8; k++)
       {
          if(bitIsOn(c, k))
          {
             freeBlockIndex = (i*8)+k;
             bitIndex = k;
             break;
          }
       }

    }

    return freeBlockIndex;

}

int VirtualDiskManager::alloc_singleIndBlock(fstream* disk, SuperBlock sb)
{
}

int VirtualDiskManager::alloc_doubleIndBlock(fstream* disk, SuperBlock sb)
{
}

bool VirtualDiskManager::writeToBlock(fstream* disk, char* data)
{
   return false;
}

bool VirtualDiskManager::bitIsOn(char c, int bitPos)
{
   return (c & (1 << bitPos)) != 0;
}

char VirtualDiskManager::toggleBit(char c, int bitPos)
{
   return c ^ (1 << bitPos);
}

void VirtualDiskManager::alloc_blocks(fstream*disk, SuperBlock sb, Inode inode, int blocksNeeded)
{
    int blocksInArr = 10;
    int blocksPerSI = sb.blockSize / 4;
    int siPerDI = sb.blockSize / 4;
    int blocksPerDI = blocksPerSI * siPerDI;

    if(blocksNeeded <= blocksInArr)
    {
       for(int i = 0; i < blocksNeeded; i++)
          inode.blocks[i] = alloc_directBlock(disk, sb);
       return;
    }
}
