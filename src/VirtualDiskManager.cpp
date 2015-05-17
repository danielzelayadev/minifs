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
    ofstream* newDisk = new ofstream(diskName, ios::binary);

    char buffer[diskSize];

    if(newDisk->is_open())
    {
       newDisk->write(buffer, diskSize);

       int blockCount = diskSize / blockSize;

       if(createSuperblock(newDisk, diskSize, blockSize, blockCount))
       {
           newDisk->close();
       delete newDisk;

       return true;

       }
    }

    return false;
}

bool VirtualDiskManager::createSuperblock(ofstream* newDisk, int diskSize, int blockSize, int blockCount)
{
}

bool VirtualDiskManager::createBitmap(ofstream* newDisk, int blockCount)
{
}

bool VirtualDiskManager::createInodeTable(ofstream* newDisk)
{
}

bool VirtualDiskManager::createInode(ofstream* newDisk)
{
}
