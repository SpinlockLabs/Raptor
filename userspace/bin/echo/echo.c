#include <liblox/io.h>

int main(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        puts(argv[i]);
    }
    puts("\n");
    return 0;
}
