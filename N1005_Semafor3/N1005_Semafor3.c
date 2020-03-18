#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "N1005_Semafor3.h"
#include <string.h>

enum {
	FALSE, TRUE
};

int main(int argc, char *argv[]) {

	int semset_id;
	int unused __attribute__((unused));
	char command[1000];
	usage(FALSE);
	printf(">");

	key_t key = ftok("/tmp", 1);

	while (TRUE) {

		char *un = fgets(command, sizeof command, stdin);
		char *pch = strtok(command, " \n");
		char *tokens[20];
		size_t n_tokens = 0;

		while (pch != NULL) {
			tokens[n_tokens] = pch;
			pch = strtok(NULL, " \n");
			n_tokens++;
		}

		if (n_tokens > 0) {

			switch (tolower(tokens[0][0])) {
			case 'c':
				if (n_tokens != 2)
					usage(FALSE);
				createsem(&semset_id, key, atoi(tokens[1]));
				break;
			case 'l':
				if (n_tokens != 2)
					usage(FALSE);
				opensem(&semset_id, key);
				locksem(semset_id, atoi(tokens[1]));
				break;
			case 'u':
				if (n_tokens != 2)
					usage(FALSE);
				opensem(&semset_id, key);
				unlocksem(semset_id, atoi(tokens[1]));
				break;
			case 'd':
				(&semset_id, key);
				removesem(semset_id);
				break;
			case 'm':
				opensem(&semset_id, key);
				changemode(semset_id, tokens[1]);
				break;

			case 's':
				show();
				break;

			case 'q':
				exit(0);

			default:
				usage(FALSE);
			}
		}
		printf("\n>");
	}

	return 0;
}

void opensem(int *sid, key_t key) {
	/* Open the semaphore set - do not create! */
	if ((*sid = semget(key, 0, 0666)) == -1) {
		printf("Множество семафоров не существует!\n");

	}
}

void createsem(int *sid, key_t key, int members) {
	int cntr;
	union semun {
		int val;
		struct semid_ds *buf;
		unsigned short int *array;
	} semopts;

	if (members > SEMMSL) {
		printf("Извините, максимальное количество семафоров установлено в  %d\n",
		SEMMSL);
		printf("\n>");
	}
	printf("Попытка создания нового  множества семафоров с %d членами\n", members);
	if ((*sid = semget(key, members, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		fprintf(stderr, "Semaphore set already exists!\n");

	}
	semopts.val = SEM_RESOURCE_MAX;
	/* Initialize all members (could be done with SETALL) */
	for (cntr = 0; cntr < members; cntr++)
		semctl(*sid, cntr, SETVAL, semopts);
}

void locksem(int sid, int member) {
	struct sembuf sem_lock = { 0, -1, IPC_NOWAIT };

	if (member < 0 || member > (get_member_count(sid) - 1)) {
		fprintf(stderr, "Номер семафора %d выходит за границы\n", member);

	}
	/* Attempt to lock the semaphore set */
	if (!getval(sid, member)) {
		fprintf(stderr, "Ресурс семафора исчерпаны (без блокировки)!\n");

	}
	sem_lock.sem_num = member;
	if ((semop(sid, &sem_lock, 1)) == -1) {
		fprintf(stderr, "Lock failed\n");

	} else
		printf("Ресурс семафора уменьшен на единицу (заблокирован)\n");

	dispval(sid, member);
}

void unlocksem(int sid, int member) {
	struct sembuf sem_unlock = { member, 1, IPC_NOWAIT };
	int semval;

	if (member < 0 || member > (get_member_count(sid) - 1)) {
		fprintf(stderr, "Номер семафора %d выходит за границы\n", member);

	}
	/* Is the semaphore set locked? */
	semval = getval(sid, member);
	if (semval == SEM_RESOURCE_MAX) {
		fprintf(stderr, "Семафор не заблокирован!\n");

	}
	sem_unlock.sem_num = member;
	/* Attempt to lock the semaphore set */
	if ((semop(sid, &sem_unlock, 1)) == -1) {
		fprintf(stderr, "Unlock failed\n");
		printf("\n>");
	} else
		printf("Ресурс семафора увеличен на единицу (разблокирован)\n");

	dispval(sid, member);
}

void removesem(int sid) {
	semctl(sid, 0, IPC_RMID, 0);
	printf("Множество семафоров удалено\n");
}

unsigned short get_member_count(int sid) {
	int rc;
	union semun {
		int val;
		struct semid_ds *buf;
		unsigned short int *array;
	} semopts;

	struct semid_ds mysemds;

	semopts.buf = &mysemds;

	rc = semctl(sid, 0, IPC_STAT, semopts);
	if (rc == -1) {
		perror("semctl");
		exit(1);
	}

	/* Return number of members in the semaphore set */
	return (semopts.buf->sem_nsems);
}

int getval(int sid, int member) {
	int semval;

	semval = semctl(sid, member, GETVAL, 0);
	return (semval);
}

void changemode(int sid, char *mode) {
	int rc;
	union semun {
		int val;
		struct semid_ds *buf;
		unsigned short int *array;
	} semopts;
	struct semid_ds mysemds;

	/* Get current values for internal data structure */
	semopts.buf = &mysemds;
	rc = semctl(sid, 0, IPC_STAT, semopts);
	if (rc == -1) {
		perror("semctl");
		exit(1);
	}
	printf("Старые права были%o\n", semopts.buf->sem_perm.mode);
	/* Change the permissions on the semaphore */
	sscanf(mode, "%ho", &semopts.buf->sem_perm.mode);
	/* Update the internal data structure */
	semctl(sid, 0, IPC_SET, semopts);
	printf("Обновлено...\n");
}

void dispval(int sid, int member) {
	int semval;

	semval = semctl(sid, member, GETVAL, 0);
	printf("Значение семафора для члена %d is %d\n", member, semval);
}

void usage(int err) {

	_IO_FILE *outstream;

	outstream = stdout;

	if (err)
		outstream = stderr;

	fprintf(outstream, "Утилита для исследования семафоров\n");
	fprintf(outstream, "\nИспользование. Введите с клавиатуры:\n");
	fprintf(outstream, "                 (c)reate <semcount>\n");
	fprintf(outstream, "                 (l)ock <sem #>\n");
	fprintf(outstream, "                 (u)nlock <sem #>\n");
	fprintf(outstream, "                 (d)elete\n");
	fprintf(outstream, "                 (m)ode <mode>\n");
	fprintf(outstream, "                 (s)how\n");
	fprintf(outstream, "                 (q)uit\n");
	fprintf(outstream, "\n");

	if (err)
		exit(1);

}

void show() {

	int sems_id;
	key_t key  = ftok("/tmp", 1);;

	if ((sems_id = semget(key, 1, 0666)) == -1) {
		printf("Множество семафоров не существует\n");

	} else
		show_sem_usage(sems_id);

}

void show_sem_usage(int sid) {
	int cntr = 0, maxsems, semval;

	maxsems = get_member_count(sid);
	while (cntr < maxsems) {
		semval = semctl(sid, cntr, GETVAL, 0);
		printf("Семафор #%d:  --> %d\n", cntr, semval);
		cntr++;
	}
}



