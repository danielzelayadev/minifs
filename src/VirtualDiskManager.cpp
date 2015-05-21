#include "VirtualDiskManager.h"
#include <fstream>
#include <iostream>

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

bool VirtualDiskManager::createDisk(int diskSize, int blockSize, int blockCount, string diskName, char partitionChar)
{
    ofstream* newDisk = new ofstream(diskName, ios::binary);

    char buffer[diskSize];

    if(newDisk->is_open())
    {
       newDisk->write(buffer, diskSize);

       int inodeBlockCount = blockCount * 0.2;

       if(createSuperblock(newDisk, diskName, diskSize, blockSize, blockCount, inodeBlockCount))
       {
            goToBlock(newDisk, 1, blockSize);

              if(createDataBitmap(newDisk, blockCount))
              {
                 goToBlock(newDisk, 2, blockSize);

                 int inodeCount = (blockSize*inodeBlockCount) / INODE_SIZE;

                 if(createInodeTable(newDisk, inodeCount))
                 {
                    newDisk->close();
                    delete newDisk;
                    return true;
                 }
              }

       }
    }

    return false;
}

bool VirtualDiskManager::createSuperblock(ofstream* newDisk, string diskName, int diskSize, int blockSize,
                              int blockCount, int inodeBlockCount)
{

      newDisk->seekp(0);

      newDisk->write( diskName.c_str(), diskName.length());
      newDisk->write( (char*)&diskSize, sizeof(int));
      newDisk->write( (char*)&blockSize, sizeof(int));
      newDisk->write( (char*)&blockCount, sizeof(int));
      newDisk->write( (char*)&inodeBlockCount, sizeof(int));

      return true;

}

bool VirtualDiskManager::createDataBitmap(ofstream* newDisk, int blockCount)
{
     int byteCount = blockCount / 8;

     bool bitmap[byteCount];

     for(int i = 0; i < byteCount; i++)
     {
         bitmap[i] = 0;
         newDisk->write( (char*)&bitmap[i], sizeof(bool));
     }

     return true;
}

bool VirtualDiskManager::createInodeTable(ofstream* newDisk, int inodeCount)
{

  return true;
}

bool VirtualDiskManager::createInode(ofstream* newDisk)
{
  return true;
}

void VirtualDiskManager::goToBlock(ofstream* disk, int blockPos, int blockSize)
{
    disk->seekp(blockSize*blockPos);
}
