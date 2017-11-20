/* Find, in parallel, those random integers that are divisible by two
   given numbers and print them to the screen. */

#include "transducers.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

void random_garbage(const void *arg, FILE *out) {
  int len = *(int*)arg;

  for (int i = 0; i < len; i++) {
    int x = rand();
    fwrite(&x, sizeof(int), 1, out);
  }
}

void divisible_by(const void *arg, FILE *out, FILE *in) {
  int d = *(int*)arg;

  int x;
  while (fread(&x, sizeof(int), 1, in) == 1) {
    int y = (x % d) == 0;
    fwrite(&y, sizeof(int), 1, out);
  }
}

void and_streams(const void *arg, FILE *out, FILE *in1, FILE *in2) {
  arg=arg; // Unused
  int x, y;
  while ((fread(&x, sizeof(int), 1, in1) == 1) &&
         (fread(&y, sizeof(int), 1, in2) == 1)) {
    int z = x && y;
    fwrite(&z, sizeof(int), 1, out);
  }
}

void filter_streams(const void *arg, FILE *out, FILE *in1, FILE *in2) {
  arg=arg; // Unused
  int x, y;
  while ((fread(&x, sizeof(int), 1, in1) == 1) &&
         (fread(&y, sizeof(int), 1, in2) == 1)) {
    if (x) {
      fwrite(&y, sizeof(int), 1, out);
    }
  }
}

void save_stream(void *arg, FILE *in) {
  /* We will be writing bytes to this location. */
  unsigned char *d = arg;

  while (fread(d, sizeof(unsigned char), 1, in) == 1) {
    d++; /* Move location ahead by one byte. */
  }
}

int main() {
  stream* s[9];

  int len = 10;
  int two = 2;
  int three = 3;

  char *output = malloc(len + 1);
  output[len] = '\0'; /* Ensure terminating NULL. */

  assert(transducers_link_source(&s[0], random_garbage, &len) == 0);
  assert(transducers_dup(&s[1], &s[2], s[0]) == 0);

  // Negative test - dup tests for stream is_read flag
  assert(transducers_dup(&s[1], &s[2], s[0]) != 0);

  assert(transducers_dup(&s[3], &s[4], s[1]) == 0);
  assert(transducers_link_1(&s[5], divisible_by, &two, s[2]) == 0);

  // Negative test - link1 tests for stream is_read flag
  assert(transducers_link_1(&s[5], divisible_by, &two, s[2]) != 0);

  assert(transducers_link_1(&s[6], divisible_by, &three, s[3]) == 0);
  assert(transducers_link_2(&s[7], and_streams, NULL, s[5], s[6]) == 0);

  // Negative test - link2 tests for stream is_read flag
  assert(transducers_link_2(&s[7], and_streams, NULL, s[5], s[6]) != 0);

  assert(transducers_link_2(&s[8], filter_streams, NULL, s[7], s[4]) == 0);
  assert(transducers_link_sink(save_stream, output, s[8]) == 0);

  // Note the sizeof()-trick to determine the number of elements in
  // the array.  This *only* works for statically allocated arrays,
  // *not* ones created by malloc().
  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[0])); i++) {
    transducers_free_stream(s[i]);
  }

  return 0;
}
