/**
 * Copyright (c) 2014 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */


#ifndef NVVICACHE_H
#define NVVICACHE_H

#include "nvos.h"
#include "nvodm_query_camera.h"

#define MAX_COMMAND_SEQUENCE_SIZE 2000

/**
 * Enum defining the type of operations to be supported by cache function
 */
typedef enum NvViCacheCmd {
    NvViCacheCmd_NOOP = 0,
    NvViCacheCmd_USLEEP_SYNC,
    NvViCacheCmd_I2C_WR_SYNC,
    NvViCacheCmd_VGP_WR_SYNC,
    NvViCacheCmd_BEGIN_VENDOR_EXTENSIONS = 0x10000000,
    NvViCacheCmd_FORCE32 = 0x7FFFFFFF,
} NvViCacheCmd;

typedef struct NvViCacheI2cReg {
    NvU16 bus;
    NvU16 address;
    NvU16 offset;
    NvU16 value;
} NvViCacheI2cReg;

typedef struct NvViCacheSleep {
    NvU32 usleep;
} NvViCacheSleep;

/**
 * Struct container for vi package.
 */
typedef struct NvViCacheCommand
{
    NvU8 Dirty;
    NvViCacheCmd Command;
    union {
        NvViCacheI2cReg Reg;
        NvViCacheSleep Sleep;
    } NvViCacheRegCmd;
    /**
     * __Data mapping based on Command__
     * NvViCmd_USLEEP_SYNC : NvViSleep
     * NvViCmd_I2C_WR_SYNC : NvViI2cReg
     */
} NvViCacheCommand;

typedef struct NvViCacheContextRec *NvViCacheHandle;

/**
 * NvViCacheOpen creates a new instance of NvViCache.
 * @param phContext a pointer to a NULL handle that will be assigned a new instance.
 * @returns BadParameter if hContext is not NULL; InsufficientMemory if no available
 *      memory for cache instance; else NvSuccess
 */
NvError NvViCacheOpen(NvViCacheHandle *phContext);
/**
 * NvViCacheClose closes instance of NvViCache.
 * @param hContext a pointer to the instance to be closed.
 * @returns BadParameter if hContext is NULL; else NvSuccess
 */
NvError NvViCacheClose(NvViCacheHandle hContext);
/**
 * NvViCacheGetVersion returns NvViCache version
 * @param hContext a pointer to the instance.
 * @param pVersion a pointer to NvU32 to be filled with version.
 * @returns BadParameter if hContext is NULL; else NvSuccess
 */
NvError NvViCacheGetVersion(NvViCacheHandle hContext, NvU32 *pVersion);
/**
 * NvViCacheAdd cache a new register command.
 * @param hContext a pointer to the instance to be closed.
 * @param pDev a pointer to the target device's profile.
 * @param offset the target register offset|address.
 * @param val the target register value.
 * @returns BadParameter if params are NULL;
        InsufficientMemory if cache is full; else NvSuccess
 */
NvError NvViCacheAdd(NvViCacheHandle hContext, DeviceProfile *pDev, NvU32 offset, NvU32 val);
/**
 * NvViCacheDirty marks a command as dirty (permission to overwrite)
 * @param hContext a pointer to the instance to be closed.
 * @param pCacheCommand a pointer to the command to mark dirty.
 * @returns BadParameter if params are NULL; else NvSuccess
 */
NvError NvViCacheDirty(NvViCacheHandle hContext, NvViCacheCommand *pCacheCommand);
/**
 * NvViCacheDirtyDevice marks a all commands associated with a specified
 *      device as dirty (permission to overwrite)
 * @param hContext a pointer to the instance to be closed.
 * @param pDev a pointer to the device profile to mark all associated commands as dirty.
 * @returns BadParameter if params are NULL; else NvSuccess
 */
NvError NvViCacheDirtyDevice(NvViCacheHandle hContext, DeviceProfile *pDev);
/**
 * NvViCacheDirtyAll marks a all commands in cache as dirty (permission to overwrite)
 * @param hContext a pointer to the instance to be closed.
 * @returns BadParameter if params are NULL; else NvSuccess
 */
NvError NvViCacheDirtyAll(NvViCacheHandle hContext);
/**
 * NvViCacheGetCache grabs a pointer to the cache memory.
 * @param hContext a handle to an existing instance of NvViCacheContext.
 * @param phCommandCache a NULL pointer to that will get assigned the buffer from hContext.
 * @param pMaxCommands pointer to the size of the buffer pDestCommands is providing.
 * @returns BadParameter if hContext is NULL; else NvSuccess with pMaxCommands the size of cache.
 */
NvError NvViCacheGetCache(NvViCacheHandle hContext, NvViCacheCommand *phCommandCache, NvU32 *pMaxCommands);


#endif /* NVVICACHE_H */
