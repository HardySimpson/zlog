/* Copyright (c) Hardy Simpson
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <syslog.h>

int work(long loop_count)
{
	while(loop_count-- > 0) {
		syslog(LOG_INFO, "loglog");
	}
	return 0;
}


int test(long process_count, long loop_count)
{
	long i;
	pid_t pid;

	for (i = 0; i < process_count; i++) {
		pid = fork();
		if (pid < 0) {
			printf("fork fail\n");
		} else if(pid == 0) {
			work(loop_count);
			return 0;
		}
	}

	for (i = 0; i < process_count; i++) {
		pid = wait(NULL);
	}

	return 0;
}


int main(int argc, char** argv)
{

	if (argc != 3) {
		fprintf(stderr, "test nprocess nloop");
		exit(1);
	}

	openlog(NULL, LOG_NDELAY|LOG_NOWAIT|LOG_PID, LOG_LOCAL0);

	test(atol(argv[1]), atol(argv[2]));

	
	return 0;
}
