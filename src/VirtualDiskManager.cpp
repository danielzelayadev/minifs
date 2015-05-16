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

bool VirtualDiskManager::createDisk(int diskSize, int blockSize, string diskName, char diskChar)
{
    ofstream newDisk(diskName);

    newDisk.close();

    return true;
}

bool VirtualDiskManager::createSuperblock()
{
}

bool VirtualDiskManager::createBitmap()
{
}

bool VirtualDiskManager::createInodeTable()
{
}

bool VirtualDiskManager::createInode()
{
}
