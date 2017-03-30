#include "libc.h"
#include "stdio.h"

void runPhilo(int i, int in, int out) {
	while (1) {
		// pick up fork
		write(out, "1", 1);
		char x[1];
		if (read(in, x, 1) == 0) {
			printf("#################### Philo %d got EOF\n", i);
			return;
		}

		// eat
		printf("Philosopher %d eats.\n", i);
		for (int j = 0; j < 3; j++) {
			yield();
		}
		// printf("Philosopher %d puts down forks\n", i);

		// release fork
		write(out, "0", 1);

		//think
		printf("Philosopher %d thinks.\n", i);
		for (int j = 0; j < 3; j++) {
			yield();
		}
		for (int j = 0; j < 1000000; j++) {
			asm("nop");
		}
	}
}

void startPhilo(int i, int *fd_read, int *fd_write) {
	int fd1[2];
	if (pipe(fd1) < 0) {
		write(STDOUT_FILENO, "PError\n", 7);
		return;
	}

	int fd2[2];
	if (pipe(fd2) < 0) {
		write(STDOUT_FILENO, "PError\n", 7);
		return;
	}

	int pid = fork();
	if (pid == 0) {
		close(fd1[1]);
		close(fd2[0]);
		runPhilo(i, fd1[0], fd2[1]);
		exit(EXIT_SUCCESS);
	} else if (pid == -1) {
		write(STDOUT_FILENO, "FError\n", 7);
	} else {
		close(fd1[0]);
		close(fd2[1]);
		*fd_read  = fd2[0];
		*fd_write = fd1[1];
		fd_setblock(fd2[0], false);
	}
}

#define NUM_PHILO 16
#define NUM_FORKS 16

size_t left_fork(int philo_id) {
	return philo_id;
}

size_t right_fork(int philo_id) {
	return (philo_id + 1) % NUM_PHILO;
}

void main_philo() {
	int reads[NUM_PHILO];
	int writes[NUM_PHILO];
	int mutex[NUM_FORKS];
	int waiting[NUM_PHILO];

	for (int i = 0; i < NUM_FORKS; i++) {
		mutex[i] = -1;
	}

	for (int i = 0; i < NUM_PHILO; i++) {
		reads[i]   = 0;
		writes[i]  = 0;
		waiting[i] = 0;
		startPhilo(i, &reads[i], &writes[i]);
	}


	while (1) {
		int philo_id;
		for (philo_id = 0; philo_id < NUM_PHILO; philo_id++) {
			char x;
			int n = read(reads[philo_id], &x, 1);
			if (n <= 0) {
				// printf("[Waitor] No messages from philo %d\n", philo_id);
			} else {
				size_t left = left_fork(philo_id);
				size_t right = right_fork(philo_id);
				if (x == '1') {
					// Immediately available
					if (mutex[left] < 0 && mutex[right] < 0) {
						printf("[Waitor] Allocating fork %d and %d to philo %d\n",
								(int)left, (int)right, philo_id);
						mutex[left] = philo_id;
						mutex[right] = philo_id;
						write(writes[philo_id], "1", 1);
					} else {
						printf("[Waitor] Philo %d waiting for forks\n", philo_id);
						waiting[philo_id] = 1;
					}
				} else if (x == '0') {
					if (mutex[left] != philo_id || mutex[right] != philo_id) {
						printf("[Waitor] ##### Philo didn't have forks!\n");
					} else {
						mutex[left] = -1;
						mutex[right] = -1;

						printf("[Waitor] Philo %d put down forks\n", philo_id);

						for (int k = 0; k < NUM_PHILO; k++) {
							if (waiting[k]) {
								size_t k_left = left_fork(k);
								size_t k_right = right_fork(k);
								if (mutex[k_left] < 0 && mutex[k_right] < 0) {
									printf("[Waitor] Allocating fork %d and %d to philo %d\n",
											(int)k_left, (int)k_right, k);
									waiting[k] = 0;
									mutex[k_left] = k;
									mutex[k_right] = k;
									write(writes[k], "1", 1);
								}
							}
						}
					}
				} else {
					printf("[Waiting] ###### invalid char! ######\n");
				}
			}
		}

		yield();
	}

	exit(EXIT_SUCCESS);
}
