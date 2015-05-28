#ifndef INODETABLE_H
#define INODETABLE_H

struct InodeInfo
{
   int iNumber;
   char songName[50];
   bool free;
};

class InodeTable
{
    public:
        InodeTable(int inodeCount);
        InodeTable(InodeInfo* inodeTable, int tableSize);
        virtual ~InodeTable();

        InodeInfo* getInodeInfo(char* fileName);
        InodeInfo* getInodeInfo(int iNumber);
        InodeInfo* getFreeInodeInfo();

        int getTableSize();

        void printTable();
        void printOccupied();
        void printInodeInfo(int iNumber);
        void printInodeInfo(char* fileName);
        void printInodeInfo(InodeInfo* iInfo);

    private:
       InodeInfo* inodeTable;

       int tableSize;
};

#endif // INODETABLE_H
