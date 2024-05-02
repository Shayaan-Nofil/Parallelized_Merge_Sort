//Muhammad Shayaan Nofil
//i21-0450
//Assignment 1

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

#define MAX_THREADS 6

typedef struct Node {
    int data;
    struct Node* next;
} Node;

void addRollNumbersToListParallel(Node** head, int* Numbers, int num) {

    for (int i = 0; i < num; i++) {

        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = Numbers[i];
        newNode->next = head[(i/(num/MAX_THREADS))];
        head[(i/(num/MAX_THREADS))] = newNode;
    }
}

void addRollNumbersToList(Node** head, int* Numbers, int num) {

    for (int i = 0; i < num; i++) {

        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = Numbers[i];
        newNode->next = *head;
        *head = newNode;
    }
}

void readRollNumbers(FILE* mfile, int* Numbers, int num) {

    for (int i = 0; i < num; i++) {

        fscanf(mfile, "%d", &Numbers[i]);
    }
}

void setAffinity(pthread_t thread, int coreId) {

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

Node* mergelist(Node* left, Node* right) {
    if (left == NULL) return right;
    if (right == NULL) return left;

    Node* result = NULL;

    if (left->data <= right->data) {

        result = left;
        result->next = mergelist(left->next, right);
    } 
    else {

        result = right;
        result->next = mergelist(left, right->next);
    }

    return result;
}

void* mergeSortParallel(void* arg) {
    Node* head = (Node*)arg;
    
    if (head == NULL || head->next == NULL) {
        return head;
    }

    Node *slow = head, *fast = head->next;

    while (fast != NULL && fast->next != NULL) {

        slow = slow->next;
        fast = fast->next;
        fast = fast->next;
    }

    Node* right = (Node*)mergeSortParallel(slow->next);
    slow->next = NULL;
    Node* left = (Node*)mergeSortParallel(head);
    return mergelist(left, right);
}

Node* mergeSort(Node* arg) {
    Node* head = (Node*)arg;
    
    if (head == NULL || head->next == NULL) {
        return head;
    }

    Node *slow = head, *fast = head->next;

    while (fast != NULL && fast->next != NULL) {

        slow = slow->next;
        fast = fast->next;
        fast = fast->next;
    }

    Node* right = (Node*)mergeSort(slow->next);
    slow->next = NULL;
    Node* left = (Node*)mergeSort(head);
    return mergelist(left, right);
}


int main() {
    int num = 100000;
    int Numbers[num];  

    FILE* mfile = fopen("data.txt", "r");
    readRollNumbers(mfile, Numbers, num);
    fclose(mfile);

    Node* head[MAX_THREADS], *head2 = NULL;
    addRollNumbersToListParallel(head, Numbers, num);
    addRollNumbersToList(&head2, Numbers, num);

    clock_t begin = clock();
    pthread_t tid[MAX_THREADS];

    for (int i = 0; i < MAX_THREADS; i++){
        Node *temp = (Node*)malloc(sizeof(Node));
        temp = head[i];
        pthread_create(&tid[i], NULL, mergeSortParallel, (void*)temp);
        setAffinity(tid[i],i);    
    }
    
    for (int i = 0; i < MAX_THREADS; i++){
        void* sorted_head;
        pthread_join(tid[i], &sorted_head);
        head[i] = (Node*)sorted_head;
    }

    Node* list = NULL;
    for (int i = 0; i < MAX_THREADS; i++){
        list = mergelist(head[i], list);
    }

    // Node* temp = list;
    // while (temp != NULL) {
    //     printf("%d, ", temp->data);
    //     temp = temp->next;
    // }

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time spend(Parallel): %f\n", time_spent);

    // Node* temp = head2;
    // while (temp != NULL) {
    //     printf("%d ", temp->data);
    //     temp = temp->next;
    // }

    head2 = mergeSort(head2);

    //Node* temp = head2;
    // while (temp != NULL) {
    //     printf("%d ", temp->data);
    //     temp = temp->next;
    // }

    clock_t end2 = clock();
    time_spent = (double)(end2 - end) / CLOCKS_PER_SEC;
    printf("time spend(serial): %f\n", time_spent);


    return 0;
}