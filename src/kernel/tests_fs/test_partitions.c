#include "tests_fs.h"

void test_find_free_space() {
  printf("#####test_find_free_space########\n");
  uint32_t occupiedSpace1[] = {10, 20, 30, 40, 50, 60, 70, 80};
  int numSegments1 = sizeof(occupiedSpace1) / (2*sizeof(uint32_t));
  printf("###numSegments %d\n", numSegments1);

  uint32_t diskSize1 = 100;
  uint32_t* freeSpace1;
  int numFreeSegments1;
  find_free_space(occupiedSpace1, 
                                numSegments1,
                                diskSize1,
                                &freeSpace1,
                                &numFreeSegments1);
  printf("Test Case 1:\n");
  printf("Occupied space segments:\n");
  for (int i = 0; i < numSegments1; i ++) {
      printf("Start: %u, End: %u\n", occupiedSpace1[2*i], occupiedSpace1[2*i + 1]);
  }

  printf("Free space segments:\n");
  for (int i = 0; i < numFreeSegments1; i ++) {
      printf("Start: %u, End: %u\n", freeSpace1[2*i], freeSpace1[2*i + 1]);
  }
  printf("numFreeSegments1 = %d\n", numFreeSegments1);
  assert(numFreeSegments1 == 5);
  assert(freeSpace1[0] == 0);
  assert(freeSpace1[1] == 9);
  assert(freeSpace1[2] == 21);
  assert(freeSpace1[3] == 29);
  assert(freeSpace1[4] == 41);
  assert(freeSpace1[5] == 49);
  assert(freeSpace1[6] == 61);
  assert(freeSpace1[7] == 69);
  assert(freeSpace1[8] == 81);
  assert(freeSpace1[9] == 99);

  free(freeSpace1);
  printf("\n");

  uint32_t occupiedSpace2[] = {10, 25, 30, 40, 55, 60, 65, 68};
  int numSegments2 = sizeof(occupiedSpace2) / (2*sizeof(uint32_t));

  uint32_t diskSize2 = 70;
  uint32_t* freeSpace2;
  int numFreeSegments2;
  find_free_space(occupiedSpace2, numSegments2, diskSize2, &freeSpace2, &numFreeSegments2);

  printf("Test Case 2:\n");
  printf("Occupied space segments:\n");
  for (int i = 0; i < numSegments2; i++) {
      printf("Start: %u, End: %u\n", occupiedSpace2[2*i], occupiedSpace2[2*i + 1]);
  }

  printf("Free space segments:\n");
  for (int i = 0; i < numFreeSegments2; i++) {
      printf("Start: %u, End: %u\n", freeSpace2[2*i], freeSpace2[2*i + 1]);
  }

  assert(numFreeSegments2 == 5);
  assert(freeSpace2[0] == 0);
  assert(freeSpace2[1] == 9);
  assert(freeSpace2[2] == 26);
  assert(freeSpace2[3] == 29);
  assert(freeSpace2[4] == 41);
  assert(freeSpace2[5] == 54);
  assert(freeSpace2[6] == 61);
  assert(freeSpace2[7] == 64);
  assert(freeSpace2[8] == 69);
  assert(freeSpace2[9] == 69);

  free(freeSpace2);
  printf("\n");
}

void test_is_segment_in_free_space() {
  printf("#####test_is_segment_in_free_space########\n");
  uint32_t occupiedSpace[] = {10, 20, 30, 40, 50, 60, 70, 80};
  int numSegments = sizeof(occupiedSpace) / (2*sizeof(uint32_t));

  uint32_t diskSize = 100;
  uint32_t* freeSpace;
  int numFreeSegments;
  find_free_space(occupiedSpace, numSegments, diskSize, &freeSpace, &numFreeSegments);

  printf("Test Case 1:\n");
  printf("Free space segments:\n");
  for (int i = 0; i < numFreeSegments; i++) {
      printf("Start: %u, End: %u\n", freeSpace[2*i], freeSpace[2*i + 1]);
  }

  uint32_t start1 = 25;
  uint32_t size1 = 10;
  bool isInFreeSpace1 = is_segment_in_free_space(start1, size1, freeSpace, numFreeSegments);
  printf("Segment (%u - %u) is%s in the free space.\n", start1, start1 + size1 - 1, isInFreeSpace1 ? "" : " not");

  assert(!isInFreeSpace1);

  uint32_t start2 = 42;
  uint32_t size2 = 7;
  bool isInFreeSpace2 = is_segment_in_free_space(start2, size2, freeSpace, numFreeSegments);
  printf("Segment (%u - %u) is%s in the free space.\n", start2, start2 + size2 - 1, isInFreeSpace2 ? "" : " not");

  assert(isInFreeSpace2);

  free(freeSpace);
  printf("\n");
}








