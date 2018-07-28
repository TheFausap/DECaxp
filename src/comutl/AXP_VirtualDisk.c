/*
 * Copyright (C) Jonathan D. Belanger 2018.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied only
 * in accordance with the terms of such license and with the inclusion of the
 * above copyright notice.  This software or any other copies thereof may not
 * be provided or otherwise made available to any other person.  No title to
 * and ownership of the software is hereby transferred.
 *
 * The information in this software is subject to change without notice and
 * should not be construed as a commitment by the author or co-authors.
 *
 * The author and any co-authors assume no responsibility for the use or
 * reliability of this software.
 *
 * Description:
 *
 *  This source file contains the code to support the Virtual Disk interface
 *  consistent with Microsoft's.
 *
 * Revision History:
 *
 *  V01.000	08-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_VirtualDisk.h"
#include "AXP_Utility.h"
#include "AXP_Blocks.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_VHDX.h"
#include "AXP_VHD.h"
#include "AXP_RAW.h"

/*
 * AXP_VHD_Create
 *  Creates a virtual hard disk (VHD) image file, either using default
 *  parameters or using an existing virtual disk.
 *
 * Input Parameters:
 *  storageType:
 *	A pointer to a VIRTUAL_STORAGE_TYPE structure that contains the desired
 *	disk type and vendor information.
 *  path:
 *	A pointer to a valid string that represents the path to the new virtual
 *	disk image file.
 *  accessMask:
 *	The AXP_VHD_ACCESS_MASK value to use when opening the newly created
 *	virtual disk file.  If the version member of the param parameter is set
 *	to AXP_VHD_CREATE_VER_2 then only the AXP_VHD_ACCESS_NONE (0) value may
 *	be specified.
 *  securityDsc:
 *	An optional pointer to a AXP_VHD_SECURITY_DSC to apply to the virtual
 *	disk image file.  If this parameter is NULL, the parent directory's
 *	security descriptor will be used.
 *  flags:
 *	Creation flags, which must be a valid combination of the
 *	AXP_VHD_CREATE_FLAG enumeration.
 *  providerSpecFlags
 *	Flags specific to the type of virtual disk being created. May be zero
 *	if none are required.
 *  param:
 *	A pointer to a valid AXP_VHD_CREATE_PARAM structure that contains
 *	creation parameter data.
 *  async:
 *	An optional pointer to a valid AXP_VHD_ASYNC structure if asynchronous
 *	operation is desired.
 *
 * Output Parameters:
 *  handle:
 *  	A pointer to the handle object that represents the newly created
 *  	virtual disk.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_INV_PARAM:		An invalid parameter or combination of
 *				parameters was detected.
 *  AXP_VHD_FILE_EXISTS:	File already exists.
 *  AXP_VHD_INV_HANDLE:		Failed to create the VHDX file.
 *  AXP_VHD_WRITE_FAULT:	An error occurred writing to the VHDX file.
 */
u32 AXP_VHD_Create(
		AXP_VHD_STORAGE_TYPE *storageType,
		char *path,
		AXP_VHD_ACCESS_MASK accessMask,
		AXP_VHD_SEC_DSC *securityDsc,
		AXP_VHD_CREATE_FLAG flags,
		u32 providerSpecFlags,
		AXP_VHD_CREATE_PARAM *param,
		AXP_VHD_ASYNC *async,
		AXP_VHD_HANDLE *handle)
{
    char		*parentPath;
    u64			diskSize;
    u32			retVal = AXP_VHD_SUCCESS;
    u32			blkSize, sectorSize, deviceID, parentDevID;

    /*
     * Go check the parameters and extract some information from within them.
     */
    retVal = AXP_VHD_ValidateCreate(
    		storageType,
    		path,
    		accessMask,
    		flags,
    		param,
    		handle,
		&parentPath,
		&parentDevID,
    		&diskSize,
    		&blkSize,
    		&sectorSize,
    		&deviceID);

    /*
     * If the parameters we good, then we can proceed with trying to create the
     * virtual hard disk.
     */
    if (retVal == AXP_VHD_SUCCESS)
    {

	/*
	 * Based on storage type, call the appropriate create function.
	 */
	switch (deviceID)
	{

	    /*
	     * Create a VHD formatted virtual disk.
	     */
	    case STORAGE_TYPE_DEV_VHD:
		retVal = _AXP_VHD_Create(
			    path,
			    flags,
			    parentPath,
			    parentDevID,
			    diskSize,
			    blkSize,
			    sectorSize,
			    deviceID,
			    handle);
		break;


	    /*
	     * Create a VHDX formatted virtual disk.
	     */
	    case STORAGE_TYPE_DEV_VHDX:
		retVal = _AXP_VHDX_Create(
			    path,
			    flags,
			    parentPath,
			    parentDevID,
			    diskSize,
			    blkSize,
			    sectorSize,
			    deviceID,
			    handle);
		break;

	    /*
	     * We don't create RAW or ISO disks.  For RAW disks, we are accessing
	     * the physical disk drive.  For ISO disks, these have an file format
	     * that is embedded in the disk, so at the system level, we should not
	     * be making any assumptions.
	     */
	    case STORAGE_TYPE_DEV_RAW:
	    case STORAGE_TYPE_DEV_ISO:
	    case STORAGE_TYPE_DEV_UNKNOWN:
	    default:
		retVal = AXP_VHD_CALL_NOT_IMPL;
		break;
	}
    }

    /*
     * Return the result of this call back to the caller.
     */
    return(retVal);
}

