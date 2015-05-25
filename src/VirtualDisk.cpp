#include "VirtualDisk.h"

VirtualDisk::VirtualDisk(SuperBlock* sb, Bitmap* bitmap, InodeTable* iTable, Inode* inodes)
{
    this->superBlock = sb;
    this->bitmap = bitmap;
    this->inodeTable = iTable;
    this->inodes = inodes;
}

VirtualDisk::~VirtualDisk()
{
    delete superBlock;
    delete bitmap;
    delete inodeTable;
    delete inodes;
}
