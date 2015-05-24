#include "VirtualDiskManager.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <ctime>

#define INODE_BLOCK_PERCENTAGE 0.07

VirtualDiskManager::VirtualDiskManager()
{
    this->vdisks = nullptr;
    this->open = false;
}

VirtualDiskManager::~VirtualDiskManager()
{
   delete vdisks;
}

bool VirtualDiskManager::openVDiskManager()
{
    open = true;
}

bool VirtualDiskManager::closeVDiskManager()
{
    open = false;
}

bool VirtualDiskManager::isOpen()
{
   return open;
}

bool VirtualDiskManager::loadVirtualDisks()
{


}

bool VirtualDiskManager::unloadVirtualDisks()
{
}

bool VirtualDiskManager::createDisk(char diskName[30], int blockCount, int blockSize, int diskSize, char partitionChar)
{
    ofstream* newDisk = new ofstream(diskName, ios::binary);

    char* buffer = new char[diskSize];

    if(newDisk->is_open())
    {
       newDisk->write(buffer, diskSize);

       delete buffer;

       int inodeSize = sizeof(Inode), inodeBlockCount = (( diskSize - (3*blockSize) ) * INODE_BLOCK_PERCENTAGE) / blockSize,
           inodeCount = (inodeBlockCount*blockSize) / inodeSize,
           dataBlockCount = blockCount-(3+inodeBlockCount);

       if(createSuperblock(newDisk, diskName, partitionChar, diskSize, blockSize, blockCount, dataBlockCount,
          inodeSize, inodeCount, inodeBlockCount))
       {
            goToBlock(newDisk, 1, blockSize);

            if(createDataBitmap(newDisk, dataBlockCount))
            {
               goToBlock(newDisk, 2, blockSize);

               if(createInodeTable(newDisk, inodeCount))
               {
                    goToBlock(newDisk, getCurrentBlock(newDisk, blockSize)+1, blockSize);

                    if(createInodes(newDisk, inodeCount))
                    {
                       newDisk->close();
                       delete newDisk;
                       return true;

                    }
               }
            }
       }
    }

    return false;
}

bool VirtualDiskManager::createSuperblock(ofstream* newDisk, char diskName[30], char partitionChar, int diskSize, int blockSize,
                                        int blockCount, int dataBlockCount, int inodeSize, int inodeCount, int inodeBlockCount)
{

      newDisk->seekp(0);

      SuperBlock sp;
      strcpy(sp.diskName, diskName);
      sp.blockCount = blockCount;
      sp.blockSize = blockSize;
      sp.filesOnDisk = 0;
      sp.dataBlockCount = dataBlockCount;
      sp.diskSize = diskSize;
      sp.inodeTableBlockCount = (sizeof(InodeInfo)*inodeCount % blockSize != 0 ?
      sizeof(InodeInfo)*inodeCount / blockSize + 1 : sizeof(InodeInfo)*inodeCount / blockSize);
      sp.usedSpace = (blockSize*2) + (sp.inodeTableBlockCount*blockSize) + (inodeBlockCount*blockSize); //Revisar despues
      sp.freeSpace = diskSize - (blockSize*2) - sp.usedSpace;
      sp.usedBlocks = 2 + sp.inodeTableBlockCount + inodeBlockCount; //Revisar
      sp.freeBlocks = blockCount - sp.usedBlocks;
      sp.initBlocks = 10;
      sp.blocksPerSI = blockSize / 4;
      sp.siPerDI = blockSize / 4;
      sp.blocksPerDI = sp.blocksPerSI * sp.siPerDI;
      sp.inodeBlockCount = inodeBlockCount;
      sp.usedInodes = 0;
      sp.freeInodes = inodeCount;
      sp.inodeCount = inodeCount;
      sp.inodeSize = inodeSize;
      sp.partitionChar = partitionChar;

      newDisk->write((char*)&sp, sizeof(SuperBlock));

      return true;

}

bool VirtualDiskManager::createDataBitmap(ofstream* newDisk, int blockCount)
{
     int byteCount = blockCount / 8;

     char bitmap[byteCount];

     for(int i = 0; i < byteCount; i++)
     {
         bitmap[i] = 0;
         newDisk->write(&bitmap[i], sizeof(char));
     }

     return true;
}

bool VirtualDiskManager::createInodeTable(ofstream* newDisk, int inodeCount)
{

  for(int i = 0; i < inodeCount; i++)
  {
     InodeInfo iInfo;

     iInfo.iNumber = i;
     iInfo.free = true;

     newDisk->write((char*)&iInfo, sizeof(InodeInfo));
  }

  return true;
}

