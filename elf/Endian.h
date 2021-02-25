#ifndef ENDIAN_H
#define ENDIAN_H

class EndianConverter
{
  public:
	static void Little(unsigned char &x);
	static void Little(unsigned short &x);
	static void Little(unsigned int &x);
	static void Little(unsigned long int &x);
	static void Little(int &x);
	static void Little(long int &x);
};

#if defined(ENDIAN_CPP) || defined(INLINE_ON)

#ifndef INLINE
#define INLINE
#endif

INLINE void
EndianConverter::Little(unsigned char &x)
{
}

INLINE void
EndianConverter::Little(unsigned short &x)
{
    unsigned short res = ((unsigned char*)&x)[1] + (((unsigned char*)&x)[0] << 8);
    x = res;
}

INLINE void
EndianConverter::Little(unsigned int &x)
{
    unsigned int res = ((unsigned char*)&x)[3] + (((unsigned char*)&x)[2] << 8) + (((unsigned char*)&x)[1] << 16) + (((unsigned char*)&x)[0] << 24);
    x = res;
}

INLINE void
EndianConverter::Little(unsigned long int &x)
{
    unsigned long int res = ((unsigned char*)&x)[3] + (((unsigned char*)&x)[2] << 8) + (((unsigned char*)&x)[1] << 16) + (((unsigned char*)&x)[0] << 24);
    x = res;
}

INLINE void
EndianConverter::Little(int &x)
{
    int res = ((unsigned char*)&x)[3] + (((unsigned char*)&x)[2] << 8) + (((unsigned char*)&x)[1] << 16) + (((unsigned char*)&x)[0] << 24);
    x = res;
}

INLINE void
EndianConverter::Little(long int &x)
{
    long int res = ((unsigned char*)&x)[3] + (((unsigned char*)&x)[2] << 8) + (((unsigned char*)&x)[1] << 16) + (((unsigned char*)&x)[0] << 24);
    x = res;
}

#endif // ENDIAN_CPP || INLINE_ON

#endif // ENDIAN_H
