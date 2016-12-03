#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AC5xDrv_Drv.h"

typedef struct {
    Tac5xMiiBootPacket block;
    U32 size;
    U32 last;
}AC5X_PROGROM_BLOCK;

#define AC5X_PROGROM_BLOCK_MAX         2000
#define AC5X_PROGROM_BLOCK_HEADER_SIZE 8

static AC5X_PROGROM_BLOCK *ac5xProgramBlock = NULL;
static U32 ac5xProgramBlockIdMax = 0;

void ac5x_program_block_destroy(void){
    ac5xProgramBlockIdMax = 0;

    if (NULL != ac5xProgramBlock) {
        free(ac5xProgramBlock);
        ac5xProgramBlock = NULL;
    }

    return;
}

U32 ac5x_program_block_init(char *fileName){
    FILE *fd;
    U32 i;

    ac5x_program_block_destroy();

    if (NULL == (fd = fopen(fileName, "rb"))
        || (0 != fseek(fd, AC5X_PROGRAM_DOWNLOAD_HEADER_SIZE__IN_BYTES, SEEK_SET))) {
        AC5X_DEBUG("Not dsp file please input file in file\n");
        return AC5X_FAIL;
    }

    if (NULL == (ac5xProgramBlock = malloc(AC5X_PROGROM_BLOCK_MAX * sizeof(AC5X_PROGROM_BLOCK)))) {
        AC5X_DEBUG("No memory\n");
        fclose(fd);
        return AC5X_FAIL;
    }

    memset((char *)ac5xProgramBlock, 0, (AC5X_PROGROM_BLOCK_MAX * sizeof(AC5X_PROGROM_BLOCK)));

    for (i = 0; i < AC5X_PROGROM_BLOCK_MAX; i++) {
        Tac5xMiiBootPacket *block = &ac5xProgramBlock[i].block;
        SplitFieldsFromLong(block->MessageId, AC5X_MII_MESSAGE_ID__PROGRAM_DOWNLOAD);
        SplitFieldsFromLong(block->u.ProgramDownload.BlockId, i);

        if (AC5X_PROGROM_BLOCK_HEADER_SIZE == fread(((char *)&block->u.ProgramDownload) + 4,
                                                    1,
                                                    AC5X_PROGROM_BLOCK_HEADER_SIZE,
                                                    fd)) {
            U32 blockSize = (block->u.ProgramDownload.BlockSize_Msb << 8) + block->u.ProgramDownload.BlockSize_Lsb;

            if (AC5X_MII_MAX_DOWNLOAD_BLOCK_SIZE >= blockSize) {
                if (blockSize == fread(block->u.ProgramDownload.Data, 1, blockSize, fd)) {
                    ac5xProgramBlock[i].size = 16 + blockSize;

                    if (AC5X_PROGRAM_DOWLOAD__LAST_BLOCK_FLAG == block->u.ProgramDownload.Control) {
                        ac5xProgramBlock[i].last = 1;
                        break;
                    }

                    continue; //so far everthing is ok, continue to read next block.
                } else {
                    AC5X_DEBUG("Failed to read block '%lu'.\n", i);
                }
            } else {
                AC5X_DEBUG("Block '%lu' size '%lu' too large.\n", i, blockSize);
            }
        } else {
            AC5X_DEBUG("Failed to read block '%lu' header.\n", i);
        }

        break; //something wrong, break the loop
    }

    fclose(fd);

    if (i < AC5X_PROGROM_BLOCK_MAX && 1 == ac5xProgramBlock[i].last) {
        ac5xProgramBlockIdMax = i;
        AC5X_DEBUG("Program block init success test, '%lu' blocks in tatal.\n", i);
        return AC5X_OK;
    }

    AC5X_DEBUG("Program block init failed, '%lu' blocks in tatal.\n", i);
    ac5x_program_block_destroy();
    return AC5X_FAIL;
}

U32 ac5x_program_block_get(U32 blockId, void **block, U32 *blockSize, U32 *last) {
    if (blockId > ac5xProgramBlockIdMax || NULL == block || NULL == blockSize || NULL == last) {
        return AC5X_FAIL;
    }

    *block = (void *)&ac5xProgramBlock[blockId].block;
    *blockSize = ac5xProgramBlock[blockId].size;
    *last = ac5xProgramBlock[blockId].last;
    return AC5X_OK;
}

void ac5x_program_block_test(void){
    U32 i;

    if (0 == ac5xProgramBlockIdMax) {
        AC5X_DEBUG("Program block is empty\n");
        return;
    }

    AC5X_DEBUG("%lu program blocks\n", ac5xProgramBlockIdMax + 1);
    AC5X_DEBUG("ID\tsize\tisEnd\t\tID\tsize\tisEnd\t\tID\tsize\tisEnd\t\tID\tsize\tisEnd\n");

    for (i = 0; i < AC5X_PROGROM_BLOCK_MAX; i++) {
        void *block;
        U32 blockSize, last;
        if (0 == ac5x_program_block_get(i, &block, &blockSize, &last)) {
            AC5X_DEBUG("%lu\t", i);
            AC5X_DEBUG("%lu\t", ac5xProgramBlock[i].size);
            AC5X_DEBUG("%lu", ac5xProgramBlock[i].last);

            if ((i + 1) % 4) {
                AC5X_DEBUG("\t\t");
            } else {
                AC5X_DEBUG("\n");
            }
        }
    }

    AC5X_DEBUG("\n");
    return;
}

