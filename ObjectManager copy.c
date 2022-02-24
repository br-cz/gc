

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// The number of bytes of memory we have access to -- put here so
// everyone's consistent.
#define MEMORY_SIZE 1024 * 512

// For any unsuccessful insertion of objects
#define NULL_REF 0

typedef unsigned long Ref;
typedef unsigned long ulong;
typedef unsigned char uchar;
typedef struct BUFFER_NODES bufferNode;

struct BUFFER_NODES
{
    ulong offset; // starting index of object
    ulong size;   // length of the memory of object
    int refCount; // the amount of objects in existence
    Ref ID;       // unique ID, starts at 1
    bufferNode *next;
};

// the two buffers
uchar *bufrOne;
uchar *bufrTwo;

uchar *bufrPtr; // should point to b_1 at first

static ulong freePtr = __LONG_MAX__; // an impossible number to mean that there is no pool created yet
static Ref currID;                   // 1 at first, increments per insert

// Used for LL insertion, deletion and traversal
static bufferNode *top;
static bufferNode *currNode;
static bufferNode *prevNode;

/*.
 * This function tries to allocate a block of given size from our buffer.
 * It will fire the garbage collector as required.
 * We always assume that an insert always creates a new object...
 * On success it returns the reference number for the block of memory
 * allocated for the object.
 * On failure it returns NULL_REF (0)
 */
Ref insertObject(ulong);
static Ref initList(bufferNode *, ulong);
static Ref insertLast(bufferNode *, ulong);

// returns a pointer to the object being requested given by the reference id
void *retrieveObject(Ref ref);

// update our index to indicate that we have another reference to the given object
void addReference(Ref ref);

// update our index to indicate that a reference is gone
void dropReference(Ref ref);
static void removeObject(Ref ID);
static void removeAfter(Ref ID);

// initialize the object manager
void initPool();

// clean up the object manager (before exiting)
void destroyPool();

/*
 * This function traverses the index and prints the info in each entry
 * corresponding to a block of allocated memory. You should print the
 * block's reference id, its starting address, and its size (in
 * bytes).
 */
void dumpPool();

static void compact();
static void copyBuffer(uchar[]);

// invariants
static void validate_node(bufferNode *);
static void validate_list();

static void validate_node(bufferNode *currNode)
{
    assert(currNode);
    assert(currNode->offset >= 0 && currNode->offset < MEMORY_SIZE);
    assert(currNode->size < MEMORY_SIZE);
    assert(currNode->offset + currNode->size < MEMORY_SIZE);
    assert(currNode->refCount > 0);
    assert(currNode->ID > 0);
}

static void validate_list()
{
    bufferNode *prev;
    bufferNode *curr;

    if (top != NULL)
    {
        validate_node(top);

        prev = top;
        curr = top->next;

        while (curr && prev->ID < curr->ID)
        {
            validate_node(prev);
            validate_node(curr);

            prev = curr;
            curr = curr->next;
        }
    }
}

//------------------------------------------------------
// initPool
//
// PURPOSE: initializes the current pool
// OUTPUT PARAMETERS:
// Allocate memory for the two buffers, resets freePtr and currID, and sets top to NULL
//------------------------------------------------------
void initPool()
{
    // make sure there is no list or buffer yet, as mentioned above in freePtr init
    if (freePtr != __LONG_MAX__)
    {
        destroyPool();
    }

    // allocate memory, instead of using a file
    bufrOne = (uchar *)malloc(MEMORY_SIZE);
    bufrTwo = (uchar *)malloc(MEMORY_SIZE);

    bufrPtr = bufrOne; // first buffer should be buffer One
    freePtr = 0;       // now the free ptr is the first index in the buffer
    currID = 1;        // and the first object has an id 1
    top = NULL;        // there is no object in buffer yet during initialization

    assert(!top);
}