bool VirtualDiskManager::createInodes(ofstream* newDisk, int inodeCount)
{

  for(int i = 0; i < inodeCount; i++)
  {
    Inode inode;
    inode.iNumber = i;
    inode.openMode[0] = 'r';
    newDisk->write((char*)&inode, sizeof(Inode));
  }
  return true;
}

void VirtualDiskManager::goToBlock(ofstream* disk, int blockPos, int blockSize)
{
    disk->seekp(blockSize*blockPos);
}

int VirtualDiskManager::getCurrentBlock(ofstream* disk, int blockSize)
{
    int blockPos = -1;

    int currentPos = disk->tellp();

    blockPos = currentPos / blockSize;

    return blockPos;
}

bool VirtualDiskManager::writeToDisk(char* diskName, char* fileName)
{
    ifstream* file = new ifstream(fileName, ios::binary | ios::ate);

    if(file->is_open())
    {
       fstream* disk = new fstream(diskName, ios::binary | ios::ate | ios::in | ios::out);

       if(disk->is_open())
       {
        disk->seekg(0);

        SuperBlock sb;

        disk->read((char*)&sb, sizeof(SuperBlock));

        int fileSize = file->tellg();

        if(sb.freeSpace < fileSize || fileExists(disk, fileName, sb))
        {cout << "File exists\n";
          disk->close();
          file->close();
          delete file;
          delete disk;
          return false;
        }

        goToBlock((ofstream*)disk, 2, sb.blockSize);

        disk->seekg(disk->tellp());

        InodeInfo inodeTable[sb.inodeCount];

        disk->read((char*)&inodeTable, sizeof(inodeTable));

        int freeInodeIndex = -1;

        for(int i = 0; i < sb.inodeCount; i++)
        {
           if(inodeTable[i].free)
           {
              inodeTable[i].free = false;
              strcpy(inodeTable[i].songName, fileName);
              freeInodeIndex = inodeTable[i].iNumber;
              disk->seekp( (sb.blockSize*2) + (sizeof(InodeInfo)*i));
              disk->write((char*)&inodeTable[i], sizeof(InodeInfo));
              break;
           }
        }

        goToBlock((ofstream*)disk, 2+sb.inodeTableBlockCount, sb.blockSize);

        disk->seekg(disk->tellp()+(freeInodeIndex*sb.inodeSize));

        Inode fileInode;

        disk->read((char*)&fileInode, sizeof(Inode));

        fileInode.fileSize = fileSize;
        fileInode.creationTime = time(NULL);

        //Conseguir bloques libres necesarios

        int blocksNeeded = fileSize / sb.blockSize;

        if(fileSize % sb.blockSize != 0) blocksNeeded++;

        fileInode.blocksInUse = blocksNeeded;

        goToBlock((ofstream*)disk, 1, sb.blockSize);

        alloc_blocks(disk, sb, fileInode, blocksNeeded);

        file->seekg(0);

        int ctr = 0;

        for(int i = 0; i < blocksNeeded; i++)
        {
           char* data = new char[sb.blockSize];
           file->read(data, sb.blockSize);
           //cout << data << endl;

           if(i < 10)
              goToBlock((ofstream*)disk, fileInode.blocks[i], sb.blockSize);

           if(i >= 10 && i < sb.blocksPerSI)
              goToIndirectBlock(disk, sb, fileInode.singleIndirectBlock, i);

           if(i >= sb.blocksPerSI && i < sb.blocksPerDI)
           {
               if(i % sb.blocksPerSI == 0)
               goToIndirectBlock(disk, sb, fileInode.doubleIndirectBlock, i); //Avanzar de indirecto
               goToIndirectBlock(disk, sb, fileInode.singleIndirectBlock, ctr); //Avanzar de directo
               ctr++;
           }

           disk->write(data, sb.blockSize);

           delete data;
        }

        //Modificar metadata del disco
        sb.freeBlocks -= blocksNeeded;
        sb.usedSpace += blocksNeeded*sb.blockSize;
        sb.freeSpace -= sb.usedSpace;
        sb.usedBlocks += blocksNeeded;
        sb.filesOnDisk++;
        sb.usedInodes++;
        sb.freeInodes--;

        goToBlock((ofstream*)disk, 0, sb.blockSize);

        disk->write((char*)&sb, sizeof(SuperBlock));

        goToBlock((ofstream*)disk, 0, sb.blockSize);

//        SuperBlock sb2;
//
//        disk->read((char*)&sb2, sizeof(SuperBlock));

//        cout << sb2.usedInodes << endl;
//        cout << sb.usedInodes << endl;

        disk->close();
        file->close();
        delete file;
        delete disk;
        return true;
       }
       delete disk;
    }

    delete file;

    return false;
}

