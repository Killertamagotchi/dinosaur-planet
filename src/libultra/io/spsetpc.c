// @DECOMP_OPT_FLAGS=-O1
#include <PR/os_internal.h>
#include <PR/rcp.h>

s32 __osSpSetPc(u32 data)
{
    register u32 stat = IO_READ(SP_STATUS_REG);
    if (!(stat & SP_STATUS_HALT))
        return -1;
    else
    {
        IO_WRITE(SP_PC_REG, data);
    }
    return 0;
}
