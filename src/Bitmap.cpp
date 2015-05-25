#include "Bitmap.h"

#include <cmath>

Bitmap::Bitmap(int totalBits)
{
   bitCount = totalBits;
   bitmap = new char[calculateByteCount(totalBits)];
   turnOffAll();
}

Bitmap::Bitmap(char* bitmap, int totalBits)
{
   bitCount = totalBits;
   this->bitmap = bitmap;
}

Bitmap::~Bitmap()
{
    delete bitmap;
}

bool Bitmap::bitIsOn(int bitPos)
{
   int bytePos = getBytePos(bitPos);
   char c = bitmap[bytePos];
   bitPos = std::abs((bytePos*8) - bitPos);
   return (c  & (1 << bitPos)) != 0;
}

void Bitmap::toggleBit(int bitPos)
{
   int bytePos = getBytePos(bitPos);
   bitPos = std::abs((bytePos*8) - bitPos);
   bitmap[bytePos] ^= (1<< bitPos);
}

void Bitmap::turnOnBit(int bitPos)
{
   int bytePos = getBytePos(bitPos);
   bitPos = std::abs((bytePos*8) - bitPos);
   bitmap[bytePos] |= (1 << bitPos);
}

void Bitmap::turnOffBit(int bitPos)
{
   int bytePos = getBytePos(bitPos);
   bitPos = std::abs((bytePos*8) - bitPos);
   bitmap[bytePos] &= ~(1<<bitPos);
}

void Bitmap::turnOnAll()
{
   for(int i = 0; i < bitCount; i++)
     turnOnBit(i);
}

void Bitmap::turnOffAll()
{
   for(int i = 0; i < bitCount; i++)
      turnOffBit(i);
}

int Bitmap::calculateByteCount(int totalBits)
{
    int byteCount = totalBits / 8;

    if(totalBits % 8 != 0) byteCount++;

    return byteCount;
}

int Bitmap::getBitCount()
{
   return bitCount;
}

int Bitmap::getBytePos(int bitPos)
{
   return bitPos / 8;
}
