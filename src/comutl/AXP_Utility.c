/*
 * Copyright (C) Jonathan D. Belanger 2017.
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
 *	This file contains the utility functions needed throughout the Alpha AXP
 *	emulation code.
 *
 * Revision History:
 *
 *	V01.000		29-May-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		01-Jun-2017	Jonathan D. Belaanger
 *	Added an LRU function to return the least recently used set with a
 *	particular index.
 */
#include "AXP_Utility.h"

/*
 * AXP_LRUAdd
 * 	This function is called to add a particular item into a Least Recently Used
 * 	(LRU) queue.  This list is maintained with the LRU Queue Header's
 * 	forward-link pointing to the LRU entry and the backward-link pointing to
 * 	the Most Recently Used (MRU) item.
 *
 * Input Parameters:
 * 	lruQ:
 * 		A pointer to the LRU Queue Head.
 * 	entry:
 * 		A pointer to the entry to be made the MRU item.  If the entry is
 * 		already the MRU item, then nothing is done.  If it is already in the
 * 		queue, then it is moved to the end of the queue.  Otherwise, it is
 * 		inserted to the end of the queue.
 *
 * Output Parameters:
 * 	lruQ:
 * 		The flink and blink fields may be updated, as well as the flink of the
 * 		current last entry in the queue.
 * 	entry:
 * 		The flink and blink fields may be updated.
 *
 *
 * Return Value:
 * 	None, this always succeeds.
 */
void AXP_LRUAdd(AXP_QUEUE_HDR *lruQ, AXP_QUEUE_HDR *entry)
{

	/*
	 * If the entry is already at the end of the queue, then there is nothing
	 * else to do.
	 */
	if (entry->flink != (void *) &lruQ->flink)
	{

		/*
		 * If the entry does not point to itself, then it is already in the
		 * queue.  Point the forward-link of the entry before this one to the
		 * next entry after us, and point the backward-link of the entry after
		 * us to the entry before us.  This effectively pulls us out of the
		 * queue.
		 */
		if (entry->flink != (void *) &entry->flink)
		{
			((AXP_QUEUE_HDR *) entry->blink)->flink = entry->flink;	// entry after points to entry before
			((AXP_QUEUE_HDR *) entry->flink)->blink = entry->blink;	// entry before points to entry after
		}

		/*
		 * Insert the entry at the end of the queue.  We do this by first
		 * getting the entry to point its forward-link to the queue head and
		 * its backward link to the current last item in the queue (pointed to
		 * by the header's backward-link).  Then we have the current last item
		 * in the queue, point its forward-link and the header's backward-link
		 * to us.
		 */
		entry->flink = (void *) &lruQ->flink;			// entry flink points to queue header
		entry->blink = lruQ->blink;						// entry blink points to last item in queue
		((AXP_QUEUE_HDR *) lruQ->blink)->flink = (void *) &entry->flink;	// flink of last item in queue points to entry
		lruQ->blink = (void *) &entry->flink;			// queue header blink points to entry
	}
	return;
}

/*
 * AXP_LRURemove
 * 	This function is called to remove a particular index/set pair off of a
 * 	Least Recently Used (LRU) list.  This list is maintained with the LRU item
 * 	at the top and the Most Recently Used (MRU) at the bottom.
 *
 * Input Parameters:
 * 	entry:
 * 		A pointer to the entry to be removed from the queue.
 *
 * Output Parameters:
 * 	entry:
 * 		The entry before will point to the entry after, the entry after will
 * 		point to the entry before, and the entry will point to itself.
 *
 * Return Value:
 * 	None.
 */
void AXP_LRURemove(AXP_QUEUE_HDR *entry)
{
	((AXP_QUEUE_HDR *) entry->blink)->flink = entry->flink;	// entry after points to entry before
	((AXP_QUEUE_HDR *) entry->flink)->blink = entry->blink;	// entry before points to entry after
	AXP_INIT_QUEP(entry);				// make the entry point to itself

	/*
	 * Return back to the caller.
	 */
	return;
}

/*
 * AXP_LRUReturn
 * 	This function is called to return the Least Recently Used (LRU) item off
 * 	the list.  This list is maintained with the LRU item at the top and the
 * 	Most Recently Used (MRU) at the bottom.
 *
 * Input Parameters:
 * 	lru:
 * 		A pointer to the list of LRU items.
 *	lruMax:
 *		A value of the last item in the list.
 *
 * Output Parameters:
 *	index:
 *		A pointer to receive the LRU index of the cache, of the index/set pair,
 *		on the list.  Index for the Icache is 33 bits long.
 *	set:
 *		A pointer to receive the LRU set of the cache, of the index/set pair,
 *		on the list.
 *
 * Return Value:
 * 	true:
 * 		If the item was successfully located (the list has items in it).
 * 	false:
 * 		If there are no items on the list.
 */
AXP_QUEUE_HDR *AXP_LRUReturn(AXP_QUEUE_HDR *lruQ)
{
	AXP_QUEUE_HDR *entry;

	if (AXP_QUEP_EMPTY(lruQ))
		entry = NULL;
	else
		entry = (AXP_QUEUE_HDR *) lruQ->flink;
	return(entry);
}