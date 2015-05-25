#ifndef BLOCK_H
#define BLOCK_H

class Block
{
    public:
        Block(int blockSize);
        virtual ~Block();

        char* data;
};

#endif // BLOCK_H
