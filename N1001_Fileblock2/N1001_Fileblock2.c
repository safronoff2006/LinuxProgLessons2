#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define FNAME "locki.lck"

void status(struct flock *lock)

{

	printf("Status: ");

	switch (lock->l_type)

	{

	case F_UNLCK:

		printf("F_UNLCK (Блокировка снята)\n");

		break;

	case F_RDLCK:

		printf("F_RDLCK (pid: %d)(Блокировка чтения)\n", lock->l_pid);

		break;

	case F_WRLCK:

		printf("F_WRLCK (pid: %d)(Блокировка записи)\n", lock->l_pid);

		break;

	default:
		break;

	}

}

int main(int argc, char **argv)

{

	int unused __attribute__((unused));
	struct flock lock;

	int fd;

	char buffer[100];

	fd = open(FNAME, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);

	memset(&lock, 0, sizeof(struct flock));

	/*Проверим, установлена ли блокировка*/

	fcntl(fd, F_GETLK, &lock);

	if (lock.l_type == F_WRLCK || lock.l_type == F_RDLCK) {

		status(&lock);
		memset(&lock, 0, sizeof(struct flock));
		lock.l_type = F_UNLCK;

		if (fcntl(fd, F_SETLK, &lock) < 0)

			printf("Ошибка при :fcntl(fd, F_SETLK, F_UNLCK) (%s)\n",
					strerror(errno));

		else

			printf("Успешно снята блокировка: fcntl(fd, F_SETLK, F_UNLCK)\n");

	}

	status(&lock);

	unused = write(STDOUT_FILENO, "\n Введены данные: ", sizeof("\n Введены данные: "));

	while ((read(1, buffer, 100)) > 1)
		unused = write(fd, buffer, 100);

	memset(&lock, 0, sizeof(struct flock));

	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	lock.l_pid = getpid();

	if (fcntl(fd, F_SETLK, &lock) < 0)

		printf("Ошибка при: fcntl(fd, F_SETLK, F_WRLCK)(%s)\n",
				strerror(errno));
	else

		printf("Успешно: fcntl(fd, F_SETLK, F_WRLCK)\n");

	status(&lock);

	switch (fork())

	{

	case -1:
		exit(0);

	case 0:

		if (lock.l_type == F_WRLCK) {

			printf("Потомок:Невозможна запись в файл (F_WRLCK)\n");
			exit(0);

		} else {

			printf("Потомок готов к записи в файл\n");
		}

		exit(0);

	default:
		wait(NULL);
		break;

	}

	close(fd);

	return 0;

}
