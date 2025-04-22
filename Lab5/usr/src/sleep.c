#include <unistd.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    int s = atoi(argv[1]);
    sleep(s);
    return 0;
}
