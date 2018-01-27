#pragma once

#define _BYTE uint8_t
#define _WORD uint16_t
#define _DWORD uint32_t

#define LOBYTE(x) (*((_BYTE *)&(x)))
#define LOWORD(x) (*((_WORD *)&(x)))
#define HIBYTE(x) (*((_BYTE *)&(x) + 1))

#define BYTEn(x, n) (*((_BYTE *)&(x) + n))
#define BYTE1(x) BYTEn(x, 1)

template <class T>
int8_t __SETS__(T x)
{
    if (sizeof(T) == 1)
        return int8_t(x) < 0;
    if (sizeof(T) == 2)
        return int16_t(x) < 0;
    if (sizeof(T) == 4)
        return int32_t(x) < 0;
    return int64_t(x) < 0;
}

template <class T, class U>
int8_t __OFSUB__(T x, U y)
{
    if (sizeof(T) < sizeof(U))
    {
        U x2 = x;
        int8_t sx = __SETS__(x2);
        return (sx ^ __SETS__(y)) & (sx ^ __SETS__(x2 - y));
    }
    else
    {
        T y2 = y;
        int8_t sx = __SETS__(x);
        return (sx ^ __SETS__(y2)) & (sx ^ __SETS__(x - y2));
    }
}

inline void memset32(void *ptr, uint32_t value, int count)
{
    uint32_t *p = (uint32_t *)ptr;
    for (int i = 0; i < count; i++)
        *p++ = value;
}

char *decode_f1(char *input, int size, char *output)
{
    char *result; // eax@1
    char *v4;     // ebx@1
    int v5;       // ebp@1
    char *v6;     // edi@2
    char *v7;     // edx@4
    int v8;       // ecx@6
    int v9;       // esi@6
    int v10;      // esi@6
    int v11;      // ecx@6
    char *v12;    // edx@7
    int v13;      // esi@7

    result = input;
    v4 = &input[size];
    v5 = 8;
    if (input < &input[size])
    {
        v6 = output;
        do
        {
            if (v5 == 8)
            {
                v7 = (char *)(uint8_t)*result++;
                v5 = 0;
                input = v7;
                if (result == v4)
                    break;
            }
            if ((uint8_t)input & 1)
            {
                v8 = (uint8_t)*result;
                v9 = (uint8_t)result[1];
                result += 2;
                v10 = ((v8 & 0xF) << 8) + v9;
                v11 = (v8 >> 4) + 4;
                if (v11)
                {
                    v12 = &v6[-v10];
                    v13 = v11;
                    do
                    {
                        *v6++ = *v12++;
                        --v13;
                    } while (v13);
                }
            }
            else
            {
                *v6++ = *result++;
            }
            ++v5;
            input = (char *)((signed int)input >> 1);
        } while (result < v4);
    }
    return result;
}

char decode_f2(char *input, int size, char *output)
{
    int c;     // eax@1
    char v4;   // bl@1
    char *v5;  // edx@1
    char *v6;  // esi@2
    char *v7;  // edx@4
    int c_1;   // edi@4
    int c_2;   // edi@7
    char *v10; // ecx@8
    int c_3;   // edi@8

    LOBYTE(c) = size;
    v4 = *input;
    v5 = input + 1;
    if (input + 1 < &input[size])
    {
        v6 = output;
        do
        {
            LOBYTE(c) = *v5;
            if (*v5 == v4)
            {
                c = (uint8_t)v5[1];
                v7 = v5 + 1;
                c_1 = (uint8_t)v7[1];
                v5 = v7 + 2;
                if (c != 255 || c_1 != 255)
                {
                    c_2 = ((c & 0xF) << 8) + c_1;
                    c = (c >> 4) + 4;
                    if (c)
                    {
                        v10 = &v6[-c_2];
                        c_3 = c;
                        do
                        {
                            LOBYTE(c) = *v10;
                            *v6++ = *v10++;
                            --c_3;
                        } while (c_3);
                    }
                }
                else
                {
                    *v6++ = v4;
                }
            }
            else
            {
                *v6++ = c;
                ++v5;
            }
        } while (v5 < &input[size]);
    }
    return c;
}

