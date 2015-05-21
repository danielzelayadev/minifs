#ifndef INODE_H
#define INODE_H

#include "Duration.h"

class Inode
{
    public:
        Inode();
        virtual ~Inode();

        //Song Meta
        char songName[30];
        char artist[30];
        char album[30];
        Duration* duration;

        //File Meta
        int fileSize;


};

#endif // INODE_H
