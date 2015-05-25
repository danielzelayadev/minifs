#include "SuperBlock.h"

#include <iostream>

using namespace std;

SuperBlock::SuperBlock(_SuperBlock sb)
{
   this->sb = new _SuperBlock(sb);
}

SuperBlock::~SuperBlock()
{
    delete sb;
}

_SuperBlock* SuperBlock::getStruct()
{
    return sb;
}

void SuperBlock::printSuperBlock()
{
       cout << "Disk Name: " << sb->diskName<< endl;
       cout << "Partition Letter: "<< sb->partitionChar << endl;
       cout << "Disk Size: "<< sb->diskSize << " bytes" << endl;
       cout << "Free Space: "<< sb->freeSpace << " bytes" << endl;
       cout << "Used Space: "<< sb->usedSpace << " bytes" << endl;
       cout << "Files on Disk: "<< sb->filesOnDisk << endl;
       cout << "Block Size: "<< sb->blockSize << " bytes" << endl;
       cout << "Used Blocks: "<< sb->usedBlocks << endl;
       cout << "Free Blocks: "<< sb->freeBlocks << endl;
       cout << "Block Count: "<< sb->blockCount << endl;
       cout << "Data Block Count: "<< sb->dataBlockCount << endl;
       cout << "Free Data Blocks: "<< sb->freeDataBlocks << endl;
       cout << "Used Data Blocks: "<< sb->usedDataBlocks << endl;
       cout << "Free Data Space: "<< sb->freeDataSpace << " bytes" <<endl;
       cout << "Used Data Space: "<< sb->usedDataSpace << " bytes" <<endl;
       cout << "Inode Table Block Count: "<< sb->inodeTableBlockCount << endl;
       cout << "Inode Size: "<< sb->inodeSize << " bytes" << endl;
       cout << "Inode Count: "<< sb->inodeCount << endl;
       cout << "Used Inodes: "<< sb->usedInodes << endl;
       cout << "Free Inodes: "<< sb->freeInodes << endl;
       cout << "Init Direct Blocks: "<< sb->initBlocks << endl;
       cout << "Direct Blocks Per Single Indirect Block: "<< sb->blocksPerSI << endl;
       cout << "Direct Blocks Per Double Indirect Block: "<< sb->blocksPerDI << endl;
       cout << "Single Indirect Blocks per Double Indirect Block: "<< sb->siPerDI << endl;
       cout << "Inode Block Count: "<< sb->inodeBlockCount << endl;

       cout << endl;
}