int VirtualDiskManager::alloc_directBlock(fstream* disk, SuperBlock sb)
{
    int freeBlockIndex = -1;
    int bitIndex = -1;

    int byteCount = sb.blockCount / 8;

    if(sb.blockCount % 8 != 0) byteCount++;

    for(int i = 0; freeBlockIndex == -1 && i < byteCount; i++)
    {
       char c = 0;
       disk->read(&c, sizeof(char));

       for(int k = 0; k < 8; k++)
       {
          if(!bitIsOn(c, k))
          {
             freeBlockIndex = (i*8)+k;
             bitIndex = k;
             c = toggleBit(c, k);
             cout << i << "  " << i*8 << "  " << k << "  " << freeBlockIndex << endl;
             disk->seekp(disk->tellp()-1);

             disk->write(&c, sizeof(char));

             break;
          }
       }

    }

    return freeBlockIndex;

}

int VirtualDiskManager::alloc_singleIndBlock(fstream* disk, SuperBlock sb)
{
   return alloc_directBlock(disk, sb);
}

int VirtualDiskManager::alloc_doubleIndBlock(fstream* disk, SuperBlock sb)
{
   return alloc_directBlock(disk, sb);
}

bool VirtualDiskManager::writeToBlock(fstream* disk, char* data)
{
   return false;
}

bool VirtualDiskManager::bitIsOn(char c, int bitPos)
{
   return (c & (1 << bitPos)) != 0;
}

char VirtualDiskManager::toggleBit(char c, int bitPos)
{
   return c ^ (1 << bitPos);
}

void VirtualDiskManager::alloc_blocks(fstream*disk, SuperBlock sb, Inode inode, int blocksNeeded)
{
       int bn = blocksNeeded;

       for(int i = 0; bn != 0 && i < sb.initBlocks; i++, bn--)
          {inode.blocks[i] = alloc_directBlock(disk, sb); }

       if(bn != 0)
       {
          inode.singleIndirectBlock = alloc_singleIndBlock(disk, sb);

          int siPointers[sb.blocksPerSI]; //Se escribe en el sI

          goToBlock((ofstream*)disk, inode.singleIndirectBlock, sb.blockSize);

          for(int i = 0; bn != 0 && i < sb.blocksPerSI; i++, bn--)
          {
              siPointers[i] = alloc_directBlock(disk, sb);
              disk->write((char*)&siPointers[i], sizeof(int));
          }

          if(bn != 0)
          {
             inode.doubleIndirectBlock = alloc_doubleIndBlock(disk, sb);

             int sibsNeeded = bn / sb.blocksPerSI;

             int remBlocksNeeded = bn - (sibsNeeded * sb.blocksPerSI);

             if(remBlocksNeeded != 0) sibsNeeded++;

             int diPointers[sb.siPerDI]; //Se escribe en el dI
             int sPts[sibsNeeded][sb.blocksPerSI];  //Se escribe en cada sI que apunta el dI

             for(int i = 0; i < sibsNeeded; i++)
             {
                 int lim = i+1 == sibsNeeded && remBlocksNeeded != 0 ? remBlocksNeeded : sb.blocksPerSI;

                 diPointers[i] = alloc_singleIndBlock(disk, sb);
                 disk->write((char*)&diPointers[i], sizeof(int));

                 goToBlock((ofstream*)disk, diPointers[i], sb.blockSize);

                 for(int k = 0; k < lim; k++)
                 {
                    sPts[i][k] = alloc_directBlock(disk, sb);
                    disk->write((char*)&sPts[i][k], sizeof(int));
                 }

                 goToBlock((ofstream*)disk, inode.doubleIndirectBlock, sb.blockSize);
                 disk->seekp(disk->tellp()+(i*4));
             }
          }

        }

        //Escribir inodo
        goToBlock((ofstream*)disk, 2+sb.inodeTableBlockCount, sb.blockSize);
        disk->seekp(disk->tellp()+(inode.iNumber*sizeof(Inode)));

        disk->write((char*)&inode, sizeof(Inode));
}

void VirtualDiskManager::goToIndirectBlock(fstream* disk, SuperBlock sb, int ds, int i)
{
    goToBlock((ofstream*)disk, ds, sb.blockSize);
              disk->seekp((i - 10) * 4, ios::cur);
              disk->seekg(disk->tellp());
              int block = -1;
              disk->read((char*)&block, sizeof(int));
              goToBlock((ofstream*)disk, block, sb.blockSize);
}

bool VirtualDiskManager::fileExists(fstream* disk, char* fileName, SuperBlock sb)
{
    goToBlock((ofstream*)disk, 2, sb.blockSize);

    for(int i = 0; i < sb.inodeCount; i++)
    {
       InodeInfo iInfo;
       disk->read((char*)&iInfo, sizeof(InodeInfo));

       if(strcmp(iInfo.songName, fileName) ==  0)
         return true;

    }

     return false;
}
