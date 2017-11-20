#include "transducers.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

struct stream {
    int is_read;
    pid_t exec_process_id;
    FILE *file_stream;
};

void fail_with_errmessage() {
    fprintf(stderr, "A fatal error has occured: %s \n", strerror(errno));
    exit(EXIT_FAILURE);
}

pid_t Fork() {

    pid_t pid = fork();

    if (pid < 0) {
        fail_with_errmessage();
    }
    else {
        return pid;
    }
}

stream* create_stream() {
    stream* stream_ptr = (stream*) malloc(sizeof(stream));

    if (stream_ptr == NULL) {
        fail_with_errmessage();
    }

    return stream_ptr;
}

int insert_stream_values(stream* stream_ptr, int is_read, pid_t process_id, FILE* file_stream) {

	stream_ptr->is_read = is_read;
	stream_ptr->exec_process_id = process_id;
    stream_ptr->file_stream = file_stream;
    
    return 0;
}

static int file_pipe(FILE* files[2]) {

    int fds[2];
    int r = pipe(fds);

    if (r == 0) {
        files[0] = fdopen(fds[0], "r");
        files[1] = fdopen(fds[1], "w");

        if (files[0] && files[1]) {
            return 0;
        }
        else {
            return 1;
        }
    }

    else {
        fail_with_errmessage();
    }
}


void transducers_free_stream(stream *s) {
  free(s);
}


int transducers_link_source(stream **out,
                            transducers_source s, const void *arg) {

    FILE* pipe_files[2];
    file_pipe(pipe_files);
    
    pid_t pid = Fork();

    if (pid == 0) {
        fclose(pipe_files[0]);
        s(arg, pipe_files[1]);
        
        exit(EXIT_SUCCESS);
    }
    
    else if (pid > 0) {
        fclose(pipe_files[1]);
        
        stream* write_stream_ptr = create_stream();
        insert_stream_values(write_stream_ptr, 0, pid, pipe_files[0]);
        
        *out = write_stream_ptr;

	return 0;
    }
}


int transducers_link_sink(transducers_sink s, void *arg,
                          stream *in) {

    if (in->is_read == 1) {
        return -1;
    }

    waitpid(in->exec_process_id, NULL, 0);
    s(arg, in->file_stream);

    in->is_read = 1;

    return 0;
}


int transducers_link_1(stream **out,
                       transducers_1 t, const void *arg,
                       stream* in)  {

    if (in->is_read == 1) {
        return -1;
    }

    FILE *pipe_files[2];
    file_pipe(pipe_files);

    pid_t pid = Fork();

    if (pid == 0) {
        fclose(pipe_files[0]);

        waitpid(in->exec_process_id, NULL, WCONTINUED);
        t(arg, pipe_files[1], in->file_stream);

        exit(EXIT_SUCCESS);
    }

    else if (pid > 0) {
        fclose(pipe_files[1]);
    
        stream* write_stream_ptr = create_stream();
        insert_stream_values(write_stream_ptr, 0, pid, pipe_files[0]);

	    *out = write_stream_ptr;

        in->is_read = 1;
	
        return 0;
    }
}


int transducers_link_2(stream **out,
                       transducers_2 t, const void *arg,
                       stream* in1, stream* in2) {

    if (in1->is_read == 1 || in2->is_read == 1) {
        return -1;
    }
    
    FILE *pipe_files[2];
    file_pipe(pipe_files);

    pid_t pid = Fork();

    if (pid == 0) {
        fclose(pipe_files[0]);

        waitpid(in1->exec_process_id, NULL, WCONTINUED);
        waitpid(in2->exec_process_id, NULL, WCONTINUED);
        t(arg, pipe_files[1], in1->file_stream, in2->file_stream);

        exit(EXIT_SUCCESS);
    }

    else if (pid > 0) {
        fclose(pipe_files[1]);

        stream* write_stream_ptr = create_stream();
        insert_stream_values(write_stream_ptr, 0, pid, pipe_files[0]);

        *out = write_stream_ptr;

        in1->is_read = 1;
        in2->is_read = 1;

        return 0;
    }
}

int transducers_dup(stream **out1, stream **out2,
                    stream *in) {

    if (in->is_read == 1) {
        return -1;
    }

    FILE *pipe_files1[2];
    FILE *pipe_files2[2];
    file_pipe(pipe_files1);
    file_pipe(pipe_files2);

    pid_t pid = Fork();

    if (pid == 0) {

        fclose(pipe_files1[0]);
        fclose(pipe_files2[0]);

        unsigned char c;

        while ((fread(&c, sizeof(unsigned char), 1, in->file_stream)) == 1) {

            if (fwrite(&c, sizeof(unsigned char), 1, pipe_files1[1]) != 1) {
                break;
            }
            if (fwrite(&c, sizeof(unsigned char), 1, pipe_files2[1]) != 1) {
                break;
            }
        }

        exit(EXIT_SUCCESS);
    }

    else if (pid > 0) {
        
        fclose(pipe_files1[1]);
        fclose(pipe_files2[1]);

        stream *write_stream_ptr1 = create_stream();
        insert_stream_values(write_stream_ptr1, 0, pid, pipe_files1[0]);

        stream *write_stream_ptr2 = create_stream();
        insert_stream_values(write_stream_ptr2, 0, pid, pipe_files2[0]);

        *out1 = write_stream_ptr1;
        *out2 = write_stream_ptr2;

        in->is_read = 1;

        return 0;
    }
}

