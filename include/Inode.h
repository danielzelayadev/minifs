#ifndef INODE_H
#define INODE_H

struct _Inode
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
//        int directsInUse;
//        int sIndirectsInUse;
//        int dIndirectsInUse;
        int flags;

        int blocks[10];

        int singleIndirectBlock;
        int doubleIndirectBlock;
};

class Inode
{
    public:
        Inode(_Inode inode);
        virtual ~Inode();

        _Inode* getStruct();

        void printDirectBlocks();

        void printInode();

        private:
            _Inode* inode;

};

#endif // INODE_H
