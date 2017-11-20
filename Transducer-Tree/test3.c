#include "transducers.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void string_stream(const void *arg, FILE *out) {
  fputs((const char*) arg, out);
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

void save_stream(void *arg, FILE *in) {
  /* We will be writing bytes to this location. */
  unsigned char *d = arg;

  while (fread(d, sizeof(unsigned char), 1, in) == 1) {
    d++; /* Move location ahead by one byte. */
  }
}

int main() {
  stream* s[4];

  char *input = "Hello, World!";
  char *output = malloc(strlen(input)+1);
  output[strlen(input)] = '\0'; /* Ensure terminating NULL. */

  assert(transducers_link_source(&s[0], string_stream, input) == 0);
  assert(transducers_dup(&s[2], &s[1], s[0]) == 0);
  assert(transducers_link_2(&s[3], and_streams, NULL, s[1], s[2]) == 0);
  assert(transducers_link_sink(save_stream, output, s[3]) == 0);

  free(output);
  
  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[0])); i++) {
    transducers_free_stream(s[i]);
  }

  return 0;
}
