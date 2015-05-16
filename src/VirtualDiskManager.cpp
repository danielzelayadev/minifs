#include "VirtualDiskManager.h"
#include <fstream>

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

bool VirtualDiskManager::createDisk(int diskSize, int blockSize, int inodeAmount, string diskName, char diskChar)
{
    if(createSuperblock(diskSize, blockSize, inodeAmount))
    {
       int blockCount = diskSize / blockSize;

       if(createBitmap(blockCount))
       {

          if(createInodeTable())
          {
              for(int i = 0; i < inodeAmount; i++)
                 createInode();

              return true;
          }

       }

    }

    return false;
}

bool VirtualDiskManager::createSuperblock(int diskSize, int blockSize, int inodeAmount)
{
}

bool VirtualDiskManager::createBitmap(int blockCount)
{
}

bool VirtualDiskManager::createInodeTable()
{
}

bool VirtualDiskManager::createInode()
{
}
