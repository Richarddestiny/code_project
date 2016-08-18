/*
 * Copyright (c) 2013-2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */


#ifndef INCLUDED_NVCAM_FRAME_DATA_STORE_H
#define INCLUDED_NVCAM_FRAME_DATA_STORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "nvcam_frame_data.h"
#include "nvcam_frame_dataitem_ids_private.h"

#define FRAME_DATASTORE_DATA_CAPACITY           (128)
#define FRAME_DATASTORE_CLIENT_CHAR_COUNT       (128)
#define FRAME_DATASTORE_MAX_NAMEDPOINTS          (64)
#define FRAME_DATASTORE_MAX_FRAMES_IN_USE         (16)


/**
 *
 *  ////////////////// Functional Block Diagram ///////////////////
 *
 *
 *  |----------|    * SyncPoint Increment+
 *  | Capture  |--->
 *  | Request  |    |
 *  |----------|    |-------------------|
 *        ^         * Frame Arrived     |
 *        L---------V  Callback         |         |^^^^^^^^^^^^^^^^^^^^^^^^^^|
 *        |                             V         | NvCamFrameData(FRD) Pool |------->>>|
 *  |----------|            |---------------------|-------------|............|          |
 *  |  VI/ISP  |            |       NvCamFrameDataStore         |                       |
 *  |----------|            |-------------------------------^---|                       |
 *       ^v                                                 |                           |
 *       ^v                                                 |                           |
 *       ^v       |----|   |----|                           |   |--------|              |
 *       ^ > > > >| F1 |...| F2 |............>................> | FRD 1  |              | * Clients creates FRDs
 *       ^        |----|   |----|                           |   | -----  |              |   to write
 *       ^           ^        ^                             |   | | F1 | | ...          |   FrameDataItem (Instructions/data)
 *       ^           |        L-----<<<<--(+)-------|       |   | ------ |              |
 *       ^           |     Flow         |------|  |-|----|  |   L--------|              |
 *  |----------|     L----<<<--(+)--<<<-| FRD1 |..| FRD2 |.......                       |
 *  |   PCL    |                        |------|  |------|  |                           |
 *  |----------|                            ^          ^    |                           |
 *                                          |          | * Gets new FRD from data store.|
 *  |  Sensor  |                            |----------| * Writes New FR data item.     |
 *  |----------|                            |          |----^                           |
 *  |  Focuser |                        |------| ..|-------|-----------<<<<-------------|
 *  |----------|                        |  AE  |   |  AWB  |
 *  |  Flash   |                        |------|   |-------|
 *  |----------|
 */


typedef enum NvCamNamedPoint
{
    // To query the most recent FRD created for a new capture request from the app.
    NvCamNamedPoint_APIRequest,
    NvCamNamedPoint_HostCaptureRequest,
    // To query the most recent FRD pushed to the PCL layer to capture.
    NvCamNamedPoint_PhysicalCaptureRequest,
    // To query the most recent FRD currently serviced by the sensor hw.
    NvCamNamedPoint_PhysicalCaptureStart,
    // To query the most recent FRD serviced by the ISP hw.
    NvCamNamedPoint_ISPOutput,
    NvCamNamedPoint_SIZE,
    NvCamNamedPoint_Force32 = 0x7FFFFFF,
} NvCamNamedPoint;

/**
 * Defines the named point list. A named point
 * is any stage in the camera pipeline which is
 * of significant interest to clients. It is a way
 * of associating a significant event in time with
 * a NvCamFrameData object. Clients can request for
 * NvCamFrameData objects at a named point in the
 * camera system. For example if the camera system
 * is defined to have 3 basic named points.Below
 * diagram will show how the NvCamFrameData objects
 * are associated with the named points. Note that
 * the table below shows multiple snapshots of the
 * named point list as new frames enter the camera
 * system.
 *
 *    Named Points |       Snapshots of the Named Point List in Time
 *  ......................................................................>>>
 *  |--------------|--- Frame Nth --|-- Frame N+1 --|--Frame N+2 --------->>>
 *  | API request  |      5         |      6        |      7
 *  |--------------|                |               |
 *  | Last Cap Req |     NULL       |      5        |      6
 *  |--------------|                |               |
 *  |  ISP Done    |     NULL       |     NULL      |      5
 *  |--------------|                |               |
 *
 *
 */
typedef struct NvCamNamedPointList
{
    NvCamFrameData_Handle FRD[NvCamNamedPoint_SIZE];
} NvCamNamedPointList;


/**
 *  NvCamFrameDataStore performs the task of
 *  managing the Frame Data objects associated with frames
 *  in the camera pipeline.
 */
