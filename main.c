//-----------------------------------------
// NAME: Bricz Cruz
// STUDENT NUMBER: 7909956
// COURSE: COMP 2160, SECTION: A01
// INSTRUCTOR: Dr. Mehdi Niknam
// ASSIGNMENT: 3, QUESTION: 1
//
// REMARKS: For testing the implementation of table.h
//
//-----

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include "ObjectManager.h"

void testCreation();
void testInsert();
void testRetrieval();
void testReferences();
void testDump();

static int testsFailed = 0;
static int testsExecuted = 0;
int main()
{
    testCreation();
    testInsert();
    testRetrieval();
    testReferences();
    testDump();

    printf("\nTotal number of tests executed: %d", testsExecuted);
    printf("\nNumber of tests passed: %d", testsExecuted - testsFailed);
    printf("\nNumber of tests failed: %d\n", testsFailed);

    printf("\nProgram completed normally.\n");
    return 0;
}

void testCreation()
{
    printf("*****Beginning testing of creation pool/buffer.\n*****");
    initPool();

    printf("Should result in empty list only.\n");
    dumpPool();
    testsExecuted++; //not quite sure how else to test creation, as there is no access to any variables that check creation
}

void testInsert()
{
    Ref id1, id2, id3, id4, idImpossible;
    char *objPtr;

    initPool();

    printf("\n*****Beginning testing of insertions.*****\n");

    printf("\nTesting insertion of object with size 100. Should succeed.\n");
    id1 = insertObject(100);
    if (id1 != 0)
    {
        printf("Success. Insertion of object returned a non NULL_REF(0) of %lu.\n", id1);
    }
    else
    {
        printf("Failured. Insertion of object returned a NULL_REF(0).\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("\nTesting insertion of object with size 200. Should succeed.\n");
    id2 = insertObject(200);
    if (id2 != 0)
    {
        printf("Success. Insertion of object returned a non NULL_REF(0).\n");
    }
    else
    {
        printf("Failured. Insertion of object returned a NULL_REF(0).\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("\nTesting insertion of object with size of MEMORY_SIZE. Should fail.\n");
    idImpossible = insertObject(MEMORY_SIZE);
    if (idImpossible == 0)
    {
        printf("Success. Insertion of object returned a NULL_REF(0).\n");
    }
    else
    {
        printf("Failured. Insertion of object returned a non NULL_REF(0).\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("\nTesting insertion of object with size 3000. Should succeed.\n");
    id3 = insertObject(3000);
    if (id3 != 0)
    {
        printf("Success. Insertion of object returned a non NULL_REF(0).\n");
    }
    else
    {
        printf("Failured. Insertion of object returned a NULL_REF(0).\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("\nTesting insertion of object with size 4000. Should succeed.\n");
    id4 = insertObject(4000);
    if (id4 != 0)
    {
        printf("Success. Insertion of object returned a non NULL_REF(0).\n");
    }
    else
    {
        printf("Failured. Insertion of object returned a NULL_REF(0).\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("Now testing deletion of created buffer. Retrieval of id1 should result in NULL_REF (if retrieval works properly.\n");
    destroyPool();
    objPtr = (char *)retrieveObject(id1);
    if (objPtr == NULL)
    {
        printf("Success. Cannot retrieve from a deleted buffer.\n");
    }
    else
    {
        printf("Failure. Retrieved from a deleted buffer.\n");
        testsFailed++;
    }
    testsExecuted++;
}

void testRetrieval()
{
    Ref id1, id2, id3, idImpossible;
    char *objPtr;

    initPool();

    printf("\n*****Beginning testing of retrieval of objects.*****\n");

    id1 = insertObject(100);
    id2 = insertObject(20000);
    idImpossible = insertObject(MEMORY_SIZE - 20000);
    id3 = insertObject(200);

    printf("Retrieving id1 and overwriting its contents with the alphabet.\n");
    objPtr = (char *)retrieveObject(id1);
    for (int i = 0; i < 100; i++)
    {
        objPtr[i] = (char)(i % 26 + 'A');
    }

    objPtr = (char *)retrieveObject(id1);
    printf("Now testing if overwriting the retrieved memory space works.\n");
    if (objPtr[0] == 'A' && objPtr[99] == 'V')
    {
        printf("Success. Expected values are in memory.\n");
    }
    else
    {
        printf("Failure. Expected values are not in memory.\n");
        testsFailed++;
    }
    fprintf(stdout, "\n");

    objPtr = (char *)retrieveObject(idImpossible);
    printf("Now testing if retrieving memory from assigning of an impossible value works. Should fail.\n");
    if (objPtr == NULL)
    {
        printf("Success. Cannot retrieve an impossibly sized object values are in memory.\n");
    }
    else
    {
        printf("Failure. Impossibly sized object is in memory.\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("Retrieving id3 after failed insertion and overwriting its contents with the alphabet.\n");
    objPtr = (char *)retrieveObject(id3);
    for (int i = 0; i < 100; i++)
    {
        objPtr[i] = (char)(i % 26 + 'A');
    }

    objPtr = (char *)retrieveObject(id3);
    printf("Now testing if retrieving an inserted object after failed insertion works. Should succeed.\n");
    if (objPtr[0] == 'A' && objPtr[99] == 'V')
    {
        printf("Success. Expected values are in memory.\n");
    }
    else
    {
        printf("Failure. Expected values are not in memory.\n");
        testsFailed++;
    }
    fprintf(stdout, "\n");

    printf("Now testing deletion of created buffer. Retrieval of id1 should result in NULL_REF (if retrieval works properly.\n");
    destroyPool();
    objPtr = (char *)retrieveObject(id1);
    if (objPtr == NULL)
    {
        printf("Success. Cannot retrieve from a deleted buffer.\n");
    }
    else
    {
        printf("Failure. Retrieved from a deleted buffer.\n");
        testsFailed++;
    }
    testsExecuted++;
}

void testReferences()
{
    Ref id1, id2, id3, idImpossible;
    char *objPtr;

    initPool();

    printf("\n*****Beginning testing of reference methods of objects.*****\n");

    id1 = insertObject(100);
    id2 = insertObject(20000);
    idImpossible = insertObject(MEMORY_SIZE - 20000);
    id3 = insertObject(2000);

    printf("Now testing if adding references works properly. If retrieval works properly, overwritting to an object with an added reference should work.\n");
    addReference(id1);
    objPtr = (char *)retrieveObject(id1);
    for (int i = 0; i < 100; i++)
    {
        objPtr[i] = (char)(i % 26 + 'A');
    }

    objPtr = (char *)retrieveObject(id1);
    printf("Now testing if overwriting the retrieved memory space works.\n");
    if (objPtr[0] == 'A' && objPtr[99] == 'V')
    {
        printf("Success. Expected values are in memory and adding references does not change its existence entirely.\n");
    }
    else
    {
        printf("Failure. Expected values are not in memory and adding messes up the buffer memory with regards to the object.\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("Now testing if deleting references works properly. If retrieval works properly, overwritting to an object with a deleted reference should not work and be deleted instead.\n");
    dropReference(id2);
    objPtr = (char *)retrieveObject(id2);
    if (objPtr == NULL)
    {
        printf("Success. Cannot retrieve an impossibly sized object values are in memory.\n");
    }
    else
    {
        printf("Failure. Impossibly sized object is in memory.\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("Now testing if adding an object after deleting reference works properly. If retrieval works properly, overwritting to an object with an added reference should work.\n");
    id2 = insertObject(MEMORY_SIZE - 15000);
    objPtr = (char *)retrieveObject(id2);
    for (int i = 0; i < 100; i++)
    {
        objPtr[i] = (char)(i % 26 + 'A');
    }

    printf("Now testing if overwriting the retrieved memory space works.\n");
    if (objPtr[0] == 'A' && objPtr[99] == 'V')
    {
        printf("Success. Expected values are in memory and adding references does not change its existence entirely.\n");
    }
    else
    {
        printf("Failure. Inserting after removal of an object results in failed retrieval of some sort.\n");
        testsFailed++;
    }
    testsExecuted++;

    printf("Now testing deletion of created buffer. Retrieval of id1 should result in NULL_REF (if retrieval works properly.\n");
    destroyPool();
    objPtr = (char *)retrieveObject(id1);
    if (objPtr == NULL)
    {
        printf("Success. Cannot retrieve from a deleted buffer.\n");
    }
    else
    {
        printf("Failure. Retrieved from a deleted buffer.\n");
        testsFailed++;
    }
    testsExecuted++;
}

void testDump()
{
    Ref id1, id2, id3, idImpossible;
    // char *objPtr;

    initPool();

    printf("\n*****Beginning testing of dumpPool() methods of objects.*****\n");

    id1 = insertObject(100);
    id2 = insertObject(20000);
    idImpossible = insertObject(MEMORY_SIZE - 20000);
    id3 = insertObject(2000);

    printf("The buffer should contain id1,id2,id3 if every other method works properly.There is no way to check within the program but the visual output should justify test pass.\n");
    dumpPool();
    testsExecuted++;

    dropReference(id2);
    printf("The buffer should contain id1,id3 if every other method works properly.There is no way to check within the program but the visual output should justify test pass.\n");
    dumpPool();
    testsExecuted++;
    id2 = insertObject(200);
    printf("The buffer should contain id1,id3,id4 if every other method works properly.There is no way to check within the program but the visual output should justify test pass.\n");
    dumpPool();
    testsExecuted++;

    printf("Now testing deletion of created buffer. Retrieval of id1 should result in NULL_REF (if retrieval works properly.\n");
    destroyPool();
    //objPtr = (char *)retrieveObject(id1);
    if ((char *)retrieveObject(id1) == NULL)
    {
        printf("Success. Cannot retrieve from a deleted buffer.\n");
    }
    else
    {
        printf("Failure. Retrieved from a deleted buffer.\n");
        testsFailed++;
    }
    testsExecuted++;
}
