#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/sortfuncs.c"

//
// wrapper so that it's OK if main() does not call exit().
//
void
_main()
{
  extern int main();
  main();
  exit(0);
}

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

/**
 * @brief This function reads buffer_size number of characters from any input source
 * @param file_descriptor Identifies opened file
 * @param *buffer Buffer where we will write data into
 * @param buffer_size Size of the buffer
 * @return Total number of characters read or -1 for error
*/
int
fgets(int file_descriptor, char *buffer, uint buffer_size)
{
  int total_chars_read = 0;
  int curr_chars_read = 0;
  int i;
  char curr_char;

  for (i = 0; i + 1 < buffer_size; i++) { // i + 1 to ensure space for the null byte
    curr_chars_read = read(file_descriptor, &curr_char, 1);
    if (curr_chars_read < 1) break;

    total_chars_read++;
    if (curr_char == '\n' || curr_char == '\r') break;

    *(buffer + i) = curr_char;
  }

  *(buffer + i) = '\0';
  if (curr_chars_read == -1) return -1;

  return total_chars_read;
}

char*
gets(char *buf, int max)
{
  fgets(0, buf, max);
  return buf;
}

/**
 * @brief This function reads one whole line at a time. Will write file contents to
 * a buffer and print the buffer. Will allocate more memory for the buffer if needed.
 * @param **buffer Pointer to buffer where we will write to
 * @param *buffer_size Size of buffer
 * @param file_descriptor Identifies open file
 * @return Number of characters read or -1 for error
*/
int
getline(char **buffer, uint *buffer_size, int file_descriptor)
{
  int bytes_read = 0;
  int total_bytes_read = 0;
  char last_char;

  if (*buffer == NULL || *buffer_size == 0) {
    *buffer_size = 16;
    *buffer = (char *) malloc(*buffer_size);
    if (*buffer == NULL) return -1;
  }

  bytes_read = fgets(file_descriptor, *buffer, *buffer_size);
  while (bytes_read > 0) {
    total_bytes_read += bytes_read;
    last_char = *(*(buffer) + *buffer_size - 2); // Last readable char -> last char is null byte
    if (last_char == '\n' || last_char == '\0') return total_bytes_read;

    /* Double size of buffer */
    *buffer_size *= 2;
    char *resized_buffer = (char *) malloc(*buffer_size);
    if (resized_buffer == NULL) return -1;

    memcpy(resized_buffer, *buffer, total_bytes_read);
    free(*buffer);

    /* Start writing to end of current buffer. Only write to remaining number of bytes in buffer -> prevents buffer overflow */
    bytes_read = fgets(file_descriptor, resized_buffer + total_bytes_read, *buffer_size - total_bytes_read);
    *buffer = resized_buffer;
  }

  return bytes_read; // Will always be 0 here -> we only get here if while loop condition fails
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
    while(n-- > 0)
      *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while(n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}

int
memcmp(const void *s1, const void *s2, uint n)
{
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

void *
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}

/**
 * @author Shyon Ghahghahi
 * @author Amin Joseph
 * @brief Parent function that handles all input. Will call various
 *        functions in sortfuncs.c to delegate tasks such as sorting,
 *        printing, freeing, etc.
 * @param fd File descriptor that determines whether we're reading from a file or stdin (via a pipe)
 * @param num_flags Number of flags passed by the user
 * @param *flags[] Flags passed by the user
 * @return 0 for success, 1 for failure
*/
int
sort(int fd, int num_flags, char *flags[])
{
  FlagStruct fs = processFlags(num_flags, flags);

  if (fs.help_flag) {
    help();
    return 0;
  }

  char *line = NULL;
  char **lines = (char **) malloc(MAX_LINES * sizeof(char *));
  if (lines == NULL) return errorInt();

  uint buffer_size = 128;
  int num_lines = 0;
  int len;

  while (1) {
    if ((len = getline(&line, &buffer_size, fd)) <= 0) break;
    *(lines + num_lines) = (char *) malloc((len + 1) * sizeof(char));
    if (*(lines + num_lines) == NULL) return errorInt();

    strcpy(*(lines + num_lines++), line);
  }

  /* 
    Setting flag precedence:

      -reverse & -unique sorted frist if called in order
      to maintain proper data manipulation in case other
      flags are called

      Precedence is set by the order of flag booleans,
      pretty ugly but it works

      Regular insertion sort is called if no flags are 
      present
  */
  if (num_flags <= 2 && (fs.unq_flag || fs.rev_flag)) insertionSortOrig(num_lines, lines);

  if (fs.ig_blanks_flag)  ignoreBlanks(num_lines, lines);
  if (fs.ig_case_flag)    ignoreCase(num_lines, lines);
  if (fs.num_flag)        numeric(num_lines, lines);
  if (fs.rev_flag)        reverse(num_lines, lines);
  if (fs.unq_flag)        unique(&num_lines, lines);

  if (num_flags == 0) insertionSortOrig(num_lines, lines);
  printLines(num_lines, lines);
  freeLines(num_lines, lines);
  freeLines(num_flags, flags);

  return 0;
}