typedef struct NvCamFrameDataStore
{
    NvU32 UniqueIdCurrent;//...............Used by the data store to assign new IDs to frame
                                        // data objects.
    NvOsMutexHandle hFRDUseMutex;//....... Lock to ensure thread-safety of the FRD.
    NvCamFrameData_Handle ReprocessFRDInUseList[FRAME_DATASTORE_DATA_CAPACITY];
    NvU32 ReprocessFRDLastElementIdx;

    NvCamFrameData FrameDataPool[FRAME_DATASTORE_DATA_CAPACITY];// A pool of NvCamFrameData objects.


    NvCamDataInfoHandle hDataInfo;//.......A helper to store/retrieve NvCamFrameDataItem values.
                                        // This is defined by the meta data framework.

    NvCamNamedPointList NamedPointList;//..A List which maps named points to the
                                        // frame data objects. NOTE: Multiple frame data
                                        // objects can be associated with a single
                                        // named point.

    NvCamFrameDataItemIDMap *pDataItemMap;// A map which tracks static identifiers to
                                          // dynamic descriptors at run time.
} NvCamFrameDataStore;

typedef NvCamFrameDataStore* NvCamFrameDS_Handle;


/**
 *  Functions associated with the NvCamFrameDataStore
 *  object.
 */

// Allocates a new frame data store.
NvError NvCamFrameDS_Init(NvCamFrameDS_Handle *phFrameDataStore);

// Releases a frame data store.
void NvCamFrameDS_Destroy(NvCamFrameDS_Handle hFrameDataStore);

// Used by the client to create new frame data object when it wants
// to perform some operations on it. Usually an API request
// would trigger the frame data creation.
NvCamFrameData_Handle NvCamFrameDS_CreateFrameData(
    NvCamFrameDS_Handle hDataStore,
    NvU64 ReprocessId,
    NvBool isReprocessingRequest);

// Function searches previously stored reprocessing FRD's which
// match the framecount specified in the call.
NvCamFrameData_Handle NvCamFrameDS_SearchFrameDataWithFrameCount(
    NvCamFrameDS_Handle hDataStore,
    NvU64 FrameCount);

// Releases reprocessing FRDs identified by the list.
void NvCamFrameDS_ReProcessFrameDataRelease(
    NvCamFrameDS_Handle hDataStore,
    NvU64 *pList,
    NvU32 SizeOfList);

// Releases a specific Frame data object.
NvError NvCamFrameDS_FrameDataRelease(
    NvCamFrameDS_Handle hDataStore,
    NvCamFrameData_Handle hFrameData);

// Gets a descriptor handle for the frame data item registered
// with the nvdata framework. Clients are assumed to always have
// access to statically enumerated IDs and they can use descriptors
// to get hold of the data items.
NvCamDataItemDescriptorHandle NvCamFrameDS_GetDescriptorForDataItemID(
    NvCamFrameDS_Handle hDataStore,
    NvU32 StaticId);

// Get a frame data object at a particular named point in the camera pipeline.
NvCamFrameData_Handle NvCamFrameDS_GetFrameDataAtNamedPoint(
    NvCamFrameDS_Handle hDataStore,
    NvCamNamedPoint NamedPoint);

// Get a frame data object at a particular named point from a
// snapshot namedpoint list.
NvCamFrameData_Handle NvCamFrameDS_GetFrameDataFromNamedPointSnapShot(
    NvCamFrameDS_Handle hDataStore,
    NvCamNamedPointList *pNamedPointList,
    NvCamNamedPoint NamedPoint);

// Set a frame data object at a particular named point in the camera pipeline.
NvError NvCamFrameDS_SetFrameDataAtNamedPoint(
    NvCamFrameDS_Handle hDataStore,
    NvCamFrameData_Handle hFrameData,
    NvCamNamedPoint NamedPoint);

// Gets the current snapshot of named point list in the data store.
// Client needs to allocate the memory needed to hold the named point
// list.
NvError NvCamFrameDS_GetNamedPointListSnapShot(
    NvCamFrameDS_Handle hDataStore,
    NvCamNamedPointList *pNamedPointList);

// Reset the current state of the data store. Actions include
// clearing any cached Frame data queues and named point list.
NvError NvCamFrameDS_ResetState(NvCamFrameDS_Handle hDataStore);

void NvCamFrameDS_PrintNamedPointList(NvCamFrameDS_Handle hDataStore);

#ifdef __cplusplus
}
#endif
#endif // INCLUDED_NVCAM_FRAME_DATA_STORE_H

