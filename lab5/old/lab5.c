/*
    Написать на языке С программу с помощью библиотеки FUSE, которая подключает виртуальную файловую систему, дерево директорий которой (полученное с помощью команды tree) задано ниже.
    Файловая система содержит 4 директории: foo, bar, baz и bin,— а также 4 файла, из которых 3 — текстовые файлы: example, readme.txt, test,— и 1 бинарный (содержимое бинарного файла должно быть взято из соответствующей стандартной системной утилиты, название которой соответствует названию файла: ls, grep, pwd,...) Содержимое остальных файлов:
    ●	readme.txt: Student <имя и фамилия>, <номер зачетки>
    ●	test.txt: <Любой текст на ваш выбор с количеством строк равным последним двум цифрам номера зачетки>
    ●	example: Hello world
    Файловая система должна монтироваться в папку /mnt/fuse/, после чего должна быть возможность осуществить листинг ее директорий и просмотр содержимого виртуальных файлов. При обращении к файловой системе должны проверяться права доступа (маска прав указана в дереве директорий через слеш после имени файла). Владельцем всех файлов должен быть текущий пользователь, который выполняет монтирование системы.

   Вариант 1:

    /
    |--bar/705
    |   |--bin/700
    |   |   |--echo/555
    |   |   `--readme.txt/400
    |   `--baz/644
    |       `--example/222
    `--foo/233
        `--test.txt/007

    Дополнительно должна быть реализована функция `mkdir`.
*/

#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static const char *readme_str = "Student Дмитрий Лапин 16150061";
static const char *example_str = "Hello world";
static char testtxt_str[61*2] = "";
static char mkdired_path[256] = " ";
static mode_t *mkdired_mode;
static int hello_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    //stbuf->st_uid = getuid();
    stbuf->st_uid = 1000;
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/bar") == 0){
        stbuf->st_mode = S_IFDIR | 0705;
        stbuf->st_nlink = 2+1+1;
    }
    else if (strcmp(path, "/bar/bin") == 0){
        stbuf->st_mode = S_IFDIR | 0700;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/bar/bin/echo") == 0){
        stbuf->st_mode = S_IFREG | 0555;
        stbuf->st_nlink = 1;

        struct stat buffer;
        stat("/bin/echo", &buffer);//Получение размера /bin/echo
        stbuf->st_size = buffer.st_size;
    }
    else if (strcmp(path, "/bar/bin/readme.txt") == 0){
        stbuf->st_mode = S_IFREG | 0400;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(readme_str);
    }
    else if (strcmp(path, "/bar/baz") == 0){
        stbuf->st_mode = S_IFDIR | 0644;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/bar/baz/example") == 0){
        stbuf->st_mode = S_IFREG | 0222;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(example_str);
    }
    else if (strcmp(path, "/foo") == 0){
        stbuf->st_mode = S_IFDIR | 0233;
        stbuf->st_nlink = 2;
    }
    else if (strcmp(path, "/foo/test.txt") == 0){
        stbuf->st_mode = S_IFREG | 007;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(testtxt_str);
    }
    else if (strcmp(path, mkdired_path) == 0){
        stbuf->st_mode = mkdired_mode;
        stbuf->st_nlink = 2;
    }
    else{
        res = -ENOENT;
    }
            
    return res;
}
static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;
    if (strcmp(path, "/") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "foo", NULL, 0);
        filler(buf, "bar", NULL, 0);
        if (strcmp(mkdired_path, " ") != 0){
            filler(buf, mkdired_path+1, NULL, 0);
        }
        return 0;
    }
    else if (strcmp(path, "/bar") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "bin", NULL, 0);
        filler(buf, "baz", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/bar/bin") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "echo", NULL, 0);
        filler(buf, "readme.txt", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/bar/baz") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "example", NULL, 0);
        return 0;
    }
    else if (strcmp(path, "/foo") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "test.txt", NULL, 0);
        return 0;
    }
    else{
        return -ENOENT;
    }
}
static int hello_open(const char *path, struct fuse_file_info *fi)
{
    /*
    if (strcmp(path, hello_path) != 0)
            return -ENOENT;
    if ((fi->flags & 3) != O_RDONLY)
            return -EACCES;
    */
    return -EACCES;
}
static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    char *fileBuffer;
    if (strcmp(path, "/bar/bin/echo") == 0) {
        struct stat echo_stat;
        stat("/bin/echo", &echo_stat);//Получение размера /bin/echo
        len = echo_stat.st_size;

        FILE *f;
        unsigned char buffer[len];
        f = fopen("bin/echo", "r");
        fread(buffer, len, 1, f);
        fileBuffer = buffer;        
    }
    else if (strcmp(path, "/bar/bin/readme.txt") == 0) {
        len = strlen(readme_str);
        fileBuffer = readme_str;
    }
    else if (strcmp(path, "/bar/baz/example") == 0) {
        len = strlen(example_str);
        fileBuffer = example_str;
    }
    else if (strcmp(path, "/foo/test.txt") == 0) {
        len = strlen(testtxt_str);
        fileBuffer = testtxt_str;
    }
    else{
        return -ENOENT;
    }

    if (offset < len){
        if (offset + size > len){
            size = len-offset;
        }
        memcpy(buf, fileBuffer+offset, size);
        return size;
    }
    else{
        return 0;
    }
}

static int hello_mkdir(const char *path, mode_t mode){
    /*
    strcpy(mkdired_path, "/");
    strcat(mkdired_path, path);
    mkdired_mode = mode;
    return 1;
    */
    int res;
    
        res = mkdir(path, mode);
        if (res == -1)
            return -errno;
    
        return 0;
}
// fuse_operations hello_oper is redirecting function-calls to _our_ functions implemented above
static struct fuse_operations hello_oper = {
    .getattr        = hello_getattr,
    .readdir        = hello_readdir,
    .mkdir          = hello_mkdir,
    .open           = hello_open,
    .read           = hello_read,
};
// in the main function we call the blocking fuse_main(..) function with &hello_oper 
int main(int argc, char *argv[])
{
    for (int i=0; i<61*2; i++){//<Любой текст на ваш выбор с количеством строк равным последним двум цифрам номера зачетки>
        strcat(testtxt_str, "1\n");
    }
    return fuse_main(argc, argv, &hello_oper, NULL);
}