/*
 * AXP_VHD_Open
 *  This function is called to open an already created virtual disk.  This
 *  function can be called to determine, based on the file path, whether the
 *  disk being opened is a VHDX, VHD, or RAW format.  The last is used for
 *  physical disks.
 *
 * Input Parameters:
 *  storageType:
 *	A pointer to a VIRTUAL_STORAGE_TYPE structure that contains the desired
 *	disk type and vendor information.
 *  path:
 *	A pointer to a valid string that represents the path to the virtual
 *	disk image file or physical disk.
 *  accessMask:
 *	The AXP_VHD_ACCESS_MASK value to use when opening the virtual disk
 *	file or physical disk.
 *  flags:
 *	Open flags, which must be a valid combination of the
 *	AXP_VHD_OPEN_FLAG enumeration.
 *  param:
 *	A pointer to a valid AXP_VHD_OPEN_PARAM structure that contains open
 *	parameter data.
 *
 * Output Parameters:
 *  handle:
 *  	A pointer to the handle object that represents the newly opened virtual
 *  	or physical disk.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_INV_PARAM:		An invalid parameter or combination of
 *				parameters was detected.
 *  AXP_VHD_FILE_NOT_FOUND:	Virtual disk file not found.
 *  AXP_VHD_PATH_NOT_FOUND:	Physical disk file not found.
 *  AXP_VHD_INV_HANDLE:		Failed to create the VHDX file.
 */
u32 AXP_VHD_Open(
		AXP_VHD_STORAGE_TYPE *storageType,
		char *path,
		AXP_VHD_ACCESS_MASK accessMask,
		AXP_VHD_OPEN_FLAG flags,
		AXP_VHD_OPEN_PARAM *param,
		AXP_VHD_HANDLE *handle)
{
    u32			retVal = AXP_VHD_SUCCESS;
    u32			deviceID;

    /*
     * Go check the parameters and extract some information from within them.
     */
    retVal = AXP_VHD_ValidateOpen(
    		storageType,
    		path,
    		accessMask,
    		flags,
    		param,
    		handle,
    		&deviceID);

    /*
     * If the device type indicates ANY, we need to go and determine what
     * the file format actually is.  This call does not verify or validate
     * the contents, it just looks for an indicate of what the file format
     * actually is.
     */
    if ((retVal == AXP_VHD_SUCCESS) && (deviceID == STORAGE_TYPE_DEV_ANY))
	retVal = AXP_VHD_GetDeviceID(path, &deviceID);

    /*
     * If the parameters look good, then we can proceed with trying to open
     * the virtual/physical disk (or determining which we have).
     */
    if (retVal == AXP_VHD_SUCCESS)
    {

	/*
	 * Based on storage type, call the appropriate open function.
	 */
	switch (deviceID)
	{

	    /*
	     * Create a VHD formatted virtual disk.
	     */
	    case STORAGE_TYPE_DEV_VHD:
		retVal = _AXP_VHD_Open(
			    path,
			    flags,
			    deviceID,
			    handle);
		break;


	    /*
	     * Create a VHDX formatted virtual disk.
	     */
	    case STORAGE_TYPE_DEV_VHDX:
		retVal = _AXP_VHDX_Open(
			    path,
			    flags,
			    deviceID,
			    handle);
		break;

	    case STORAGE_TYPE_DEV_RAW:
	    case STORAGE_TYPE_DEV_ISO:
		retVal = _AXP_RAW_Open(
			    path,
			    flags,
			    deviceID,
			    handle);
		break;

	    /*
	     * We don't create RAW or ISO disks.  For RAW disks, we are accessing
	     * the physical disk drive.  For ISO disks, these have an file format
	     * that is embedded in the disk, so at the system level, we should not
	     * be making any assumptions.
	     */
	    case STORAGE_TYPE_DEV_UNKNOWN:
	    default:
		retVal = AXP_VHD_CALL_NOT_IMPL;
		break;
	}
    }

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}

/*
 * AXP_VHD_CloseHandle
 *  Closes an open object handle.
 *
 * Input Parameters:
 *  handle:
 *	A valid handle to an open object.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  AXP_VHD_SUCCESS:		Normal Successful Completion.
 *  AXP_VHD_INV_HANDLE:		Failed to create the VHDX file.
 */
u32 AXP_VHD_CloseHandle(AXP_VHD_HANDLE handle)
{
    AXP_VHDX_Handle	*vhdx = (AXP_VHDX_Handle *) handle;
    u32			retVal = AXP_VHD_SUCCESS;

    /*
     * Verify that we have a proper handle.
     */
    if (AXP_ReturnType_Block(handle) == AXP_VHDX_BLK)
	AXP_Deallocate_Block(vhdx);
    else
	retVal = AXP_VHD_INV_HANDLE;

    /*
     * Return the results of this call back to the caller.
     */
    return(retVal);
}
