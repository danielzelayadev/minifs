#ifndef VIRTUALDISKMANAGER_H
#define VIRTUALDISKMANAGER_H

#include <vector>
#include <string>
#include "VirtualDisk.h"

using namespace std;

class VirtualDiskManager
{
    public:
        VirtualDiskManager();
        virtual ~VirtualDiskManager();

        bool createVirtualDisk(char diskName[30], char partitionChar, int blockSize, int blockCount); //Add
        bool deleteVirtualDisk(char* diskName); //Remove
        VirtualDisk* getVirtualDisk(char* diskName); //Get

    private:

        vector<VirtualDisk*>* vdisks;

};

#endif // VIRTUALDISKMANAGER_H
