# Toolchain selection mirrors src/Makefile, so that CC and CROSS_COMPILE reach
# the tests as well as the library
CROSS_COMPILE?=
CC:=$(CROSS_COMPILE)$(shell sh -c 'type $(CROSS_COMPILE)$(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')

OPTIMIZATION?=-O2
DEBUG?=-g
REAL_CFLAGS=$(OPTIMIZATION) $(DEBUG) -Wall -D_GNU_SOURCE $(CFLAGS) -I. -I../src
REAL_LDFLAGS=$(LDFLAGS) -L../src -lzlog -lpthread -Wl,-rpath ../src

exe = 		\
	fifo_test	\
	test_tmp	\
	test_longlog	\
	test_buf	\
	test_bitmap	\
	test_conf	\
	test_conf2  \
	test_conf_file_type	\
	test_consumer_rules	\
	test_dzlog_conf	\
	test_hashtable	\
	test_fork	\
	test_hello	\
	test_hex	\
	test_init	\
	test_init_from_string	\
	test_level	\
	test_lock_fairness	\
	test_leak	\
	test_mdc	\
	test_multithread	\
	test_record	\
	test_rotate_lock	\
	test_pipe	\
	test_pipe2	\
	test_press_zlog		\
	test_press_zlog2	\
	test_press_write	\
	test_press_write2	\
	test_press_syslog	\
	test_syslog	\
	test_default	\
	test_profile	\
	test_category   \
	test_prompt	\
	test_enabled

all     :       $(exe)

$(exe)  :       %:%.o
	$(CC) -o $@ $^ $(REAL_LDFLAGS)

.c.o	:
	$(CC) $(REAL_CFLAGS) -o $@ -c $<

clean	:
	rm -f press.log* consumer_rules_* init_from_string_* conf_file_type* *.o $(exe)

.PHONY : clean all
