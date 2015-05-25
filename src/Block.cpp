#include "Block.h"

Block::Block(int blockSize)
{
    data = new char[blockSize];
}

Block::~Block()
{
    delete data;
}
