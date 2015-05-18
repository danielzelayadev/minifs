#ifndef VIRTUALDISKMANAGER_H
#define VIRTUALDISKMANAGER_H

#include <vector>
#include <string>
#include "VirtualDisk.h"

using namespace std;

class VirtualDiskManager
{
    public:
        VirtualDiskManager();
        virtual ~VirtualDiskManager();

        bool openVDiskManager();
        bool closeVDiskManager();
        bool isOpen();
        bool createDisk(int diskSize, int blockSize, int blockCount, string diskName, char diskChar);


    private:

        vector<VirtualDisk*>* vdisks;
        bool open;

        bool createSuperblock(ofstream* newDisk, string diskName, int diskSize, int blockSize,
                              int blockCount, int inodeBlockCount);
        bool createDataBitmap(ofstream* newDisk, int blockCount);
        bool createInodeBitmap(ofstream* newDisk, int inodeBlockCount);
        bool createInodeTable(ofstream* newDisk, int inodeBlockCount);
        bool createInode(ofstream* newDisk);

        bool loadVirtualDisks();
        bool unloadVirtualDisks();

        void goToBlock(ofstream* disk, int blockPos, int blockSize);

};

#endif // VIRTUALDISKMANAGER_H
