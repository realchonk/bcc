VER="0.8"

CC=cc -g -std=c99 -Og
CFLAGS += -Iinclude -Wall -Wextra -D_XOPEN_SOURCE=700 -DBCC_VER=\"$(VER)\"
