#ifndef VIRTUALDISK_H
#define VIRTUALDISK_H

#include <fstream>

#include "SuperBlock.h"
#include "Bitmap.h"
#include "InodeTable.h"
#include "Inode.h"

using namespace std;

enum SI_Modes
{
  FI, FDI
};

class VirtualDisk
{
    public:
        VirtualDisk(char diskName[30], char partitionChar, int blockSize, int blockCount);
        VirtualDisk(char diskName[30]);
        virtual ~VirtualDisk();

        void writeFile(char* fileName);
        void deleteFile(char* fileName);
        void exportFile(char* fileName, char* destination);

        void printSuperBlock();
        void printBitmap();
        void printInodeTable();
        void printInodeTableOccupied();
        void printInode(int iNumber);

    private:
        SuperBlock* superBlock;
        Bitmap* bitmap;
        InodeTable* inodeTable;

        Inode* loadedInode;

        fstream* disk;

        void initialize(char diskName[30], char partitionChar, int blockSize, int blockCount);

        void createSuperBlock(char diskName[30], char partitionChar, int diskSize, int blockSize, int blockCount);
        void createBitmap(int blockCount);
        void createInodeTable(int inodeCount);
        void createInodes(int inodeCount);

        void loadSuperBlock();
        void loadBitmap();
        void loadInodeTable();

        void flushSuperBlock();
        void flushBitmap();
        void flushInodeTable();
        void flushInode(_Inode inode);

        bool checkEnoughSpace(int fileSize, int* blocksNeeded);
        bool isRepeated(char* fileName);
        int getBlocksNeeded(int fileSize);
        Inode* loadInode(int iNumber);
        void alloc_blocks(_Inode* inode);
        int alloc_block();

        void alloc_sIndirect(_Inode* inode, int *blocksNeeded, int mode);
        void alloc_dIndirect(_Inode* inode, int *blocksNeeded);
        void writePointersToBlock(int* pointers, int blockPos);

        void write(ifstream* file, _Inode* inode);
        void writeToDirectBlocks(ifstream* file, int* bn, int* blocks, int blockCount);
        void writeToSIBlock(ifstream* file, _Inode* inode, int* bn, int indDir);
        void writeToDIBlock(ifstream* file, _Inode* inode, int* bn);

        char* loadFile(char* fileName);
        void loadBlockData(char* fileData, char* blockData, int blockIndex);
        int* getSIndirectPointers(int siBlockDir);
        int* getDIndirectPointers(int diBlockDir);
        void calculateBlocksNeededByCat(int blocksNeeded, int* initCount, int* siCount, int* diCount, int* lastSIBlockCount);

        void loadSI(_SuperBlock sb, _Inode inode, int siDir, char* fileData, int* blockIndex, int* bn);

        int getFileSize(char* fileName);

};

#endif // VIRTUALDISK_H
