#ifndef VIRTUALDISKMANAGER_H
#define VIRTUALDISKMANAGER_H

#include <vector>
#include <string>
#include "VirtualDisk.h"

using namespace std;

struct SuperBlock
{
  char diskName[30];
  char partitionChar;
  int diskSize;
  int freeSpace;
  int usedSpace;
  int filesOnDisk;
  int blockSize;
  int usedBlocks;
  int freeBlocks;
  int blockCount;
  int dataBlockCount;
  int inodeTableBlockCount;
  int inodeSize;
  int inodeCount;
  int usedInodes;
  int freeInodes;
  int initBlocks;
  int blocksPerSI;
  int blocksPerDI;
  int siPerDI;
  int inodeBlockCount;
};

struct InodeInfo
{
   int iNumber;
   char songName[50];
   bool free;
};

struct Inode
{
  //Song Meta
        char artist[30];
        char album[30];
        int songTime;

        //File Meta
        int iNumber;
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
        bool createDisk(char diskName[30], int blockCount, int blockSize, int diskSize, char partitionChar);

        bool writeToDisk(char* diskName, char* fileName);


    private:

        vector<VirtualDisk*>* vdisks;
        bool open;

        bool createSuperblock(ofstream* newDisk, char diskName[30], char partitionChar, int diskSize, int blockSize, int blockCount,
                              int dataBlockCount, int inodeSize, int inodeCount, int inodeBlockCount);
        bool createDataBitmap(ofstream* newDisk, int blockCount);
        bool createInodeTable(ofstream* newDisk, int inodeCount);
        bool createInodes(ofstream* newDisk, int inodeCount);

        bool loadVirtualDisks();
        bool unloadVirtualDisks();

        void goToBlock(ofstream* disk, int blockPos, int blockSize);
        void goToDataBlock(fstream* disk, int blockPos, SuperBlock sb);

        void goToIndirectBlock(fstream* disk, SuperBlock sb, int ds, int i);

        int getCurrentBlock(ofstream* disk, int blockSize);

        bool writeToBlock(fstream* disk, char* data);

        int alloc_directBlock(fstream* disk, SuperBlock sb);
        int alloc_singleIndBlock(fstream* disk, SuperBlock sb);
        int alloc_doubleIndBlock(fstream* disk, SuperBlock sb);

        void alloc_blocks(fstream*disk, SuperBlock sb, Inode inode, int blocksNeeded);

        char toggleBit(char c, int bitPos);
        bool bitIsOn(char c, int bitPos);

        bool fileExists(fstream* disk, char* fileName, SuperBlock sb);

};

#endif // VIRTUALDISKMANAGER_H
