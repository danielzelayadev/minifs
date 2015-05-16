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
        bool createDisk(int diskSize, int blockSize, int inodeAmount, string diskName, char diskChar);


    private:

        vector<VirtualDisk*>* vdisks;
        bool open;

        bool createSuperblock(int diskSize, int blockSize, int inodeAmount);
        bool createBitmap(int blockCount);
        bool createInodeTable();
        bool createInode();

        bool loadVirtualDisks();
        bool unloadVirtualDisks();

};

#endif // VIRTUALDISKMANAGER_H
