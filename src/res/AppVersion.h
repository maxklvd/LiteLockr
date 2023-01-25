#ifndef APP_VERSION_H
#define APP_VERSION_H

#define APP_VERSION_MAIN  2
#define APP_VERSION_MAIN2 0
#define APP_VERSION_SUB   0
#define APP_VERSION_SUB2  0

#define TOSTRING2(arg) #arg
#define TOSTRING(arg) TOSTRING2(arg)

#define APP_VERSION4_STR TOSTRING(APP_VERSION_MAIN) "." \
						 TOSTRING(APP_VERSION_MAIN2) "." \
						 TOSTRING(APP_VERSION_SUB) "." \
						 TOSTRING(APP_VERSION_SUB2)

#endif // APP_VERSION_H