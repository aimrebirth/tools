#pragma once

#define LOBYTE(x) (*((uint8_t *)&(x)))

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
                    v10 = &v6[-c_2];
                    c_3 = c;
                    do {
                        LOBYTE(c) = *v10;
                        *v6++ = *v10++;
                        --c_3;
                    } while (c_3);
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

template <typename T>
auto decode_rle(const T *input, int size, T *output) {
    if (size < 2) {
        return (uint8_t *)output;
    }
    const auto base = input;
    const auto rle_indicator = (uint8_t)*input++;
    while (1) {
        auto c = *input++;
        //msvc bad warn. workaround vvvvvvvv
        if ((sizeof(T) == 1 ? c : ((uint16_t)c >> 8)) != rle_indicator) {
            *output++ = c;
        } else {
            uint32_t count = sizeof(T) == 1 ? *input++ : (c & 0xFF);
            if (count == 0xFF) {
                *output++ = sizeof(T) == 1 ? rle_indicator : *input++;
            } else {
                for (int i = 0; i < count + 3; i++) {
                    *output++ = *input;
                }
                ++input;
            }
        }
        if ((uint8_t *)input >= (uint8_t *)base + size) {
            return (uint8_t *)output;
        }
    }
}
