/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Backend* backend_t;

// Obtains the backend_t of a specified path.
backend_t wasmfs_get_backend_by_path(char* path __attribute__((nonnull)));

// Obtains the backend_t of a specified fd.
backend_t wasmfs_get_backend_by_fd(int fd);

// Creates and opens a new file in the new file system under a specific backend.
// Returns the file descriptor for the new file like `open`. Returns a negative
// value on error. TODO: It might be worth returning a more specialized type
// like __wasi_fd_t here.
int wasmfs_create_file(const char* pathname __attribute__((nonnull)), mode_t mode, backend_t backend);

// Creates a new directory in the new file system under a specific backend.
// Returns 0 on success like `mkdir`, or a negative value on error.
int wasmfs_create_directory(const char* path __attribute__((nonnull)), mode_t mode, backend_t backend);

// Backend creation

// Creates a JSFile Backend in the new file system.
backend_t wasmfs_create_js_file_backend(void);

// A function that receives a void* and returns a backend.
typedef backend_t (*backend_constructor_t)(void*);

backend_t wasmfs_create_memory_backend(void);

// Note: this cannot be called on the browser main thread because it might
// deadlock while waiting for its dedicated worker thread to be spawned.
//
// Note: This function blocks on the main browser thread returning to its event
// loop. Calling this function while holding a lock the main thread is waiting
// to acquire will cause a deadlock.
//
// TODO: Add an async version of this function that will work on the main
// thread.
backend_t wasmfs_create_fetch_backend(const char* base_url __attribute__((nonnull)));

backend_t wasmfs_create_node_backend(const char* root __attribute__((nonnull)));

// Note: this cannot be called on the browser main thread because it might
// deadlock while waiting for the OPFS dedicated worker thread to be spawned.
//
// Note: This function blocks on the main browser thread returning to its event
// loop. Calling this function while holding a lock the main thread is waiting
// to acquire will cause a deadlock.
//
// TODO: Add an async version of this function that will work on the main
// thread.
backend_t wasmfs_create_opfs_backend(void);

backend_t wasmfs_create_icase_backend(backend_constructor_t create_backend,
                                      void* arg);

// Hooks

// A hook users can do to create the root directory. Overriding this allows the
// user to set a particular backend as the root. If this is not set then the
// default backend is used.
backend_t wasmfs_create_root_dir(void);

// A hook users can do to run code during WasmFS startup. This hook happens
// before file preloading, so user code could create backends and mount them,
// which would then affect in which backend the preloaded files are loaded (the
// preloaded files have paths, and so they are added to that path and whichever
// backend is present there).
void wasmfs_before_preload(void);

#ifdef __cplusplus
}
#endif
