# Name: Jack Yohn
# 1000#: 1002056627
# Lang/Ver: Python 3.11
# OS: Linux (Omega server)
#
# Uses a recursive function with os.scandir.
# Skips symlinks to avoid infinite loops. Prints only the byte total.

import os
import sys

def sum_dir(path: str) -> int:
    """Recursively sum regular file sizes under 'path'."""
    total = 0
    try:
        # If it's a symlink, skip it entirely
        if os.path.islink(path):
            return 0
        # If it's a file, return its size via lstat (do not follow links)
        st = os.lstat(path)
        if os.path.isfile(path):
            return st.st_size
        # If it's a directory, recurse into entries
        if os.path.isdir(path):
            with os.scandir(path) as it:
                for entry in it:
                    # Skip "." and ".." implicitly; scandir won’t return them explicitly
                    child_path = entry.path
                    try:
                        # Use lstat to avoid following symlinks
                        if entry.is_symlink():
                            continue
                        if entry.is_file(follow_symlinks=False):
                            total += os.lstat(child_path).st_size
                        elif entry.is_dir(follow_symlinks=False):
                            total += sum_dir(child_path)  # recursion
                        # Other types ignored
                    except Exception:
                        # Skip unreadable entries
                        pass
        # If neither file nor directory, ignore
    except Exception:
        # Skip paths we can't stat/read
        return 0
    return total

if __name__ == "__main__":
    # Always start at current directory per assignment
    print(sum_dir("."))
