/*
 * mini_shell 功能审查测试 —— 路径解析、命令语义、编辑器行为
 */
#include "ms.h"
#include <stdio.h>
#include <string.h>

static int write_fun(const char* buf, size_t count) {
  fwrite(buf, 1, count, stdout);
  return (int)count;
}

static void feed(const char* cmd) {
  for (const char* p = cmd; *p; p++) ms_input(*p);
}

static void mark(const char* desc) {
  printf("\n--- [%s] ---\n", desc);
}

int main(void) {
  ms_init(write_fun);
  ms_start();
  printf("\n\n");

  printf("############################################################\n");
  printf("#  F1: 绝对路径 + 尾随斜杠（疑似 bug）\n");
  printf("############################################################\n");
  mark("cd /etc          —— 基准，应成功");
  feed("cd /etc\n");
  mark("pwd              —— 应在 /etc");
  feed("pwd\n");
  mark("cd /             —— 回根");
  feed("cd /\n");
  mark("cd /etc/         —— 绝对路径尾随斜杠");
  feed("cd /etc/\n");
  mark("pwd              —— 若失败则仍在 /");
  feed("pwd\n");
  mark("ls /etc/         —— 绝对路径尾随斜杠列目录");
  feed("ls /etc/\n");
  mark("cd /bin/         —— 另一个绝对尾随斜杠");
  feed("cd /bin/\n");
  feed("pwd\n");

  printf("\n############################################################\n");
  printf("#  F2: 双斜杠\n");
  printf("############################################################\n");
  feed("cd /\n");
  mark("cd //etc         —— 双斜杠，POSIX 等价 /etc");
  feed("cd //etc\n");
  feed("pwd\n");
  mark("ls //            —— 双斜杠根");
  feed("ls //\n");
  mark("ls //etc/        —— 双斜杠 + 尾随");
  feed("ls //etc/\n");

  printf("\n############################################################\n");
  printf("#  F3: 相对路径尾随斜杠（对照组）\n");
  printf("############################################################\n");
  feed("cd /\n");
  feed("cd etc\n");
  feed("pwd\n");
  feed("cd /\n");
  mark("cd etc/          —— 相对路径尾随斜杠");
  feed("cd etc/\n");
  feed("pwd\n");

  printf("\n############################################################\n");
  printf("#  F4: . 和 .. 解析\n");
  printf("############################################################\n");
  feed("cd /\n");
  mark("cd ./etc         —— 单点");
  feed("cd ./etc\n");
  feed("pwd\n");
  feed("cd /\n");
  mark("cd ..            —— 在根上 ..，应停在根");
  feed("cd ..\n");
  feed("pwd\n");
  mark("cd /etc/../bin   —— 中途回退");
  feed("cd /etc/../bin\n");
  feed("pwd\n");
  mark("cd /../../..     —— 连续越界回退，应停在根");
  feed("cd /../../..\n");
  feed("pwd\n");

  printf("\n############################################################\n");
  printf("#  F5: cd 无参 / cd 单斜杠\n");
  printf("############################################################\n");
  feed("cd /\n");
  mark("cd（无参）        —— 应留在当前目录还是回家？");
  feed("cd\n");
  feed("pwd\n");
  mark("cd /             —— 单斜杠");
  feed("cd /\n");
  feed("pwd\n");

  printf("\n############################################################\n");
  printf("#  F6: ls 各种路径形式\n");
  printf("############################################################\n");
  feed("cd /\n");
  mark("ls               —— 当前目录");
  feed("ls\n");
  mark("ls .             —— 点");
  feed("ls .\n");
  mark("ls ..            —— 父目录");
  feed("ls ..\n");
  mark("ls /etc          —— 绝对路径");
  feed("ls /etc\n");
  mark("ls etc           —— 相对路径");
  feed("ls etc\n");

  printf("\n############################################################\n");
  printf("#  F7: cat 语义\n");
  printf("############################################################\n");
  mark("cat ls           —— 会找到 /bin/ls 可执行文件？");
  feed("cat ls\n");
  mark("cat /bin/ls      —— 显式路径");
  feed("cat /bin/ls\n");
  mark("cat /nonexist    —— 不存在");
  feed("cat /nonexist\n");

  printf("\n############################################################\n");
  printf("#  F8: echo 重定向语义\n");
  printf("############################################################\n");
  mark("echo hi > /etc   —— 写到目录（应拒绝）");
  feed("echo hi > /etc\n");
  mark("echo hi > /nonexist —— 写不存在的文件");
  feed("echo hi > /nonexist\n");
  mark("echo hi > /bin/ls   —— 写可执行文件");
  feed("echo hi > /bin/ls\n");

  printf("\n############################################################\n");
  printf("#  F9: 编辑器 —— 光标移动与删除\n");
  printf("############################################################\n");
  feed("cd /\n");
  mark("输入 abc，左移一格，插入 X，回车");
  feed("abc");
  ms_input(0x1B); ms_input('['); ms_input('D');   /* ← */
  feed("X\n");
  mark("输入 12345，左移两次，退格");
  feed("12345");
  ms_input(0x1B); ms_input('['); ms_input('D');
  ms_input(0x1B); ms_input('['); ms_input('D');
  ms_input(0x7F);                                  /* Backspace */
  ms_input('\n');
  mark("在空行退格 —— 边界");
  ms_input(0x7F);
  ms_input(0x7F);
  ms_input('\n');
  mark("行首左移 —— 边界");
  for (int i = 0; i < 5; i++) {
    ms_input(0x1B); ms_input('['); ms_input('D');
  }
  ms_input('\n');
  mark("行尾右移 —— 边界");
  feed("ab");
  for (int i = 0; i < 5; i++) {
    ms_input(0x1B); ms_input('['); ms_input('C');
  }
  ms_input('\n');

  printf("\n############################################################\n");
  printf("#  F10: Tab 补全\n");
  printf("############################################################\n");
  feed("cd /\n");
  mark("输入 e + Tab（应补全 etc 还是列出候选？）");
  feed("e");
  ms_input('\t');
  ms_input('\n');
  mark("输入 空 + Tab（应列出候选）");
  ms_input('\t');
  ms_input('\n');
  mark("输入 cd /e + Tab");
  feed("cd /e");
  ms_input('\t');
  ms_input('\n');
  feed("pwd\n");

  printf("\n############################################################\n");
  printf("#  F11: 历史命令\n");
  printf("############################################################\n");
  feed("cd /\n");
  feed("pwd\n");
  feed("ls\n");
  mark("连续上箭头 3 次后回车（应执行 ls 或更早的）");
  for (int i = 0; i < 3; i++) { ms_input(0x1B); ms_input('['); ms_input('A'); }
  ms_input('\n');
  mark("上箭头到底再继续上（边界）");
  for (int i = 0; i < 12; i++) { ms_input(0x1B); ms_input('['); ms_input('A'); }
  ms_input('\n');
  mark("下箭头到底再继续下（边界）");
  for (int i = 0; i < 12; i++) { ms_input(0x1B); ms_input('['); ms_input('B'); }
  ms_input('\n');

  printf("\n############################################################\n");
  printf("#  F12: 空命令 / 纯空格\n");
  printf("############################################################\n");
  ms_input('\n');
  feed("   \n");
  feed("\t\n");
  mark("以上均不应崩溃或产生错误输出");

  printf("\n############################################################\n");
  printf("#  功能审查测试完毕\n");
  printf("############################################################\n");
  return 0;
}
