#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <vector>
#include <string>

using namespace std;

enum Errors
{
   COULD_NOT_CREATE_SUPERBLOCK,
   COULD_NOT_CREATE_BITMAP,
   COULD_NOT_CREATE_INODETABLE,
   COULD_NOT_CREATE_INODE,
   COULD_NOT_CREATE_WRITE_FILE,
   COULD_NOT_CREATE_READ_FILE,
   COULD_NOT_CREATE_OPEN_FILE,
   ERROR_COUNT
};

class ErrorHandler
{
    public:
        static string getError(int key);

    private:
        int keys[ERROR_COUNT] = {COULD_NOT_CREATE_SUPERBLOCK, COULD_NOT_CREATE_BITMAP, COULD_NOT_CREATE_INODETABLE,
                                 COULD_NOT_CREATE_INODE, COULD_NOT_CREATE_WRITE_FILE, COULD_NOT_CREATE_READ_FILE,
                                 COULD_NOT_CREATE_OPEN_FILE};
        static string values[ERROR_COUNT] = {"\nERROR: Could not create Super Block!\n", "\nERROR: Could not create Bitmap!\n",
                                      "\nERROR: Could not create Inode Table!\n", "\nERROR: Could not create Inode!\n"
                                      "\nERROR: Could not write file!\n", "\nERROR: Could not read file!\n",
                                      "\nERROR: Could not create open file!\n"};
};

#endif // ERRORHANDLER_H
