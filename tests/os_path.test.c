#define OS_PATH_IMPL
#include "../os_path.h"

#include <string.h>

int main(void) {

    Os_Path unix_path = os_path_create_styled("c:\\home\\user\\directory\\file", OS_PATH_STYLE_UNIX);
    if (strncmp(unix_path.path, "c:/home/user/directory/file", 27) != 0) {
        return 1;
    } 

    Os_Path windows_path = os_path_create_styled("c:/home/user/directory/file", OS_PATH_STYLE_WINDOWS);
    if (strncmp(windows_path.path, "c:\\home\\user\\directory\\file", 27) != 0) {
        return 1;
    }

    return 0;
}
