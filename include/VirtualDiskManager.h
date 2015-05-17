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

        bool createSuperblock(ofstream* newDisk, int diskSize, int blockSize, int blockCount);
        bool createBitmap(ofstream* newDisk, int blockCount);
        bool createInodeTable(ofstream* newDisk);
        bool createInode(ofstream* newDisk);

        bool loadVirtualDisks();
        bool unloadVirtualDisks();

};

#endif // VIRTUALDISKMANAGER_H
