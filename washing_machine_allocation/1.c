#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
double sleep_timer = 0.001;
struct timeval current_time;
int n, m;
sem_t sem;
pthread_mutex_t student_arrives;
int left = 0;
int waiting_time = 0;
sem_t *fcfs;
void print_student_came(int i)
{
    struct timeval currtime;
    gettimeofday(&currtime, NULL);
    printf("\033[0;37m");
    printf("%ld: Student %d arrives\n", currtime.tv_sec - current_time.tv_sec, i);
    printf("\033[0m");
}
void print_student_came_washing(int i, int j)
{
    struct timeval currtime;
    gettimeofday(&currtime, NULL);
    waiting_time += currtime.tv_sec - current_time.tv_sec - j;
    printf("\033[0;32m");
    printf("%ld: Student %d starts washing\n", currtime.tv_sec - current_time.tv_sec, i);
    printf("\033[0m");
}
void print_student_leaves_washing(int i)
{
    struct timeval currtime;
    gettimeofday(&currtime, NULL);
    printf("\033[0;33m");
    printf("%ld: Student %d leaves after washing\n", currtime.tv_sec - current_time.tv_sec, i);
    printf("\033[0m");
}
void print_student_leaves(int i, int j)
{
    left++;
    struct timeval currtime;
    gettimeofday(&currtime, NULL);
    waiting_time += currtime.tv_sec - current_time.tv_sec - j;
    printf("\033[0;31m");
    printf("%ld: Student %d leaves without washing\n", currtime.tv_sec - current_time.tv_sec, i);
    printf("\033[0m");
}
void *student(void *std)
{
    int *students = std;
    while (1)
    {
        struct timeval currtime;
        gettimeofday(&currtime, NULL);
        if (currtime.tv_sec - current_time.tv_sec >= students[0])
        {
            pthread_mutex_lock(&student_arrives);
            print_student_came(students[3]);
            pthread_mutex_unlock(&student_arrives);
            break;
        }
    }
    sleep(students[3] * sleep_timer);
    struct timeval waittime;
    gettimeofday(&waittime, NULL);
    struct timespec timer;
    clock_gettime(CLOCK_REALTIME, &timer);
    timer.tv_sec += students[2];
    timer.tv_nsec = 0;
    int rc = sem_timedwait(&fcfs[0], &timer);
    if (rc == -1)
    {
        print_student_leaves(students[3], students[0]);
        return NULL;
    }
    for (int i = 1; i < n; i++)
    {
        rc = sem_timedwait(&fcfs[i], &timer);
        if (rc == -1)
        {
            sem_post(&fcfs[i - 1]);
            print_student_leaves(students[3], students[0]);
            return NULL;
        }
        sem_post(&fcfs[i - 1]);
    }
    rc = sem_timedwait(&sem, &timer);
    if (rc == -1)
    {
        sem_post(&fcfs[n - 1]);
        print_student_leaves(students[3], students[0]);
        return NULL;
    }
    sem_post(&fcfs[n - 1]);
    struct timeval waittimee;
    gettimeofday(&waittimee, NULL);
    waittimee.tv_sec -= waittime.tv_sec;
    print_student_came_washing(students[3], students[0]);
    while (1)
    {
        struct timeval currtime;
        gettimeofday(&currtime, NULL);
        if (currtime.tv_sec - current_time.tv_sec >= students[0] + students[1] + waittimee.tv_sec)
        {
            break;
        }
    }
    print_student_leaves_washing(students[3]);
    sem_post(&sem);
    return NULL;
}
int main()
{
    scanf("%d %d", &n, &m);
    sleep_timer = (double)1 / ((double)n * (double)10);
    int students[n][4];
    for (int i = 0; i < n; i++)
    {
        scanf("%d %d %d", &students[i][0], &students[i][1], &students[i][2]);
        students[i][3] = i + 1;
    }
    pthread_mutex_init(&student_arrives, NULL);
    fcfs = (sem_t *)malloc(sizeof(sem_t) * n);
    for (int i = 0; i < n; i++)
    {
        sem_init(&fcfs[i], 0, 1);
    }
    gettimeofday(&current_time, NULL);
    int rc;
    sem_init(&sem, 0, m);
    pthread_t threads[n];
    for (int i = 0; i < n; i++)
    {
        rc = pthread_create(&threads[i], NULL, student, students[i]);
        assert(rc == 0);
    }
    sem_destroy(&sem);
    pthread_mutex_destroy(&student_arrives);
    for (int i = 0; i < n; i++)
    {
        sem_destroy(&fcfs[i]);
    }
    for (int i = 0; i < n; i++)
    {
        rc = pthread_join(threads[i], NULL);
        assert(rc == 0);
    }
    free(fcfs);
    printf("%d\n%d\n", left, waiting_time);
    if ((double)left >= (double)n / 4)
    {
        printf("YES\n");
    }
    else
    {
        printf("NO\n");
    }
    return 0;
}