//------------------------------------------------------
// destroyPool
//
// PURPOSE: destroys the current pool and frees any allocated memory
//------------------------------------------------------
void destroyPool()
{
    currNode = top;                   // start at top
    bufferNode *temp = NULL;          // to free current node
    assert(currNode == top && !temp); // make sure

    while (currNode)
    {
        validate_node(currNode); // makes sure curr node actualy exists

        temp = currNode;
        validate_node(temp); // make sure temp exists

        currNode = currNode->next; // move curr along

        free(temp);
    }
    assert(currNode == NULL); // make sure curr was freed properly

    top = NULL;
    assert(top == NULL); // make sure top no longer points to anything

    freePtr = __LONG_MAX__; // freePtr points at an impossible value
    assert(freePtr == __LONG_MAX__);

    free(bufrOne);
    free(bufrTwo);
}

//------------------------------------------------------
// insertObject
//
// PURPOSE: attempts to insert an object of specific size in the buffer, if applicable
// INPUT PARAMETERS:
// size - the size of the new object to be inserted
// OUTPUT PARAMETERS:
// Returns the ID of an inserted object, if inserted, else NULL_REF
//------------------------------------------------------
Ref insertObject(ulong size)
{
    Ref returnID = NULL_REF;
    bufferNode *newNode = (bufferNode *)malloc((sizeof(bufferNode))); // we always assume that this method creates a new object as per instructions

    assert(returnID == NULL_REF);

    if (!top)
    {
        assert(!top);

        returnID = initList(newNode, size);

        assert(returnID != currID); // make sure we have allocated and moved the IDs properly

        validate_node(top);
        validate_list();
    }
    else
    {
        validate_list();
        validate_node(top);

        // if we have enough space
        if (freePtr + size <= MEMORY_SIZE)
        {
            assert(freePtr + size <= MEMORY_SIZE);
            returnID = insertLast(newNode, size); // insert new item to end of LL
        }
        else
        {
            assert(freePtr + size > MEMORY_SIZE); // mak sure we dont actually have enough space
            compact();                            // try to free space
            returnID = insertLast(newNode, size); // and try to insert once more
        }

        validate_list(); // make sure list is still functional after insertion attempts
    }

    assert(returnID >= NULL_REF);
    return returnID;
}

//------------------------------------------------------
// initList
//
// PURPOSE: adds on object unto an empty list, if applicable
// INPUT PARAMETERS:
// newNode - node to be inserted as the first object in buffer
// size - new objects size
// OUTPUT PARAMETERS:
// Returns the ID of an inserted object, if inserted, else NULL_REF
//------------------------------------------------------
static Ref initList(bufferNode *newNode, ulong size)
{
    Ref refID = NULL_REF;

    assert(!top); // make sure there isnt actually a list already

    if (size <= MEMORY_SIZE)
    {
        assert(size <= MEMORY_SIZE);

        newNode->offset = 0;
        newNode->size = size;
        newNode->refCount = 1;
        newNode->ID = currID;
        newNode->next = NULL;

        validate_node(newNode);

        top = newNode;

        validate_node(top);
        assert(top == newNode);

        refID = currID++; // move along ID after assigning the current ID for return

        freePtr = newNode->offset + newNode->size; // move along the next free index pointer

        assert(refID != NULL_REF); // make sure we've assigned a proper ID if insertion was possible
        validate_list();
    }
    else
    {
        printf("Unable to successfully complete memory allocation request.\n");
        assert(refID == NULL_REF);
    }

    return refID;
}

