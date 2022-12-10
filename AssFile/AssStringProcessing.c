/* 
Copyright 2019-2021 hkm(github:hihkm)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#include "AssFile.h"
#include "AssStringProcessing.h"

const struct AssEscapeListNode assEscapeList[] = 
{ /* 源字符串 转义后字符串 */
    {"\\n", "\\{}n"},
    {"\\h", "\\{}h"},
    {" ", "\\h"},
    {"\n", "\\N"},
    {NULL, NULL}
};

/* 
 * 转换十进制rgb颜色为十六进制颜色
 * 参数：
 * 十进制颜色/记录十六进制的数组（7个字节以上） 
 * 返回值：
 * 十六进制数组的地址
 */ 
char *toHexColor(int decColor, char *hexColor)
{/* 先转换为16进制再两个为一对倒序排列 */
    int i, j;
    /* 使用两个for实现位置交叉 */ 
    for(i = 0; i < 3; i++)
    {
        for(j = 1; j >= 0; j--)
        {
            if (decColor % 16 < 10)
            {
                hexColor[2 * i + j] = decColor % 16 + '0';
            }
            else
            {
                hexColor[2 * i + j] = decColor % 16 - 10 + 'A';
            }
            decColor /= 16;
        }
    }
    hexColor[6] = '\0'; 
    return hexColor;
}

/* 
 * 十进制透明度转十六进制透明度（0-255） 
 */
char *toHexOpacity(int decOpacity, char *hexOpacity)
{
    int cnt;
    for(cnt = 1; cnt >=0; cnt--)
    {
        if (decOpacity % 16 < 10)
        {
            hexOpacity[cnt] = decOpacity % 16 + '0';
        }
        else
        {
            hexOpacity[cnt] = decOpacity % 16 - 10 + 'A';
        }
        decOpacity /= 16;
    }
    hexOpacity[2] = '\0';
    return hexOpacity;
}

/* 
 * 转换十六进制rgb颜色为十进制颜色
 */ 
int toDecColor(char *hexColor)
{
    int i, j;
    int color = 0;
    
    for (i = 2; i >= 0; i--)
    {
        for (j = 0; j < 2; j++)
        {
            if (hexColor[2*i + j] >= '0' && hexColor[2*i + j] <= '9')
            {
                color *= 16;
                color += hexColor[2*i + j] - '0';
            }
            else if (hexColor[2*i + j] >= 'a' && hexColor[2*i + j] <= 'f')
            {
                color *= 16;
                color += hexColor[2*i + j] - 'a' + 10;
            }
            else if (hexColor[2*i + j] >= 'A' && hexColor[2*i + j] <= 'F')
            {
                color *= 16;
                color += hexColor[2*i + j] - 'A' + 10;
            }
        }
    }
    
    return color;
}

/*  去样式名前缀
 *
 *   0         1         2
 *   012345678901234567890123456789
 *   danmakuFactory_ext_sub00_name     ->     name
 */
char *deStyleNamePrefix(char *str)
{
    /* 合法性检查 避免偏移导致的非法访问 */
    if (strlen(str) < 22)
    {
        return NULL;
    }
    
    char *leftPtr, *rightPtr;
    leftPtr = str;
    rightPtr = str + 21;/* 从22个字节开始寻找字符 '_' 下划线 */
    
    while (*rightPtr != '\0' && *(rightPtr-1) != '_')
    {/* 寻找下划线 */
        rightPtr++;
    }
    
    while (*(rightPtr-1) != '\0')
    {/* 内容拷贝 */
        *leftPtr = *rightPtr;
        leftPtr++;
        rightPtr++;
    }
    
    return str;
}

/* 
以ass标准读取时间
输出单位：秒 
 */
