#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

extern int errno;

void status(struct flock *lock)

{

	printf("Status: ");

	switch (lock->l_type)

	{

	case F_UNLCK:
		printf("F_UNLCK\n");
		break;

	case F_RDLCK:
		printf("F_RDLCK (pid: %d)\n", lock->l_pid);
		break;

	case F_WRLCK:
		printf("F_WRLCK (pid: %d)\n", lock->l_pid);
		break;

	default:
		break;

	}

}

void writelock(char *pr, int fd, off_t from, off_t to) {

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = from;
	lock.l_whence = SEEK_SET;
	lock.l_len = to;
	lock.l_pid = getpid();

	if (fcntl(fd, F_SETLKW, &lock) < 0)

	{

		printf("%s : Ошибка fcntl(fd, F_SETLKW, F_WRLCK) (%s)\n", pr,
				strerror(errno));

		printf("\nВозник DEADLOCK (%s - proсess)!\n\n", pr);

		exit(0);

	}

	else

		printf("%s : fcntl(fd, F_SETLKW, F_WRLCK) успешно\n", pr);

	status(&lock);

}

int main()

{

	int unused __attribute__((unused));
	int fd, i;

	pid_t pid;

	if ((fd = creat("dead.txt", S_IRUSR |

	S_IWUSR | S_IRGRP | S_IROTH)) < 0)

	{

		fprintf(stderr, "Ошибка при создании......\n");

		exit(0);

	}

	/*Заполняем dead.txt 50 байтами символа X*/

	for (i = 0; i < 50; i++)

		unused = write(fd, "X", 1);

	if ((pid = fork()) < 0)

	{

		fprintf(stderr, "Ошибка fork()......\n");

		exit(0);

	}

	else if (pid == 0) //Потомок

			{

		writelock("Потомок", fd, 20, 0);

		sleep(3);

		writelock("Потомок", fd, 0, 20);

	}

	else //Родитель

	{

		writelock("Родитель", fd, 0, 20);

		sleep(1);

		writelock("Родитель", fd, 20, 0);

	}

	exit(0);

}