//------------------------------------------------------
// insertList
//
// PURPOSE: insert a node at the end if there is already a list
// INPUT PARAMETERS:
// newNode - node to be inserted as the first object in buffer
// size - new objects size:
// OUTPUT PARAMETERS:
// Returns the ID of an inserted object, if inserted, else NULL_REF
//------------------------------------------------------
static Ref insertLast(bufferNode *newNode, ulong size)
{
    Ref refID = NULL_REF;

    validate_list();

    prevNode = NULL;
    currNode = top;

    // traverse to last, making sure we're validating the list along the way
    while (currNode)
    {
        validate_node(currNode);

        prevNode = currNode;
        currNode = currNode->next;

        validate_node(prevNode);
        if (currNode)
        {
            validate_node(currNode);
        }

        validate_list();
    }

    assert(!currNode); // make sure weve hit the last node/object in LL

    if (freePtr + size <= MEMORY_SIZE)
    {
        assert(freePtr + size <= MEMORY_SIZE);

        newNode->offset = freePtr;
        newNode->size = size;
        newNode->refCount = 1;
        newNode->ID = currID;
        newNode->next = NULL;

        validate_node(newNode);

        refID = currID++;

        prevNode->next = newNode;

        freePtr = newNode->offset + newNode->size;

        assert(newNode->offset < freePtr);

        // make sure weve moved the ID along properly
        assert(prevNode->ID < newNode->ID);
        assert(newNode->ID < currID);
        assert(refID != NULL_REF);
        validate_list();
    }
    else
    {
        printf("Unable to successfully complete memory allocation request.\n");
        assert(refID == NULL_REF); // unsuccessful insertion means NULL ID
    }

    return refID;
}

//------------------------------------------------------
// dumpPool
//
// PURPOSE: Print out the details of every existing object in buffer
//------------------------------------------------------
void dumpPool()
{
    currNode = top; // start at first node

    if (!currNode)
    {
        printf("Empty.\n");
    }
    else
    {
        // if this exists, make sure top actually exists and we've assigned succesfully
        assert(top);
        assert(currNode == top);

        // Print the Current Node's: ID, Starting Address/Offset, and its size in bytes
        while (currNode)
        {
            assert(currNode);

            printf(" Current Node:\n\tID:%lu, Starting Adress: %lu, Size(in bytes): %lu\n", currNode->ID, currNode->offset, currNode->size);

            currNode = currNode->next;

            validate_list();
        }
    }

    assert(!currNode); // make sure we've actually reached the end of list
}

//------------------------------------------------------
// retrieveObject
//
// PURPOSE: Return the address of the object w/ the matching ID, if it exists else NULL
// INPUT PARAMETERS:
// ID - the ID of the object to be retrieved
// OUTPUT PARAMETERS:
// Return the address of the object w/ the matching ID, if it exists else NULL.
//------------------------------------------------------
void *retrieveObject(Ref ID)
{
    void *address = NULL; // Assume the object does not exist at first

    currNode = top; // start at top

    while (currNode)
    {
        if (currNode->ID == ID)
        {
            address = &bufrPtr[currNode->offset];
            assert(address != NULL);
        }
        currNode = currNode->next;
    }

    return address;
}

//------------------------------------------------------
// addReferecne
//
// PURPOSE: Increment the amount of reference to the object w/ the matching ID
// INPUT PARAMETERS:
// ID - the ID of the object to be retrieved
//------------------------------------------------------
void addReference(Ref ID)
{
    currNode = top;

    while (currNode)
    {
        assert(currNode);
        if (currNode->ID == ID)
        {
            currNode->refCount++;
            assert(currNode->refCount > 1); // bigger than 1 because this object wouldnt exist if the previous ID was <1
        }
        currNode = currNode->next;
    }
}

//------------------------------------------------------
// dropReference
//
// PURPOSE: Decrement the amount of reference to the object w/ the matching ID
//          Delete the matching object if its reference count is 0
// INPUT PARAMETERS:
// ID - the ID of the object to be retrieved
//------------------------------------------------------
void dropReference(Ref ID)
{
    currNode = top;

    while (currNode)
    {
        if (currNode->ID == ID)
        {
            currNode->refCount--;

            if (currNode->refCount == 0)
            {
                assert(currNode->refCount == 0);

                removeObject(currNode->ID);
            }
            else
            {
                assert(currNode->refCount >= 1);
            }
        }

        if (currNode)
        {
            assert(currNode);
            currNode = currNode->next;
        }
    }
}

