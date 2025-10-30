#include <stdio.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

#define BLOCK_SZ    4096

// 获取文件大小
off_t get_file_size(int fd) {
    struct stat st;

    if(fstat(fd, &st) < 0) {
        perror("fstat");
        return -1;
    }
    if (S_ISBLK(st.st_mode)) {
        // 块设备
        unsigned long long bytes;
        // 使用BLKGETSIZE64 ioctl命令获取块设备大小
        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
            perror("ioctl");
            return -1;
        }
        return bytes;
    } else if (S_ISREG(st.st_mode))
        return st.st_size; // 常规文件直接返回大小

    return -1;
}

// 输出文件内容到控制台
void output_to_console(const char *buf, size_t len) {
    fwrite(buf, 1, len, stdout);
}

int read_and_print_file(char *file_name) {
    struct iovec *iovecs;
    // 打开文件
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd < 0) {
        perror("open");
        return 1;
    }

    // 获取文件大小
    off_t file_sz = get_file_size(file_fd);
    off_t bytes_remaining = file_sz;
    // 计算需要多少个块来存储文件数据
    int blocks = (int) file_sz / BLOCK_SZ;
    if (file_sz % BLOCK_SZ) blocks++;
    // 分配iovec数组内存，一个块对应一个iovec结构
    iovecs = malloc(sizeof(struct iovec) * blocks);

    int current_block = 0;
    // 为要读取的文件分配足够的块以容纳文件数据。
    // 每个块由一个iovec结构描述，该结构作为iovs数组的一部分传递给readv。
    while (bytes_remaining) {
        // 计算当前iovec需要读取的字节数
        off_t bytes_to_read = bytes_remaining;
        if (bytes_to_read > BLOCK_SZ)
            bytes_to_read = BLOCK_SZ;

        void *buf;
        // 为buf分配一块大小为BLOCK_SZ的内存，且地址按BLOCK_SZ对齐
        if( posix_memalign(&buf, BLOCK_SZ, BLOCK_SZ)) {
            perror("posix_memalign");
            return 1;
        }
        // 设置当前iovec的缓冲区和长度
        iovecs[current_block].iov_base = buf;
        iovecs[current_block].iov_len = bytes_to_read;
        // 移动到下一个块
        current_block++;
        // 更新剩余字节数
        bytes_remaining -= bytes_to_read;
    }

    // readv读取文件内容到iovecs数组中，该函数会阻塞。fd是要读取的文件的文件描述符
    int ret = readv(file_fd, iovecs, blocks);
    if (ret < 0) {
        perror("readv");
        return 1;
    }

    for (int i = 0; i < blocks; i++)
        output_to_console(iovecs[i].iov_base, iovecs[i].iov_len); // 将iovecs中的内容输出到控制台

    free(iovecs);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename1> [<filename2> ...]\n",
                argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        // 读取每个文件并打印内容
        if(read_and_print_file(argv[i])) {
            fprintf(stderr, "Error reading file\n");
            return 1;
        }
    }

    return 0;
}