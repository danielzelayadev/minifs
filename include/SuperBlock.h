#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

struct _SuperBlock
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
  int freeDataBlocks;
  int usedDataBlocks;
  int freeDataSpace;
  int usedDataSpace;
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

class SuperBlock
{
    public:
        SuperBlock(_SuperBlock sb);
        virtual ~SuperBlock();

        _SuperBlock* getStruct();

        void printSuperBlock();

    private:
        _SuperBlock* sb;
};

#endif // SUPERBLOCK_H
