#define NULL ((void *) 0)
#define NUM_LINES 1000

void
insertionSort(int num_lines, char *lines[])
{
  int j;
  char *curr_line = NULL;
  for (int i = 1; i < num_lines; i++) {
    j = i - 1;
    curr_line = *(lines + i);
    while (j >= 0 && strcmp(*(lines + j), curr_line) > 0) {
      *(lines + j + 1) = *(lines + j);
      j--;
    }
    *(lines + j + 1) = curr_line;
  }
}


void 
getFlags(int argc, char *argv[], char*** flags, int* flagCount) {
    *flags = (char**)malloc(100 * sizeof(char*)); 
    if (*flags == NULL) {
        printf("Memory allocation failed\n");
        exit(1); 
    }

    *flagCount = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            (*flags)[*flagCount] = argv[i]; 
            (*flagCount)++;
            
            if (*flagCount >= 100) { 
                printf("Maximum number of flags reached (100).\n");
                break;
            }
        }
    }
}

void
printLines(int num_lines, char *lines[])
{
  for (int i = 0; i < num_lines; i++) {
    printf("%s\n", *(lines + i));
  }
}

void
freeLines(int fd, int argc, char *argv[], int num_lines, char *lines[], char **flags)
{
  for (int i = 0; i < argc; i++) {
    free(*(argv + i));
  }
  free(argv);
  argv = NULL;

  for (int i = 0; i < num_lines; i++) {
    free(*(lines + i));
  }
  free(lines);
  lines = NULL;

  free(flags);
  flags = NULL;

  close(fd);
}

//Swaps idnexes for reversing
void swap(char** a, char** b) {
    char* temp = *a;
    *a = *b;
    *b = temp;
}

void 
reverse(int num_lines, char *lines[]) {
    int start = 0;              
    int end = num_lines - 1;  

    while (start < end) {
        swap(&lines[start], &lines[end]);
        start++;
        end--;
    }
}


//True = 1
//False = 0
int
reverseCheck(char** flags, int flagCount) {
  for (int i = 0; i < flagCount; i++) {
    if (strcmp(flags[i], "-r") == 0) {
      return 1;
    }
    printf("%s\n", flags[i]);
  }
  return 0;
}