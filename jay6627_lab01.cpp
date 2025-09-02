/* Name: Jack Yohn
 * 1000#: 1002056627
 * Lang/Ver: C++17
 * OS: Linux (Omega server)
 *
 * Recursively sum sizes of regular files starting at ".".
 * Skips symlinks to avoid loops and double-counting.
 * Prints only the integer byte total (no extra text).
 */

#define _XOPEN_SOURCE 700
#include <iostream>
#include <string>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <limits.h>
#include <stdint.h>

static bool isDotOrDotDot(const char* name) {
    return std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0;
}

static bool joinPath(const std::string& base, const char* name, std::string& out) {
    // Safe path join without exceeding PATH_MAX when converted to C string
    out.clear();
    if (!base.empty() && base.back() == '/')
        out = base + name;
    else if (base.empty())
        out = name;
    else
        out = base + "/" + name;

    // Rough guard: ensure it won’t overflow typical C buffers, though we use std::string
    return out.size() < PATH_MAX;
}

// Recursive function: Returns total size (bytes) of regular files under 'path'.

static uint64_t sumDir(const std::string& path) {
    uint64_t total = 0;

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        // If not a directory or unreadable, try to count it as a single file.
        struct stat st;
        if (lstat(path.c_str(), &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                total += static_cast<uint64_t>(st.st_size);
            }
        }
        return total;
    }

    // Ensure directory stream gets closed even if we early-return
    struct DirCloser {
        DIR* d;
        ~DirCloser() { if (d) closedir(d); }
    } closer{dir};

    struct dirent* ent;
    std::string child;

    while ((ent = readdir(dir)) != nullptr) {
        if (isDotOrDotDot(ent->d_name)) continue;

        if (!joinPath(path, ent->d_name, child)) {
            // Path too long; skip safely
            continue;
        }

        struct stat st;
        if (lstat(child.c_str(), &st) != 0) {
            // Unreadable entry, skip
            continue;
        }

        if (S_ISLNK(st.st_mode)) {
            // Skip symlinks entirely
            continue;
        } else if (S_ISREG(st.st_mode)) {
            total += static_cast<uint64_t>(st.st_size);
        } else if (S_ISDIR(st.st_mode)) {
            total += sumDir(child); // recursion
        }
    }

    return total;
}

int main() {
    uint64_t total = sumDir(".");
    std::cout << total << '\n'; // print ONLY the integer
    return 0;
}
