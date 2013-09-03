#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>


#define FPGA_RD_LOC "/dev/roach/mem"
#define MMAP 1

int main(void) {

        int fpga_fd = -1;
        struct timeval t1, t2;
        int elapsed;
        int i;
        int readval=0;
#ifdef MMAP
        size_t length = 1024 * 1024;
        off_t offset = 0;
        int prot = (PROT_READ | PROT_WRITE);
        int flags = MAP_SHARED;
        void *addr = 0;
#endif


        fpga_fd = open(FPGA_RD_LOC, O_RDWR);
        if(fpga_fd == 0){
                printf("ERR: Opening of %s failed\n", FPGA_RD_LOC);
                return -1;
        }

#ifdef MMAP
        addr = mmap(NULL, length, prot, flags, fpga_fd, offset);
        if(addr == 0){
                printf("memory mapping failed for %s\n", FPGA_RD_LOC);
                return -1;
        }
#endif

        /* start timer */
        gettimeofday(&t1, NULL);


#ifdef MMAP
        addr = mmap(NULL, length, prot, flags, fpga_fd, offset);
        /* write, read and verify scratchpad register result at loc:0xC half a million times*/
        for(i = 0; i < 500000; i++){

                *((int *)( addr + 0xC)) = 0xcafebabe; 

                readval = *((int *) (addr + 0xC));

                if(readval != 0xcafebabe){
                        printf("verification inconsistency at %dth time\n", i);
                        break;
                }

        }

#endif

        /* stop timer */
        gettimeofday(&t2, NULL);

        /* Compute and print elapsed time in millisec */
        elapsed = (t2.tv_sec - t1.tv_sec) * 1000;    //  s to ms
        elapsed += (t2.tv_usec - t1.tv_usec) / 1000; // us to ms

        printf("Elapsed time in ms : %d\n", elapsed);

#ifdef MMAP
        if(munmap(addr, length) == -1){
                printf("ERR: Unmapping unsuccessful\n");
                return -1;
        }
#endif

        if(close(fpga_fd) == -1){
             printf("ERR: Closing of FPGA file descriptor failed\n");
             return -1;
        }

        return 0;
}
