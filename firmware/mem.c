#include <stddef.h>
#include <stdint.h>

// Those implementations are pretty bad!

void* memcpy(void* dest, const void* src, size_t cnt)
{
    uint8_t* destb = (uint8_t*)dest;
    const uint8_t* srcb = (const uint8_t*)src;
    for(size_t i = 0; i < cnt; i++)
    {
        destb[i] = srcb[i];
    }
    return dest;
}

void* memset(void* dest, int ch, size_t cnt)
{
    uint8_t* destb = (uint8_t*)dest;
    uint8_t b = (uint8_t)ch;
    for(size_t i = 0; i < cnt; i++)
    {
        destb[i] = b;
    }
}

void* memmove(void* dest, const void* src, size_t cnt)
{
    uint8_t* destb = (uint8_t*)dest;
    const uint8_t* srcb = (const uint8_t*)src;
    if(src < dest)
    {
        // Copy from right-to-left
        for(size_t i = cnt; i > 0; i--)
        {
            destb[i - 1] = srcb[i - 1];
        }
    } 
    else if(dest < src)
    {
        // Copy from left-to-right
        for(size_t i = 0; i < cnt; i++)
        {
            destb[i] = srcb[i];
        }
    }
    // Otherwise do nothing

    return dest;
}

size_t strlen(const char* str)
{
    size_t i = 0;
    while(str[i] != 0)
    {
        i++;
    }
    return i;
}