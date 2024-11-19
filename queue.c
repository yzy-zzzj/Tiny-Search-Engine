#include <stdlib.h>
#include "queue.h"

typedef struct node {
    void *data;
    struct node *next;
} node_t;

struct queue {
    node_t *front;
    node_t *rear;
    int size;
};

queue_t* qopen(void) {
    struct queue *qp = (struct queue *)malloc(sizeof(struct queue));
    if (qp == NULL) {
        return NULL; // Memory allocation failed
    }
    qp->front = NULL;
    qp->rear = NULL;
    qp->size = 0;
    return (queue_t *)qp;
}

void qclose(queue_t *qp) {
    struct queue *q = (struct queue *)qp;
    node_t *current = q->front;
    node_t *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(q);
}

int32_t qput(queue_t *qp, void *elementp) {
    struct queue *q = (struct queue *)qp;
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL) {
        return -1; // Memory allocation failed
    }
    new_node->data = elementp;
    new_node->next = NULL;
    if (q->rear == NULL) {
        q->front = new_node;
    } else {
        q->rear->next = new_node;
    }
    q->rear = new_node;
    q->size++;
    return 0;
}

void* qget(queue_t *qp) {
    struct queue *q = (struct queue *)qp;
    if (q->front == NULL) {
        return NULL; // Queue is empty
    }
    node_t *front_node = q->front;
    void *data = front_node->data;
    q->front = front_node->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(front_node);
    q->size--;
    return data;
}

void qapply(queue_t *qp, void (*fn)(void* elementp)) {
    struct queue *q = (struct queue *)qp;
    node_t *current = q->front;
    while (current != NULL) {
        fn(current->data);
        current = current->next;
    }
}

void* qsearch(queue_t *qp, bool (*searchfn)(void* elementp, const void* keyp), const void* skeyp) {
    struct queue *q = (struct queue *)qp;
    node_t *current = q->front;
    while (current != NULL) {
        if (searchfn(current->data, skeyp)) {
            return current->data;
        }
        current = current->next;
    }
    return NULL;
}

void* qremove(queue_t *qp, bool (*searchfn)(void* elementp, const void* keyp), const void* skeyp) {
    struct queue *q = (struct queue *)qp;
    node_t *current = q->front;
    node_t *prev = NULL;
    while (current != NULL) {
        if (searchfn(current->data, skeyp)) {
            if (prev == NULL) {
                q->front = current->next;
            } else {
                prev->next = current->next;
            }
            if (current->next == NULL) {
                q->rear = prev;
            }
            void *data = current->data;
            free(current);
            q->size--;
            return data;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

void qconcat(queue_t *q1p, queue_t *q2p) {
    struct queue *q1 = (struct queue *)q1p;
    struct queue *q2 = (struct queue *)q2p;
    if (q1->rear == NULL) {
        q1->front = q2->front;
    } else {
        q1->rear->next = q2->front;
    }
    if (q2->rear != NULL) {
        q1->rear = q2->rear;
    }
    q1->size += q2->size;
    free(q2);
}