float timeToFloat(const char *ipStr)
{
    /* 0:00:01.60 */
    int num = 0;
    float time = 0.00;
    char *ptr = (char *)ipStr;
    
    /* 小时部分 */
    while (*ptr != ':' && *ptr != '.' && *ptr != '\0')
    {
        if (*ptr == ' ')
        {
            ptr++;
            continue;
        }
        num *= 10;
        num += *ptr - '0';
        ptr++;
    }
    time = num * 3600;
    num = 0;
    ptr++;
    
    /* 分钟部分 */
    while (*ptr != ':' && *ptr != '.' && *ptr != '\0')
    {
        if (*ptr == ' ')
        {
            ptr++;
            continue;
        }
        num *= 10;
        num += *ptr - '0';
        ptr++;
    }
    time += num * 60;
    num = 0;
    ptr++;
    
    /* 秒部分 */
    while (*ptr != '.' && *ptr != ':' && *ptr != '\0')
    {
        if (*ptr == ' ')
        {
            ptr++;
            continue;
        }
        num *= 10;
        num += *ptr - '0';
        ptr++;
    }
    time += num;
    num = 0;
    ptr++;
    
    /* 秒部分 */
    while (*ptr != '.' && *ptr != ':' && *ptr != '\0')
    {
        if (*ptr == ' ')
        {
            ptr++;
            continue;
        }
        num *= 10;
        num += *ptr - '0';
        ptr++;
    }
    time += num / 100.0;
    
    return time;
}

/* 
 * 数组清空 
 * 参数： 
 * 数组首地址/要初始化成的值/成员数量/ 
 * 返回值：
 * ferror函数的返回值 
  */
void arrset(int *array, const int value, const int numberOfMember)
{
    int cnt;
    for(cnt = 0; cnt < numberOfMember; cnt++)
    {
        *(array + cnt) = value;
    }
    return;
}

/*
 * ASS 转义
 * 参数：
 * 目标字符串/源字符串/工作模式（ASS_ESCAPE/ASS_UNESCAPE）
 * 返回值：
 * 目标字符串
 */

char *assEscape(char *dstStr, char *srcStr, int dstStrLen, int mode)
{
    if (srcStr == NULL || dstStr == NULL)
    {
        return NULL;
    }

    int lenCnt = 0;
    char *srcPtr, *dstPtr;
    char *originalTextPtr, *assEscapedTextPtr;
    srcPtr = srcStr;
    dstPtr = dstStr;

    while (*srcPtr != '\0' && lenCnt < dstStrLen-1)
    {
        int listCnt = 0;
        while (lenCnt < dstStrLen-1)
        {
            if (mode == ASS_ESCAPE)
            {
                originalTextPtr = assEscapeList[listCnt].originalText;
                assEscapedTextPtr = assEscapeList[listCnt].assEscapedText;
            }
            else
            {
                originalTextPtr = assEscapeList[listCnt].assEscapedText;
                assEscapedTextPtr = assEscapeList[listCnt].originalText;
            }
            
            if (originalTextPtr == NULL || assEscapedTextPtr == NULL)
            { /* 匹配全部失败，直接拷贝文本 */ 
                *dstPtr = *srcPtr;
                dstPtr++;
                lenCnt++;
                break;
            }
            
            char *srcCmpPtr = srcPtr;
            while (*originalTextPtr != '\0' && *srcCmpPtr != '\0')
            {
                if (*originalTextPtr != *srcCmpPtr)
                {
                    break;
                }
                originalTextPtr++;
                srcCmpPtr++;
            }

            if (*originalTextPtr == '\0')
            {
                /* 拷贝转义后字串 */
                while (*assEscapedTextPtr != '\0')
                {
                    if (lenCnt >= dstStrLen-1)
                    {/* 目标容器超长 */
                        break;
                    }
                    
                    *dstPtr = *assEscapedTextPtr;
                    dstPtr++;
                    assEscapedTextPtr++;
                    lenCnt++;
                }
                
                srcPtr += strlen(assEscapeList[listCnt].originalText)-1;
                break;
            }

            listCnt++;
        }

        srcPtr++;
    }
    
    *dstPtr = '\0';

    return dstStr;
}