int decode_f3(char *input, int size, char *output)
{
    uint16_t s;       // cx@1
    char *v4;         // edi@1
    int result;       // eax@1
    int idx;          // edx@1
    bool v7;          // zf@1
    bool v8;          // sf@1
    uint8_t v9;       // of@1
    int v10;          // ebp@1
    char *v11;        // ebx@2
    uint16_t v12;     // ax@3
    uint8_t v13;      // cl@4
    __int16 v14;      // ax@6
    unsigned int v15; // esi@6
    void *v16;        // edi@7
    int v17;          // ebp@7
    int v18;          // eax@7
    int v19;          // edi@7
    int i;            // ecx@7
    int v21;          // [sp+8h] [bp-4h]@1
    int v22;          // [sp+14h] [bp+8h]@1

    LOBYTE(s) = 0;
    v4 = input;
    HIBYTE(s) = *input;
    result = size >> 1;
    idx = 1;
    v9 = __OFSUB__(size >> 1, 1);
    v7 = size >> 1 == 1;
    v8 = (size >> 1) - 1 < 0;
    v22 = size >> 1;
    v10 = s;
    v21 = s;
    if (!((uint8_t)(v8 ^ v9) | v7))
    {
        v11 = output;
        do
        {
            v12 = *(_WORD *)&v4[2 * idx];
            if ((*(_WORD *)&v4[2 * idx] & 0xFF00) == (_WORD)v10)
            {
                v13 = *(_WORD *)&v4[2 * idx++];
                if (v12 != (uint16_t)v10 + 255)
                {
                    v14 = *(_WORD *)&v4[2 * idx];
                    v15 = v13 + 3;
                    if ((signed int)v15 > 0)
                    {
                        LOWORD(v10) = *(_WORD *)&v4[2 * idx];
                        v16 = v11;
                        v11 += 2 * v15;
                        v17 = v10 << 16;
                        LOWORD(v17) = v14;
                        v18 = v17;
                        v10 = v21;
                        memset32(v16, v18, v15 >> 1);
                        v19 = (int)((char *)v16 + 4 * (v15 >> 1));
                        for (i = (v13 + 3) & 1; i; --i)
                        {
                            *(_WORD *)v19 = v18;
                            v19 += 2;
                        }
                        v4 = input;
                    }
                    goto LABEL_13;
                }
                *(_WORD *)v11 = *(_WORD *)&v4[2 * idx];
            }
            else
            {
                *(_WORD *)v11 = v12;
            }
            v11 += 2;
        LABEL_13:
            result = v22;
            ++idx;
        } while (idx < v22);
    }
    return result;
}

int decode_f4(char *input, int size, char *output, int segment_offset)
{
    char *in3;           // edx@1
    int result;          // eax@1
    char in1;            // bl@1
    int ptr;             // esi@1
    char *out1;          // edi@2
    char c;              // al@3
    char c_next;         // cl@4
    char v11;            // al@6
    unsigned int c_prev; // ebp@6
    int v13;             // eax@7
    char in2;            // [sp+1h] [bp-1h]@1

    in3 = input;
    result = size;
    in1 = *input;
    ptr = 1;
    in2 = *input;
    if (size > 1)
    {
        out1 = output;
        while (1)
        {
            c = in3[ptr];
            if (c != in1)
                break;
            c_next = in3[ptr++ + 1];
            if (c_next == -1)
            {
                *out1 = in1;
            LABEL_9:
                ++out1;
                goto LABEL_10;
            }
            v11 = in3[ptr++ + 1];
            c_prev = (uint8_t)c_next + 3;
            if ((signed int)c_prev > 0)
            {
                LOBYTE(segment_offset) = v11;
                BYTE1(segment_offset) = v11;
                v13 = segment_offset << 16;
                LOWORD(v13) = segment_offset;
                in1 = in2;
                memset32(out1, v13, c_prev >> 2);
                in3 = input;
                memset(&out1[4 * (c_prev >> 2)], v13, c_prev & 3);
                out1 = &output[c_prev];
            LABEL_10:
                output = out1;
            }
            result = size;
            ++ptr;
            if (ptr >= size)
                return result;
        }
        *out1 = c;
        goto LABEL_9;
    }
    return result;
}
