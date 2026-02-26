// stdafx.cpp : 仅包含标准包含文件的源文件
// SampleRdkRenderer.pch 将是预编译头文件
// stdafx.obj 将包含预编译的类型信息

#include "stdafx.h"

/** 【Gemini 注解】：
    这行代码虽然简单，但它是整个工程编译最耗时的地方。
    编译器运行到这里时，会一口气吞下你在 stdafx.h 里引用的所有 Rhino SDK 和 RDK 头文件，
    然后吐出一个几百 MB 的 .pch 二进制文件。

    之后你写黑洞逻辑的其他 .cpp 文件，只需要直接“吃”这个现成的二进制文件，
    而不需要重新解析那几万行代码。
*/