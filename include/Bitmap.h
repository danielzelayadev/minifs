#ifndef BITMAP_H
#define BITMAP_H


class Bitmap
{
    public:
        Bitmap(int totalBits);
        Bitmap(char* bitmap, int totalBits);
        virtual ~Bitmap();

        void toggleBit(int bitPos);
        void turnOnBit(int bitPos);
        void turnOffBit(int bitPos);
        bool bitIsOn(int bitPos);
        void turnOffAll();
        void turnOnAll();

        int getBitCount();

    private:
        char* bitmap;

        int bitCount;

        int calculateByteCount(int totalBits);

        int getBytePos(int bitPos);
};

#endif // BITMAP_H
