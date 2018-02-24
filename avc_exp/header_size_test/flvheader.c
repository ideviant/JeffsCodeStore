#include <stdio.h>
#include <stdint.h>

#ifndef MIN
#define MIN(a,b)    ((a)<(b) ? (a) : (b))
#endif

int bcut(FILE *fpw, FILE *fpr, int base, int size)
{
    static const int bufsz = (1<<16);
    static char pbuf[bufsz];

    int burst = 0;
    int rsz = 0, wsz = 0;       // single read-write size
    int rtt = 0, wtt = 0;       // accumulate read-write size

    if (0 != fseek(fpr, base, SEEK_SET))
    {
        printf("fseek %d error\n", base);
        return 0;
    }
    
    while(rtt<size && wtt<size)
    {
        burst = MIN(bufsz, size - rtt);
        rsz = fread(pbuf, 1, burst, fpr);   rtt += rsz;
        wsz = fwrite(pbuf, 1, rsz, fpw);    wtt += wsz;
        
        if (!rsz || rsz < burst) {
            if (feof(fpr)) {
                printf("@bcut>> feof\n");
            } else {
                printf("@err>> fread error\n");
            }
            break;
        }
        
        if (!wsz || wsz < rsz) {
            printf("@err>> fwrite error\n");
            break;
        }
    }

    printf("@bcut>> rd:0x%08x, wr:0x%08x\n", rtt, wtt);

    return wtt;
}

int seek_to_type18(FILE *fp)
{
    uint8_t buf[4];

    if (0 != fseek(fp, 13, SEEK_SET)) {
        printf("Error - can't seek to 1st tag\n");
        return 1;
    }

    while (1) 
    {
        if (1 != fread(buf, 4, 1, fp)) {
            printf("Error - can't read tag header\n");
            return 1;
        }
        uint32_t type = buf[0] & 0x1f;
        uint32_t size = (buf[1]<<16) | (buf[2]<<8) | buf[3];

        if (0 != fseek(fp, 11 - 4 + size, SEEK_CUR)) {
            printf("Error - can't seek to tag end\n");
            return 1;
        }

        // 4-byte previous tag sizes
        if (1 != fread(buf, 4, 1, fp)) {
            printf("Error - can't read pre tag size\n");
            return 1;
        }
        uint32_t pre_size = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
        if (pre_size != size + 11) {
            printf("Error - pre tag size %d not match %d + 11\n", pre_size, size);
            return 1;
        }

        // find 1st metadata
        if (type == 18) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Error - you have not specified i/o file\n");
        printf("Usage - mp4header *.moov *.mp4");
        return 1;
    }

    FILE *fpi = fopen(argv[2], "rb");
    if (fpi == NULL) {
        printf("Error - can't open input file %s\n", argv[2]);
        return 1;
    }

    if (0 != seek_to_type18(fpi)) {
        fclose(fpi);
        printf("Error - can't find moov\n");
        return 1;
    }

    FILE *fpo = fopen(argv[1], "wb");
    if (fpo == NULL) {
        printf("Error - can't open output file %s\n", argv[1]);
        fclose(fpi);
        return 1;
    }

    uint32_t rsize = ftell(fpi);
    uint32_t wsize = bcut(fpo, fpi, 0, rsize);
    fclose(fpo);
    fclose(fpi);
    return (rsize!=wsize);
}
