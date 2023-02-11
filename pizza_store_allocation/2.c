#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#define MAX_PIZZAS 1000
typedef struct _node
{
    int data;
    struct _node *prev;
    struct _node *next;
} node;
typedef struct my_dll
{
    node *root;
    node *last;
} dll;
int n, m, j, c, o, k;
sem_t *chefs_sem;
sem_t ovens;
sem_t drivethru;
struct timeval starttime;
dll oven_waiting;
dll chef_waiting;
dll order_waiting;
dll drivethru_waiting;
dll chefs_present;
pthread_t *chef_thereads;
pthread_t *customer_threads;
int order_number = 1;
int **pizzas;
int **ingredient_for_pizzas;
int **ingredient;
int **chefs;
int **customer;
int **pizzas_ordered;
void insert(dll *list, int x)
{
    //  if there is no node initially
    if (list->last == NULL)
    {
        list->last = (node *)malloc(sizeof(node));
        list->root->next = list->last;
        list->last->data = x;
        list->last->next = NULL;
        list->last->prev = list->root;
        return;
    }
    // if there are nodes before
    list->last->next = (node *)malloc(sizeof(node));
    list->last->next->data = x;
    list->last->next->prev = list->last->prev->next;
    list->last = list->last->next;
    list->last->next = NULL;
    return;
}
void insert_at(dll *list, int x, int i)
{
    int count = 0;
    node *temp = list->root->next;
    // if we need to add node on the first position
    if (i == 0)
    {
        // if there is no node from before
        if (list->root->next == NULL)
        {
            list->root->next = (node *)malloc(sizeof(node));
            list->last = list->root->next;
            list->root->next->data = x;
            list->root->next->next = NULL;
            list->root->next->prev = list->root;
            return;
        }
        // if there is node from before
        node *temp2 = list->root->next;
        list->root->next = (node *)malloc(sizeof(node));
        list->root->next->next = temp2;
        list->root->next->prev = list->root;
        list->root->next->next->prev = list->root->next;
        list->root->next->data = x;
        return;
    }
    // make temp point at the node prev to the required node
    while (1)
    {
        count++;
        if (count == i)
        {
            break;
        }
        temp = temp->next;
    }
    // if we insert at the end
    if (temp->next == NULL)
    {
        temp->next = (node *)malloc(sizeof(node));
        temp->next->data = x;
        temp->next->next = NULL;
        temp->next->prev = temp->prev->next;
        return;
    }
    // if we insert anywhere inside the linked list
    node *temp2 = temp->next;
    temp->next = (node *)malloc(sizeof(node));
    temp->next->next = temp2;
    temp->next->prev = temp->prev->next;
    temp->next->next->prev = temp->next;
    temp->next->data = x;
    return;
}
void delete_element(dll *list, int i)
{
    int count = 0;
    node *temp = list->root->next;
    // if we need to delete node from the first position
    if (i == 0)
    {
        // if there is no node from before
        if (temp->next == NULL)
        {
            list->last = NULL;
            free(list->root);
            list->root = NULL;
            list->root = (node *)malloc(sizeof(node));
            list->root->next = NULL;
            return;
        }
        // if there is node from before
        node *temp2 = list->root->next;
        list->root->next = list->root->next->next;
        list->root->next->prev = list->root;
        free(temp2);
        return;
    }
    // make temp point at the node prev to the required node
    while (1)
    {
        count++;
        if (count == i)
        {
            break;
        }
        temp = temp->next;
    }
    // if we delete from the end
    if (temp->next->next == NULL)
    {
        free(temp->next);
        temp->next = NULL;
        return;
    }
    // if we delete anywhere inside the linked list
    temp->next->next->prev = temp;
    node *temp2 = temp->next;
    temp->next = temp->next->next;
    free(temp2);
    return;
}
int find(dll *list, int x)
{
    int count = 0;
    node *temp = list->root->next;
    while (1)
    {
        if (temp == NULL)
        {
            return -1;
        }
        //  return 0 if the required number is not present
        if (temp->data == x)
        {
            break;
        }
        temp = temp->next;
        count++;
    }
    return count;
}
void prune(dll *list)
{
    node *temp = list->root->next;
    int count = 0, c = 1;
    while (1)
    {
        // delete the ones with odd index
        if (count % 2 == 1)
        {
            delete_element(list, c);
            c++;
        }
        count++;
        //  break when list ends
        if (temp->next == NULL)
        {
            break;
        }
        temp = temp->next;
    }
    return;
}
void print(dll *list)
{
    node *temp = list->root->next;
    while (1)
    {
        // if linked list is empty
        if (temp == NULL)
        {
            break;
        }
        printf("%d ", temp->data);
        //  break when list ends
        if (temp->next == NULL)
        {
            break;
        }
        temp = temp->next;
    }
    printf("\n");
    return;
}
void print_reverse(dll *list)
{
    node *temp = list->last;
    while (1)
    {
        printf("%d ", temp->data);
        temp = temp->prev; //  traverse the linked list in opposite direction using the prev pointer
        if (temp->prev == NULL)
        {
            break;
        }
    }
    printf("\n");
    return;
}
int get_size(dll *list)
{
    int count = 0;
    node *temp = list->last;
    while (1)
    {
        if (temp->prev == NULL)
        {
            break;
        }
        count++;
        temp = temp->prev;
    }
    return count;
}
int gettime()
{
    struct timeval currtime;
    gettimeofday(&currtime, NULL);
    return currtime.tv_sec - starttime.tv_sec;
}
void chef_arrives(int i)
{
    printf("Chef %d arrives at time %d.\n", i, gettime());
}
void chef_preparing_pizza(int i, int j, int k)
{
    printf("Chef %d is preparing the pizza %d for order %d.\n", i, j, k);
}
void chef_ingedient_shortage(int i, int j, int k)
{
    printf("Chef %d could not complete the pizza %d for order %d due to ingredient shortage.\n", i, j, k);
}
void chef_waiting_for_oven(int i, int j, int k)
{
    printf("Chef %d is waiting for oven allocation for pizza %d of order %d.\n", i, j, k);
}
void pizza_in_oven(int i, int j, int k)
{
    printf("Chef %d has put the pizza %d for order %d in oven at time %d.\n", i, j, k, gettime());
}
void pizza_out_of_oven(int i, int j, int k)
{
    printf("Chef %d has picked up the pizza %d for order %d from the oven at time %d.\n", i, j, k, gettime());
}
void chef_leaves(int i)
{
    printf("Chef %d exits at time %d.\n", i, gettime());
}
void customer_arrives(int i)
{
    printf("Customer %d arrives at time %d.\n", i, gettime());
}
void customer_drive_thru_allocation(int i)
{
    printf("Customer %d is waiting for drive-thru allocation.\n", i);
}
void customer_orders(int i, int j)
{
    printf("Customer %d enters the drive-thu zone and gives out their order %d.\n", i, j);
}
void customer_rejected(int i)
{
    printf("Customer %d rejected.\n", i);
}
void customer_waiting_for_pizza(int i)
{
    printf("Customer %d is waiting at the pickup spot.\n", i);
}
void customer_picks_pizza(int i, int j)
{
    printf("Customer %d picks up their pizza %d.\n", i, j % MAX_PIZZAS);
}
void customer_exits(int i)
{
    printf("Customer %d exists the drive through zone.\n", i);
}
void order_placed(int i, int j, int *pizzas, int k)
{
    printf("Order %d placed by customer %d has pizzas {", i, j);
    for (int f = 0; f < k - 1; f++)
    {
        printf("%d, ", pizzas[f]);
    }
    printf("%d}.\n", pizzas[k - 1]);
}
void order_allocation(int i, int j, int k)
{
    printf("Pizza %d in order %d has been assigned to chef %d.\n", i, j, k);
}
void order_awaits(int i, int j)
{
    printf("Order %d placed by customer %d awaits processing.\n", i, j);
}
void order_processing(int i, int j)
{
    printf("Order %d placed by customer %d is being processed.\n", i, j);
}
void order_processed(int i, int j)
{
    printf("Order %d placed by customer %d has been processed.\n", i, j);
}
void order_partially_processed(int i, int j)
{
    printf("Order %d placed by customer %d partially processed and remaining couldn't be.\n", i, j);
}
void order_rejected(int i, int j)
{
    printf("Order %d placed by customer %d is completely rejected.\n", i, j);
}
void *chef_function(void *args)
{
    int *chef = args;
    int serial_number = *chef;
    while (1)
    {
        if (gettime() == chefs[serial_number][0])
        {
            chef_arrives(chefs[serial_number][2]);
            insert(&chefs_present, chefs[serial_number][2]);
            break;
        }
    }
    while (1)
    {
        int value = 0;
        sem_getvalue(&chefs_sem[serial_number], &value);
        if (gettime() >= chefs[serial_number][1] && value == 1)
        {
            chef_leaves(chefs[serial_number][2]);
            int i = find(&chefs_present, chefs[serial_number][2]);
            delete_element(&chefs_present, i);
            break;
        }
    }
    return NULL;
}
void *pizza_function(void *args)
{
    int *current_pizzas = args;
    int current_pizza = *current_pizzas;
    int time_required = 0;
    insert(&chef_waiting, current_pizza);
    int chef_selected = 0;
    for (int i = 0; i < m; i++)
    {
        if (current_pizza % MAX_PIZZAS == pizzas[i][0])
        {
            time_required = pizzas[i][1];
            break;
        }
    }
    while (1)
    {
        node *temp = chefs_present.root->next;
        int flag = 0;
        for (int i = 0; i < get_size(&chefs_present); i++)
        {
            int value = 0;
            sem_getvalue(&chefs_sem[temp->data], &value);
            int f;
            for (f = 0; f < n; f++)
            {
                if (chefs[f][2] == temp->data)
                {
                    break;
                }
            }
            if (value == 1 && chefs[f][1] >= gettime() + time_required && chef_waiting.root->next->data == current_pizza)
            {
                sem_wait(&chefs_sem[temp->data]);
                chef_selected = temp->data;
                delete_element(&chef_waiting, find(&chef_waiting, current_pizza));
                chef_preparing_pizza(temp->data, current_pizza % MAX_PIZZAS, current_pizza / MAX_PIZZAS);
                chef_waiting_for_oven(temp->data, current_pizza % MAX_PIZZAS, current_pizza / MAX_PIZZAS);
                flag = 1;
                break;
            }
            temp = temp->next;
        }
        if (flag == 1)
        {
            break;
        }
    }
    sleep(3);
    insert(&oven_waiting, current_pizza);
    while (1)
    {
        sem_wait(&ovens);
        if (oven_waiting.root->next->data != current_pizza)
        {
            sem_post(&ovens);
        }
        else
        {
            pizza_in_oven(chef_selected, current_pizza % MAX_PIZZAS, current_pizza / MAX_PIZZAS);
            delete_element(&oven_waiting, find(&oven_waiting, current_pizza));
            break;
        }
    }
    sleep(time_required - 3);
    pizza_out_of_oven(chef_selected, current_pizza % MAX_PIZZAS, current_pizza / MAX_PIZZAS);
    *current_pizzas = 1;
    sem_post(&ovens);
    sem_post(&chefs_sem[chef_selected]);
    return NULL;
}
void *customer_function(void *args)
{
    int *serial_number_pointer = args;
    int serial_number = *serial_number_pointer;
    int number_of_pizzas = 0;
    dll pizzas_accepted;
    pizzas_accepted.last = NULL;
    pizzas_accepted.root = (node *)malloc(sizeof(node));
    pizzas_accepted.root->next = NULL;
    int *pizzas_accept;
    int order_number_for_this_customer = 0;
    pthread_t *order;
    int partial = 0;
    int *pizza_made;
    while (1)
    {
        if (customer[serial_number][0] == gettime())
        {
            customer_arrives(customer[serial_number][2]);
            insert(&drivethru_waiting, customer[serial_number][2]);
            break;
        }
    }
    while (1)
    {
        customer_drive_thru_allocation(customer[serial_number][2]);
        sem_wait(&drivethru);
        if (drivethru_waiting.root->next->data != customer[serial_number][2])
        {
            sem_post(&drivethru);
        }
        else
        {
            int flagggg = 0;
            order_number_for_this_customer = order_number;
            order_number++;
            customer_orders(customer[serial_number][2], order_number_for_this_customer);
            order_placed(order_number_for_this_customer, customer[serial_number][2], pizzas_ordered[serial_number], customer[serial_number][1]);
            order_awaits(order_number_for_this_customer, customer[serial_number][2]);
            int i = find(&drivethru_waiting, customer[serial_number][2]);
            delete_element(&drivethru_waiting, i);
            int rejected = 0;
            if (chefs_present.root->next == NULL)
            {
                rejected = 1;
                flagggg = 1;
            }
            if (flagggg == 0)
            {
                pizzas_accepted.last = NULL;
                pizzas_accepted.root = (node *)malloc(sizeof(node));
                pizzas_accepted.root->next = NULL;
                for (int i = 0; i < customer[serial_number][1]; i++)
                {
                    int flag = 0;
                    int q;
                    for (q = 0; q < m; q++)
                    {
                        if (pizzas[q][0] == pizzas_ordered[serial_number][i])
                        {
                            break;
                        }
                    }
                    for (int w = 0; w < pizzas[q][2]; w++)
                    {
                        for (int e = 0; e < j; e++)
                        {
                            if (ingredient[e][1] == ingredient_for_pizzas[q][w])
                            {
                                if (ingredient[e][0] != 0)
                                {
                                    continue;
                                }
                                else
                                {
                                    flag = 1;
                                    break;
                                }
                            }
                        }
                        if (flag == 1)
                        {
                            break;
                        }
                    }
                    if (flag == 1)
                    {
                        partial = 1;
                        continue;
                    }
                    for (int w = 0; w < pizzas[q][2]; w++)
                    {
                        for (int e = 0; e < j; e++)
                        {
                            if (ingredient[e][1] == ingredient_for_pizzas[q][w])
                            {
                                ingredient[e][1]--;
                            }
                        }
                    }
                    insert(&pizzas_accepted, order_number_for_this_customer * MAX_PIZZAS + pizzas[q][0]);
                }
                if (pizzas_accepted.root->next == NULL)
                {
                    rejected = 1;
                }
            }
            if (rejected == 0)
            {
                number_of_pizzas = get_size(&pizzas_accepted);
                pizzas_accept = (int *)malloc(sizeof(int) * number_of_pizzas);
                node *temp = pizzas_accepted.root->next;
                for (int i = 0; i < number_of_pizzas; i++)
                {
                    pizzas_accept[i] = temp->data;
                    temp = temp->next;
                }
                order = (pthread_t *)malloc(sizeof(pthread_t) * get_size(&pizzas_accepted));
                pizza_made = (int *)malloc(sizeof(int) * get_size(&pizzas_accepted));
                for (int i = 0; i < get_size(&pizzas_accepted); i++)
                {
                    pizza_made[i] = pizzas_accept[i];
                    pthread_create(&order[i], NULL, pizza_function, &pizza_made[i]);
                }
                for (int i = 0; i < number_of_pizzas; i++)
                {
                    delete_element(&pizzas_accepted, 0);
                }
                free(pizzas_accepted.root);
                break;
            }
            else
            {
                order_rejected(order_number_for_this_customer, customer[serial_number][2]);
                customer_rejected(customer[serial_number][2]);
                customer_exits(customer[serial_number][2]);
                sem_post(&drivethru);
                free(pizzas_accepted.root);
                return NULL;
            }
        }
    }
    order_processing(order_number_for_this_customer, customer[serial_number][2]);
    sleep(k);
    insert(&order_waiting, customer[serial_number][2]);
    while (1)
    {
        if (order_waiting.root->next->data == customer[serial_number][2])
        {
            customer_waiting_for_pizza(customer[serial_number][2]);
            break;
        }
    }
    int flag1 = 0;
    while (1)
    {
        int flag = 0;
        for (int i = 0; i < number_of_pizzas; i++)
        {
            if (pizza_made[i] == 1)
            {
                int rc = pthread_join(order[i], NULL);
                if (rc != 0)
                {
                    continue;
                }
                customer_picks_pizza(customer[serial_number][2], pizzas_accept[i]);
            }
        }
        for (int i = 0; i < number_of_pizzas; i++)
        {
            if (pizza_made[i] != 1)
            {
                flag = 1;
            }
        }
        if (flag == 0 && flag1 == 0)
        {
            if (partial == 0)
            {
                order_processed(order_number_for_this_customer, customer[serial_number][2]);
            }
            else
            {
                order_partially_processed(order_number_for_this_customer, customer[serial_number][2]);
            }
            flag1 = 1;
        }
        if (flag == 0)
        {
            break;
        }
    }
    customer_exits(customer[serial_number][2]);
    int i = find(&order_waiting, customer[serial_number][2]);
    delete_element(&order_waiting, i);
    sem_post(&drivethru);
    free(order);
    free(pizza_made);
    free(pizzas_accept);
    return NULL;
}
int main()
{
    scanf("%d %d %d %d %d %d", &n, &m, &j, &c, &o, &k);
    pizzas = (int **)malloc(sizeof(int *) * m);
    for (int i = 0; i < m; i++)
    {
        pizzas[i] = (int *)malloc(sizeof(int) * 3);
    }
    ingredient_for_pizzas = (int **)malloc(sizeof(int *) * m);
    for (int i = 0; i < m; i++)
    {
        scanf("%d %d %d", &pizzas[i][0], &pizzas[i][1], &pizzas[i][2]);
        ingredient_for_pizzas[i] = (int *)malloc(sizeof(int) * pizzas[i][2]);
        for (int k = 0; k < pizzas[i][2]; k++)
        {
            scanf("%d", &ingredient_for_pizzas[i][k]);
        }
    }
    ingredient = (int **)malloc(sizeof(int *) * j);
    for (int i = 0; i < j; i++)
    {
        ingredient[i] = (int *)malloc(sizeof(int) * 2);
    }
    for (int i = 0; i < j; i++)
    {
        scanf("%d", &ingredient[i][0]);
        ingredient[i][1] = i + 1;
    }
    chefs = (int **)malloc(sizeof(int *) * n);
    for (int i = 0; i < n; i++)
    {
        chefs[i] = (int *)malloc(sizeof(int) * 3);
    }
    for (int i = 0; i < n; i++)
    {
        scanf("%d %d", &chefs[i][0], &chefs[i][1]);
        chefs[i][2] = i + 1;
    }
    customer = (int **)malloc(sizeof(int *) * c);
    for (int i = 0; i < c; i++)
    {
        customer[i] = (int *)malloc(sizeof(int) * 3);
    }
    pizzas_ordered = (int **)malloc(sizeof(int *) * c);
    for (int i = 0; i < c; i++)
    {
        scanf("%d %d", &customer[i][0], &customer[i][1]);
        customer[i][2] = i + 1;
        pizzas_ordered[i] = (int *)malloc(sizeof(int) * customer[i][1]);
        for (int k = 0; k < customer[i][1]; k++)
        {
            scanf("%d", &pizzas_ordered[i][k]);
        }
    }
    starttime.tv_usec = 0;
    oven_waiting.last = NULL;
    oven_waiting.root = (node *)malloc(sizeof(node));
    oven_waiting.root->next = NULL;
    chef_waiting.last = NULL;
    chef_waiting.root = (node *)malloc(sizeof(node));
    chef_waiting.root->next = NULL;
    order_waiting.last = NULL;
    order_waiting.root = (node *)malloc(sizeof(node));
    order_waiting.root->next = NULL;
    drivethru_waiting.last = NULL;
    drivethru_waiting.root = (node *)malloc(sizeof(node));
    drivethru_waiting.root->next = NULL;
    chefs_present.last = NULL;
    chefs_present.root = (node *)malloc(sizeof(node));
    chefs_present.root->next = NULL;
    sem_init(&ovens, 0, o);
    sem_init(&drivethru, 0, k);
    chefs_sem = (sem_t *)malloc(sizeof(sem_t) * c);
    for (int i = 0; i < c; i++)
    {
        sem_init(&chefs_sem[i], 0, 1);
    }
    starttime.tv_sec = gettime();
    printf("Simulation Started.\n");
    int rc;
    chef_thereads = (pthread_t *)malloc(sizeof(pthread_t) * n);
    int yo[n];
    for (int i = 0; i < n; i++)
    {
        yo[i] = i;
        rc = pthread_create(&chef_thereads[i], NULL, chef_function, &yo[i]);
        assert(rc == 0);
    }
    customer_threads = (pthread_t *)malloc(sizeof(pthread_t) * c);
    int yoyo[c];
    for (int i = 0; i < c; i++)
    {
        yoyo[i] = i;
        rc = pthread_create(&customer_threads[i], NULL, customer_function, &yoyo[i]);
        assert(rc == 0);
    }
    for (int i = 0; i < n; i++)
    {
        rc = pthread_join(chef_thereads[i], NULL);
        assert(rc == 0);
    }
    for (int i = 0; i < c; i++)
    {
        rc = pthread_join(customer_threads[i], NULL);
        assert(rc == 0);
    }
    printf("Simulation Ended.\n");
    for (int i = 0; i < c; i++)
    {
        sem_destroy(&chefs_sem[i]);
    }
    sem_destroy(&ovens);
    sem_destroy(&drivethru);
    free(oven_waiting.root);
    free(chef_waiting.root);
    free(order_waiting.root);
    free(drivethru_waiting.root);
    free(chefs_present.root);
    free(chefs_sem);
    return 0;
}
/*
3 3 4 3 5 3
1 20 3 1 2 3
2 30 2 2 3
3 30 1 4
10 5 3 0
0 60 20 60 30 120
1 1 1
2 2 1 2
4 1 3
*/