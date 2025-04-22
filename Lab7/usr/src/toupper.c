#include <unistd.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char const *argv[])
{
    char buffer[256];
    int readChars = 0, i;
    do
    {
        readChars = read(STDIN_FILENO, buffer, 255);
        for(i = 0; i < readChars; i++) buffer[i] = toupper(buffer[i]);
        write(STDOUT_FILENO, buffer, readChars);
    } while (readChars);
    
    return 0;
}
