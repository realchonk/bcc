VER="0.9"

CC=cc -g -std=c99 -Og
CFLAGS += -Iinclude -Wall -Wextra -D_XOPEN_SOURCE=700 -DBCC_VER=\"$(VER)\"
