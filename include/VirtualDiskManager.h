#ifndef VIRTUALDISKMANAGER_H
#define VIRTUALDISKMANAGER_H

#include <vector>
#include <string>
#include "VirtualDisk.h"
#include "Inode.h"

#define INODE_SIZE 106

using namespace std;

class VirtualDiskManager
{
    public:
        VirtualDiskManager();
        virtual ~VirtualDiskManager();

        bool openVDiskManager();
        bool closeVDiskManager();
        bool isOpen();
        bool createDisk(int diskSize, int blockSize, int blockCount, string diskName, char partitionChar);


    private:

        vector<VirtualDisk*>* vdisks;
        bool open;

        bool createSuperblock(ofstream* newDisk, string diskName, int diskSize, int blockSize,
                              int blockCount, int inodeBlockCount);
        bool createDataBitmap(ofstream* newDisk, int blockCount);
        bool createInodeTable(ofstream* newDisk, int inodeCount);
        bool createInode(ofstream* newDisk);

        bool loadVirtualDisks();
        bool unloadVirtualDisks();

        void goToBlock(ofstream* disk, int blockPos, int blockSize);

};

#endif // VIRTUALDISKMANAGER_H
