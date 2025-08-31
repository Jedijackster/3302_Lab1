/* Name: Jack Yohn
 * 1000#: 1002056627
 * Lang/Ver: C17 (GCC)
 * OS: Linux (Omega server)
 *
 * Recursive Directory Space – C (Procedural)
 * Walk the current directory "." and sum sizes of regular files.
 * Excludes symlinks (to avoid loops / double-count).
 * Prints only the integer total in bytes.
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

static int is_dot_or_dotdot(const char *name) {
    return (strcmp(name, ".") == 0 || strcmp(name, "..") == 0);
}

/* join path safely into out; returns 0 on success, -1 on overflow */
static int join_path(const char *base, const char *name, char *out, size_t outsz) {
    size_t blen = strlen(base);
    int need_sep = (blen > 0 && base[blen - 1] != '/');
    int n = snprintf(out, outsz, "%s%s%s", base, need_sep ? "/" : "", name);
    return (n >= 0 && (size_t)n < outsz) ? 0 : -1;
}

/* Recursive function you wrote (required): returns sum of file sizes under 'path'. */
static unsigned long long sum_dir(const char *path) {
    unsigned long long total = 0ULL;

    DIR *dir = opendir(path);
    if (!dir) {
        // Not a directory or cannot open—if it's a file, count it below by lstat
        struct stat st;
        if (lstat(path, &st) == 0) {
            // Skip symlinks entirely
            if (S_ISREG(st.st_mode)) {
                total += (unsigned long long)st.st_size;
            }
        }
        return total;
    }

    struct dirent *ent;
    char child[PATH_MAX];

    while ((ent = readdir(dir)) != NULL) {
        if (is_dot_or_dotdot(ent->d_name)) continue;
        if (join_path(path, ent->d_name, child, sizeof(child)) != 0) {
            // Path too long; skip this entry safely.
            continue;
        }

        struct stat st;
        if (lstat(child, &st) != 0) {
            // Could not stat; skip unreadable entries.
            continue;
        }

        if (S_ISLNK(st.st_mode)) {
            // Skip symlinks to avoid cycles/double-counting.
            continue;
        } else if (S_ISREG(st.st_mode)) {
            total += (unsigned long long)st.st_size;
        } else if (S_ISDIR(st.st_mode)) {
            total += sum_dir(child); // recursion
        } else {
            // Other file types (sockets, fifos, etc.) ignored
        }
    }

    closedir(dir);
    return total;
}

int main(void) {
    unsigned long long total = sum_dir(".");
    // Print ONLY the integer (no commas/text)
    printf("%llu\n", total);
    return 0;
}
