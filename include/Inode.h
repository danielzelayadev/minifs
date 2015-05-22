#ifndef INODE_H
#define INODE_H

class Inode
{
    public:
        Inode();
        virtual ~Inode();

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

#endif // INODE_H
