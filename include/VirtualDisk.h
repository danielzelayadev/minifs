#ifndef VIRTUALDISK_H
#define VIRTUALDISK_H

#include "SuperBlock.h"
#include "Bitmap.h"
#include "InodeTable.h"
#include "Inode.h"

class VirtualDisk
{
    public:
        VirtualDisk(SuperBlock* sb, Bitmap* bitmap, InodeTable* iTable, Inode* inodes);
        virtual ~VirtualDisk();

    private:
        SuperBlock* superBlock;
        Bitmap* bitmap;
        InodeTable* inodeTable;
        Inode* inodes;
};

#endif // VIRTUALDISK_H
