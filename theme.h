#ifndef THEME_H
#define THEME_H

#define CP_NORMAL     1
#define CP_CURRENT    2
#define CP_FORMULA    3
#define CP_NUMBER     4
#define CP_VISUAL     5
#define CP_HEADER     6
#define CP_STATUS_N   7
#define CP_STATUS_I   8
#define CP_STATUS_V   9
#define CP_STATUS_CMD 10
#define CP_FORMULABAR 11

void theme_init(void);
int  theme_set(const char *name);

#endif
