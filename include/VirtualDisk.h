#ifndef VIRTUALDISK_H
#define VIRTUALDISK_H

#include <fstream>

#include "SuperBlock.h"
#include "Bitmap.h"
#include "InodeTable.h"
#include "Inode.h"

using namespace std;

//Manipular

class VirtualDisk
{
    public:
        VirtualDisk(char diskName[30], char partitionChar, int blockSize, int blockCount);
        VirtualDisk(char diskName[30]);
        virtual ~VirtualDisk();

        void writeFile(char* fileName);
        void readFile(char* fileName);
        void deleteFile(char* fileName);
        void exportFile(char* fileName, char* destination);

        void printSuperBlock();
        void printBitmap();
        void printInodeTable();
        void printInode(int iNumber);

    private:
        SuperBlock* superBlock;
        Bitmap* bitmap;
        InodeTable* inodeTable;

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

};

#endif // VIRTUALDISK_H
