
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <sjs/sjs.h>


/*
 * Open a file for low level I/O. Args:
 * - 0: path
 * - 1: flags
 * - 2: mode
 */
static duk_ret_t os_open(duk_context* ctx) {
    const char* path;
    int flags, mode, r;

    path = duk_require_string(ctx, 0);
    flags = duk_require_int(ctx, 1);
    mode = duk_require_int(ctx, 2);

    r = open(path, flags, mode);
    if (r == -1) {
        SJS_THROW_ERRNO_ERROR();
        return -42;    /* control never returns here */
    } else {
        duk_push_int(ctx, r);
        return 1;
    }
}


/*
 * Read data from a fd. Args:
 * - 0: fd
 * - 1: nread (a number or a Buffer-ish object)
 */
static duk_ret_t os_read(duk_context* ctx) {
    int fd;
    ssize_t r;
    size_t nread;
    char* buf;
    char* alloc_buf = NULL;

    fd = duk_require_int(ctx, 0);
    if (duk_is_number(ctx, 1)) {
        nread = duk_require_int(ctx, 1);
        alloc_buf = malloc(nread);
        if (!alloc_buf) {
            SJS_THROW_ERRNO_ERROR2(ENOMEM);
            return -42;    /* control never returns here */
        }
        buf = alloc_buf;
    } else {
        buf = duk_require_buffer_data(ctx, 1, &nread);
        if (buf == NULL || nread == 0) {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid buffer");
            return -42;    /* control never returns here */
        }
    }

    r = read(fd, buf, nread);
    if (r < 0) {
        free(alloc_buf);
        SJS_THROW_ERRNO_ERROR();
        return -42;    /* control never returns here */
    } else {
        if (alloc_buf) {
            /* return the string we read */
            duk_push_lstring(ctx, buf, r);
        } else {
            /* the data was written to the buffer, return how much we read */
            duk_push_int(ctx, r);
        }

        free(alloc_buf);
        return 1;
    }
}


/*
 * Write data to a fd. Args:
 * - 0: fd
 * - 1: data
 */
static duk_ret_t os_write(duk_context* ctx) {
    int fd;
    ssize_t r;
    size_t len;
    const char* buf;

    fd = duk_require_int(ctx, 0);
    if (duk_is_string(ctx, 1)) {
        buf = duk_require_lstring(ctx, 1, &len);
    } else {
        buf = duk_require_buffer_data(ctx, 1, &len);
    }

    r = write(fd, buf, len);
    if (r < 0) {
        SJS_THROW_ERRNO_ERROR();
        return -42;    /* control never returns here */
    } else {
        duk_push_int(ctx, r);
    }

    return 1;
}


/*
 * Close the fd. Args:
 * - 0: fd
 */
static duk_ret_t os_close(duk_context* ctx) {
    int fd;

    fd = duk_require_int(ctx, 0);
    close(fd);

    duk_push_undefined(ctx);
    return 1;
}


static duk_ret_t os_abort(duk_context* ctx) {
    (void) ctx;
    abort();
    return -42;    /* control never returns here */
}


static duk_ret_t os_pipe(duk_context* ctx) {
    int fds[2];
    int r;

    r = pipe(fds);
    if (r < 0) {
        SJS_THROW_ERRNO_ERROR();
        return -42;    /* control never returns here */
    } else {
        duk_push_array(ctx);
        duk_push_int(ctx, fds[0]);
    	duk_put_prop_index(ctx, -2, 0);
        duk_push_int(ctx, fds[1]);
    	duk_put_prop_index(ctx, -2, 1);
        return 1;
    }
}


static duk_ret_t os_isatty(duk_context* ctx) {
    int fd, r;

    fd = duk_require_int(ctx, 0);
    r = isatty(fd);

    duk_push_boolean(ctx, r);
    return 1;
}


static duk_ret_t os_ttyname(duk_context* ctx) {
    int fd, r;
    char path[256];

    fd = duk_require_int(ctx, 0);
    r = ttyname_r(fd, path, sizeof(path));
    if (r < 0) {
        SJS_THROW_ERRNO_ERROR();
        return -42;    /* control never returns here */
    } else {
        duk_push_string(ctx, path);
        return 1;
    }
}


static duk_ret_t os_getcwd(duk_context* ctx) {
    char* r;
    char path[8192];

    r = getcwd(path, sizeof(path));
    if (r == NULL) {
        SJS_THROW_ERRNO_ERROR();
        return -42;    /* control never returns here */
    } else {
        duk_push_string(ctx, path);
        return 1;
    }
}


#define X(name) {#name, name}
static const duk_number_list_entry module_consts[] = {
    X(O_APPEND),
    X(O_CREAT),
    X(O_EXCL),
    X(O_RDONLY),
    X(O_RDWR),
    X(O_SYNC),
    X(O_TRUNC),
    X(O_WRONLY),
    { NULL, 0.0 }
};
#undef X


static const duk_function_list_entry module_funcs[] = {
    /* name, function, nargs */
    { "open", os_open, 3 },
    { "read", os_read, 2 },
    { "write", os_write, 2 },
    { "close", os_close, 1 },
    { "abort", os_abort, 0 },
    { "pipe", os_pipe, 0 },
    { "isatty", os_isatty, 1 },
    { "ttyname", os_ttyname, 1 },
    { "getcwd", os_getcwd, 0 },
    { NULL, NULL, 0 }
};


DUK_EXTERNAL duk_ret_t sjs_mod_init(duk_context* ctx) {
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, module_funcs);
    duk_push_object(ctx);
    duk_put_number_list(ctx, -1, module_consts);
    duk_put_prop_string(ctx, -2, "c");
    return 1;
}

