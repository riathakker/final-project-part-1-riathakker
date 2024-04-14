#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include "gatorgit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (gatorgit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* gatorgit init
 *
 * - Create .gatorgit directory
 * - Create empty .gatorgit/.index file
 * - Create .gatorgit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int gatorgit_init(void) {
  fs_mkdir(".gatorgit");

  FILE* findex = fopen(".gatorgit/.index", "w");
  fclose(findex);
  
  write_string_to_file(".gatorgit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}


/* gatorgit add <filename>
 * 
 * - Append filename to list in .gatorgit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int gatorgit_add(const char* filename) {
  FILE* findex = fopen(".gatorgit/.index", "r");
  FILE *fnewindex = fopen(".gatorgit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".gatorgit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".gatorgit/.newindex", ".gatorgit/.index");

  return 0;
}


/* gatorgit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int gatorgit_rm(const char* filename) {
  
  // create new index and also original file pointer
  FILE* filePointer = fopen(".gatorgit/.index", "r");
  FILE *filePointer2 = fopen(".gatorgit/.newindex", "w");

  char currentFile[FILENAME_SIZE];
  int returnValue = 1;
  // go through the filenames 
  while(fgets(currentFile, FILENAME_SIZE, filePointer)) {
    // copy them all to newindex except for the one that is equal
    strtok(currentFile, "\n");
    if (strcmp(currentFile, filename) == 0) {
      returnValue = 0;
    } else {
      fprintf(filePointer2, "%s\n", currentFile);
    }
  }
  // if a match is never found, let the return value be 1 and print error
  if (returnValue == 1) {
    fprintf(stderr, "%s %s %s", "ERROR: File", filename, "not tracked\n");
  }
  // move the contents of newindex to index
  fs_mv(".gatorgit/.newindex", ".gatorgit/.index");
  return returnValue;
}

/* gatorgit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_gator = "GOLDEN GATOR!";

int is_commit_msg_ok(const char* msg) {
  // iterate through the msg; if the first character of golden gator is found check that the rest are equal as well
  while (*msg != '\0') {
    if (*msg == go_gator[0]) {
      int i = 1;
      while (go_gator[i] != '\0') {
        if (msg[i] != go_gator[i]) {
          return 1;
        }
        i++;
      }
      return 0;
    }
  }
  return 0;
}

void increment(char* previousID) {
  // increment
  for (int i = strlen(previousID) - 1; i >= 0; i--) {
    if (previousID[i] == 'c') {
      previousID[i] = 's';
      return;
    }
    if (previousID[i] == 's') {
      previousID[i] = '2';
      return;
    }
    if (previousID[i] == '2') {
      previousID[i] = '5';
      return;
    }
    if (previousID[i] == '5') {
      previousID[i] = '6';
      return;
    }
    if (previousID[i] == '6') {
      previousID[i] = 'c';
    }
  }
}

void next_commit_id(char* commit_id) {
  // read from prev
  FILE *filePointer = fopen(".gatorgit/.prev", "r");
  char previousID[COMMIT_ID_SIZE];
  fgets(previousID, COMMIT_ID_SIZE, filePointer);
  fclose(filePointer);

  // if prev is empty then this is the first commit and we can populate with an ID of 0s
  if (previousID[0] == '\0') {
    for (int j = 0; j < COMMIT_ID_SIZE - 1; j++) {
      commit_id[j] = '0';
    }
    previousID[COMMIT_ID_SIZE - 1] = '\0';
  } 
  
  if (strcmp("0000000000000000000000000000000000000000", previousID) == 0) {
    strcpy(commit_id, "cccccccccccccccccccccccccccccccccccccccc");
    return;
  }

  increment(previousID);

  strcpy(commit_id, previousID);
}

int gatorgit_commit(const char* msg) {
  if (is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_gator);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  char commit_dir[FILENAME_SIZE] = ".gatorgit/";

 // get ID and then make directory using ID
  next_commit_id(commit_id);
  strcat(commit_dir, commit_id);
  fs_mkdir(commit_dir);

  // copy index, prev, message
  char newIndex[FILENAME_SIZE];
  strcpy(newIndex, commit_dir);
  strcat(newIndex, "/.index");
  FILE *indexPointer = fopen(newIndex, "w");
  char newPrev[FILENAME_SIZE];
  strcpy(newPrev, commit_dir);
  strcat(newPrev, "/.prev");
  FILE *prevPointer = fopen(newPrev, "w");
  fclose(indexPointer);
  fclose(prevPointer);
  fs_cp(".gatorgit/.index", newIndex);
  fs_cp(".gatorgit/.prev", newPrev);

  write_string_to_file(".gatorgit/.prev", commit_id);

  FILE *oldIndexPointer = fopen(".gatorgit/.index", "r");
  char currentFile[FILENAME_SIZE];
  while (fgets(currentFile, FILENAME_SIZE, oldIndexPointer)) {
    strtok(currentFile, "\n");
    char fileDest[FILENAME_SIZE];
    strcpy(fileDest, commit_dir);
    strcat(fileDest, "/");
    strcat(fileDest, currentFile);
    FILE *newPointer = fopen(fileDest, "w");
    fs_cp(currentFile, fileDest);
    fclose(newPointer);
  }
  fclose(oldIndexPointer);

  // create msg file
  char newMsg[FILENAME_SIZE];
  strcpy(newMsg, commit_dir);
  strcat(newMsg, "/.msg");
  FILE *msgPointer = fopen(newMsg, "w");
  fclose(msgPointer);
  write_string_to_file(newMsg, msg);

  return 0;
}

int gatorgit_status() {
  /* YOUR CODE HERE */

  FILE *filePointer;
  filePointer = fopen(".gatorgit/.index", "r");
  char currentFile[FILENAME_SIZE];
  fprintf(stdout, "Tracked files: \n\n");
  int numElems = 0;
  // for each file in .gatorgit/.index:
  while (fgets(currentFile, FILENAME_SIZE, filePointer)) {
    fprintf(stdout, "  %s", currentFile);
    numElems++;
  }
  if (numElems != 1) {
    fprintf(stdout, "\n %d %s", numElems, "files total\n");
  } else {
    fprintf(stdout, "\n %d %s", numElems, "file total\n");
  }
  return 0;
}
