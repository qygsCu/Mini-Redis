#自动变量 $@目标文件 $^所有依赖文件 @<第一个依赖文件

TARGET := server.out
CC := gcc
CFLAGS := -g -Wall -O0 -MMD -MP

ALL_SRC := $(wildcard src/*.c)
SRC := $(filter-out src/main.c, $(ALL_SRC))
OBJS := $(patsubst %.c, %.o, $(SRC))

#自动生成依赖文件
DEPS := $(SRC:.c=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)