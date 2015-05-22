#include "VirtualDiskManager.h"
#include <fstream>
#include <iostream>
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

bool VirtualDiskManager::createDisk(string diskName, int blockCount, int blockSize, int diskSize, char partitionChar)
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

bool VirtualDiskManager::createSuperblock(ofstream* newDisk, string diskName, char partitionChar, int diskSize, int blockSize,
                                        int blockCount, int dataBlockCount, int inodeSize, int inodeCount, int inodeBlockCount)
{

      newDisk->seekp(0);
//      int freeSpace, usedSpace;

      newDisk->write( diskName.c_str(), diskName.length()); //Poner un limite
      newDisk->write( &partitionChar, sizeof(char));
      newDisk->write( (char*)&diskSize, sizeof(int));
      newDisk->write( (char*)&blockSize, sizeof(int));
      newDisk->write( (char*)&blockCount, sizeof(int));
      newDisk->write( (char*)&dataBlockCount, sizeof(int));
      newDisk->write( (char*)&inodeSize, sizeof(int));
      newDisk->write( (char*)&inodeCount, sizeof(int));
      newDisk->write( (char*)&inodeBlockCount, sizeof(int));

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

  int songNameLimit = 50;

  for(int i = 0; i < inodeCount; i++)
  {
     int iNumber = i;
     char songName[songNameLimit];
     bool free = true;

     newDisk->write((char*)&iNumber, sizeof(int));
     newDisk->write(songName, songNameLimit);
     newDisk->write((char*)&free, sizeof(bool));
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
