#include "VirtualDiskManager.h"

VirtualDiskManager::VirtualDiskManager()
{
    this->vdisks = new vector<VirtualDisk*>();
}

VirtualDiskManager::~VirtualDiskManager()
{
   delete vdisks;
}

bool VirtualDiskManager::createVirtualDisk(char diskName[30], char partitionChar, int blockSize, int blockCount)
{
   //POner validaciones y etc...
}