//------------------------------------------------------
// removeObject
//
// PURPOSE: Remove the object from the list w/ the matching ID
// INPUT PARAMETERS:
// ID - the ID of the object to be retrieved
//------------------------------------------------------
static void removeObject(Ref ID)
{
    if (top->ID == ID)
    {
        bufferNode *temp = top; // store top temporarily

        assert(top->ID == ID);
        top = top->next; // skip over current top

        free(temp); // free previous top
    }
    else
    {
        assert(top->ID != ID);
        removeAfter(ID);
    }
}

//------------------------------------------------------
// removeAfter
//
// PURPOSE: Remove the object after top from the list w/ the matching ID
// INPUT PARAMETERS:
// ID - the ID of the object to be retrieved
//------------------------------------------------------
static void removeAfter(Ref ID)
{
    int removed = 0; // assume nothing has been removed yet, to avoid looping unnecessarily after removal

    assert(top->ID != ID);

    // starts at the next node from top, if it exists
    prevNode = top;
    currNode = top->next;

    while (currNode && !removed)
    {
        if (currNode->ID == ID)
        {
            assert(currNode->ID == ID);

            prevNode->next = currNode->next; // skip over currNode
            removed = 1;                     // stop traversing now that weve found the node to remove

            free(currNode); // free the removed node

            assert(removed);
        }

        // if currNode has not been freed
        if (currNode)
        {
            prevNode = currNode;
            currNode = currNode->next;
        }
    }

    assert(!currNode || removed);
}

//------------------------------------------------------
// compact
//
// PURPOSE: defragments memory via the double buffering strategy
// OUTPUT PARAMETERS:
// This also prints out a statistic after garbage removal which includes: # of objects, # of bytes in use and # of bytes freed
//------------------------------------------------------
static void compact()
{
    int numObjects = 0;
    ulong numBytes = 0;             // num of bytes in use
    ulong bytesCollected = freePtr; // subtracts the current ptr aka the amount used up theoritically, vs the amount actually used later

    currNode = top; // start at 1st object

    while (currNode)
    {
        assert(currNode);

        numObjects++;
        numBytes += currNode->size; // size of currNode is in bytes

        currNode = currNode->next;
    }

    bytesCollected -= numBytes; // free ptr is the assumed amount of memory taken - amount actually used

    // If were currently using bufrOne, swich over to bufrTwo and vice versa
    if (bufrPtr == bufrOne)
    {
        assert(bufrPtr == bufrOne);
        copyBuffer(bufrTwo);
    }
    else
    {
        assert(bufrPtr = bufrTwo);
        copyBuffer(bufrOne);
    }

    printf("\nGarbage Collector Statistics:\n");
    printf("Objects: %d || Bytes in use: %lu || Bytes freed: %lu\n\n", numObjects, numBytes, bytesCollected);
}

//------------------------------------------------------
// copyBuffer
//
// PURPOSE: copies over the contents of the current buffer to the other
// INPUT PARAMETERS:
// newBuffer - the new buffer where the current buffer's contents are being transferred to
//------------------------------------------------------
static void copyBuffer(uchar *newBuffer)
{
    currNode = top; // start at top
    freePtr = 0;    // reset pointer

    while (currNode)
    {
        assert(currNode);

        assert(currNode->size < MEMORY_SIZE);

        memcpy(&newBuffer[freePtr], &bufrPtr[currNode->offset], currNode->size); // copy contents from the right address of the respective buffers

        currNode->offset = freePtr;
        assert(currNode->offset == freePtr);

        freePtr += currNode->size; // move along free ptr

        currNode = currNode->next;

        assert(freePtr < MEMORY_SIZE);
    }

    bufrPtr = newBuffer;
    assert(bufrPtr == newBuffer);
}