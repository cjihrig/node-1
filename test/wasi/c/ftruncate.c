#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#define BASE_DIR "/tmp"
#define OUTPUT_DIR BASE_DIR "/testdir"
#define PATH OUTPUT_DIR "/output.txt"
#define SIZE 500
#define TRUNC_SIZE (SIZE / 2)

int main(void) {
  struct stat st;
  int fd;

  assert(0 == mkdir(OUTPUT_DIR, 0755));
  fd = open(PATH, O_CREAT | O_WRONLY, 0666);
  assert(fd != -1);

  /* Verify that the file is initially empty. */
  assert(0 == fstat(fd, &st));
  assert(st.st_size == 0);
  assert(0 == lseek(fd, 0, SEEK_CUR));

  /* Increase the file size using ftruncate(). */
  assert(0 == ftruncate(fd, SIZE));
  assert(0 == fstat(fd, &st));
  assert(st.st_size == SIZE);
  assert(0 == lseek(fd, 0, SEEK_CUR));

  /* Truncate the file using ftruncate(). */
  assert(0 == ftruncate(fd, TRUNC_SIZE));
  assert(0 == fstat(fd, &st));
  assert(st.st_size == TRUNC_SIZE);
  assert(0 == lseek(fd, 0, SEEK_CUR));

  return 0;
}
