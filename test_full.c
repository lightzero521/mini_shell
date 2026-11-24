/*
 * mini_shell 全面边界测试
 * 覆盖：所有内置命令的 argc 边界、超长输入、路径 API、历史命令
 */
#include "ms.h"
#include <stdio.h>
#include <string.h>

static int write_fun(const char* buf, size_t count) {
  fwrite(buf, 1, count, stdout);
  return (int)count;
}

static int g_fail = 0;

/* 喂入一条命令（含换行） */
static void feed(const char* cmd) {
  for (const char* p = cmd; *p; p++) {
    ms_input(*p);
  }
}

/* 喂入 N 个字符 + 换行 */
static void feed_n(char c, int n) {
  for (int i = 0; i < n; i++) ms_input(c);
  ms_input('\n');
}

int main(void) {
  ms_init(write_fun);
  ms_start();

  printf("\n\n");
  printf("============================================================\n");
  printf("  T1: pwd —— 任意参数数量\n");
  printf("============================================================\n");
  feed("pwd\n");
  feed("pwd a\n");
  feed("pwd a b c d e f g\n");

  printf("\n============================================================\n");
  printf("  T2: ls —— argc 1..7（原崩溃点）\n");
  printf("============================================================\n");
  feed("ls\n");
  feed("ls /\n");
  feed("ls / a\n");            /* argc=3 → 曾 SEGV */
  feed("ls a b c d\n");        /* argc=5 */
  feed("ls a b c d e f g\n");  /* argc=7 → 曾越界写 arg_map[5] */

  printf("\n============================================================\n");
  printf("  T3: cd —— 各种参数数量\n");
  printf("============================================================\n");
  feed("cd\n");
  feed("cd /\n");
  feed("cd / a\n");
  feed("cd a b c d e f g\n");
  feed("cd /\n");  /* 回到根 */

  printf("\n============================================================\n");
  printf("  T4: cat —— 各种参数数量\n");
  printf("============================================================\n");
  feed("cat\n");
  feed("cat notexist\n");
  feed("cat a b\n");
  feed("cat a b c d e f g\n");

  printf("\n============================================================\n");
  printf("  T5: echo —— argc 1/2/3/4/5+\n");
  printf("============================================================\n");
  feed("echo\n");
  feed("echo hello\n");
  feed("echo a b\n");
  feed("echo hello > /f1\n");       /* argc=4 合法 */
  feed("echo hello > /f1 extra\n"); /* argc=5 非法 */
  feed("echo a b c d e f g\n");

  printf("\n============================================================\n");
  printf("  T6: clear / help —— 参数数量\n");
  printf("============================================================\n");
  feed("help\n");
  feed("help a b c d e f g\n");
  feed("clear a b c d e f g\n");

  printf("\n============================================================\n");
  printf("  T7: 超长输入（62/63/64/80/200 字符）\n");
  printf("============================================================\n");
  feed_n('x', 62);
  feed_n('x', 63);
  feed_n('x', 64);
  feed_n('x', 80);
  feed_n('x', 200);

  printf("\n============================================================\n");
  printf("  T8: 大量空格（连续空格边界）\n");
  printf("============================================================\n");
  feed("ls     \n");              /* 尾部多空格 */
  feed("ls    a    b    c    d    e    f    g\n");
  feed("   ls\n");                /* 前导空格 */
  feed("echo     hello\n");       /* 中间多空格 */

  printf("\n============================================================\n");
  printf("  T9: 路径 API 直接调用（64+ 字节路径）\n");
  printf("============================================================\n");
  {
    char longpath[200];
    memset(longpath, 'a', sizeof(longpath) - 1);
    longpath[0] = '/';
    longpath[sizeof(longpath) - 1] = '\0';

    ms_item_t* out = NULL;
    ms_status_t st = ms_path_to_dir(longpath, &out);
    printf("ms_path_to_dir(200字节) -> status=%d, out=%p\n", (int)st,
           (void*)out);

    out = NULL;
    st = ms_path_to_file(longpath, &out);
    printf("ms_path_to_file(200字节) -> status=%d, out=%p\n", (int)st,
           (void*)out);

    /* 恰好 MS_MAX_CMD_LENGTH 长度，最容易触发不终止 bug */
    char exact[MS_MAX_CMD_LENGTH + 1];
    memset(exact, 'b', MS_MAX_CMD_LENGTH);
    exact[0] = '/';
    exact[MS_MAX_CMD_LENGTH] = '\0';
    out = NULL;
    st = ms_path_to_dir(exact, &out);
    printf("ms_path_to_dir(64字节) -> status=%d, out=%p\n", (int)st, (void*)out);
  }

  printf("\n============================================================\n");
  printf("  T10: 历史命令（上下键）—— 检查 history_buff 是否被污染\n");
  printf("============================================================\n");
  feed("ls\n");
  feed("pwd\n");
  feed("echo history_test\n");
  /* 模拟连按上箭头 4 次 */
  for (int i = 0; i < 4; i++) {
    ms_input(0x1B); /* ESC */
    ms_input('[');
    ms_input('A');
  }
  ms_input('\n');

  printf("\n============================================================\n");
  printf("  全部测试执行完毕，无崩溃则通过\n");
  printf("============================================================\n");
  printf("\n最终状态：x=%d y=%d\n", g_fail, 0);
  return 0;
}
