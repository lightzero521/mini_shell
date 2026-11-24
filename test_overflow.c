#include "ms.h"
#include <stdio.h>
#include <string.h>

static int write_fun(const char* buf, size_t count) {
    fwrite(buf, 1, count, stdout);
    return (int)count;
}

int main(void) {
    ms_init(write_fun);
    ms_start();

    printf("\n\n########## 测试1：输入5个参数（边界内）##########\n");
    const char* cmd1 = "ls a b c d\n";
    for (const char* p = cmd1; *p; p++) ms_input(*p);

    printf("\n########## 测试2：输入7个参数（触发 arg_map[5] 越界）##########\n");
    const char* cmd2 = "ls a b c d e f g\n";
    for (const char* p = cmd2; *p; p++) ms_input(*p);

    printf("\n########## 测试3：输入恰好64字节（触发 strncpy 不终止）##########\n");
    /* 64 个字符，无 \0 */
    for (int i = 0; i < 64; i++) ms_input('x');
    ms_input('\n');

    printf("\n########## 测试4：超长输入 80 字节 ##########\n");
    for (int i = 0; i < 80; i++) ms_input('y');
    ms_input('\n');

    printf("\n########## 测试完成 ##########\n");
    return 0;
}
