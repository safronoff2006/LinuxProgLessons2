/*
 * N1005_Semafor3.h
 *
 *  Created on: 17 мар. 2020 г.
 *      Author: safronoff2006
 */

#ifndef N1005_SEMAFOR3_H_
#define N1005_SEMAFOR3_H_

#define SEM_RESOURCE_MAX 1 /* Initial value of all semaphores */
#define SEMMSL 32

#include <sys/sem.h>

void opensem(int* , key_t );
void createsem(int* , key_t , int);
void locksem(int, int);
void unlocksem(int, int);
void removesem(int);
unsigned short get_member_count(int);
int getval(int, int);
void dispval(int, int);
void changemode(int, char*);
void usage(int);
void show(void);
int get_sem_count(int);
void show_sem_usage(int);




#endif /* N1005_SEMAFOR3_H_ */
