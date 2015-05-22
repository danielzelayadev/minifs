#ifndef VIRTUALDISKMANAGER_H
#define VIRTUALDISKMANAGER_H

#include <vector>
#include <string>
#include "VirtualDisk.h"

using namespace std;

struct Inode
{
  //Song Meta
        char artist[30];
        char album[30];
        int songTime;

        //File Meta
        int fileSize;

        char ownerId[2];
        char openMode[2];

        int creationTime;
        int modTime;
        int deleteTime;
        int lastAccessTime;

        int blocksInUse;
        int flags;

        int blocks[10];

        int singleIndirectBlock;
        int doubleIndirectBlock;
};

class VirtualDiskManager
{
    public:
        VirtualDiskManager();
        virtual ~VirtualDiskManager();

        bool openVDiskManager();
        bool closeVDiskManager();
        bool isOpen();
        bool createDisk(string diskName, int blockCount, int blockSize, int diskSize, char partitionChar);


    private:

        vector<VirtualDisk*>* vdisks;
        bool open;

        bool createSuperblock(ofstream* newDisk, string diskName, char partitionChar, int diskSize, int blockSize, int blockCount,
                              int dataBlockCount, int inodeSize, int inodeCount, int inodeBlockCount);
        bool createDataBitmap(ofstream* newDisk, int blockCount);
        bool createInodeTable(ofstream* newDisk, int inodeCount);
        bool createInodes(ofstream* newDisk, int inodeCount);

        bool loadVirtualDisks();
        bool unloadVirtualDisks();

        void goToBlock(ofstream* disk, int blockPos, int blockSize);

        int getCurrentBlock(ofstream* disk, int blockSize);

};

#endif // VIRTUALDISKMANAGER_H
