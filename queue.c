#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *node, *safe;
    list_for_each_safe(node, safe, head) {
        free(list_entry(node, element_t, list)->value);
        free(list_entry(node, element_t, list));
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;
    element_t *new_ele = malloc(sizeof(element_t));
    if (!new_ele)
        return false;
    /* Copy the content of *s to new_ele. */
    new_ele->value = strdup(s);
    if (!new_ele->value) {
        free(new_ele);
        return false;
    }
    list_add(&(new_ele->list), head);
    /* cppcheck-suppress memleak */
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *victim = list_first_entry(head, element_t, list);
    list_del(head->next);

    if (sp) {
        strncpy(sp, victim->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return victim;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *victim = list_last_entry(head, element_t, list);
    list_del(head->prev);
    if (sp) {
        strncpy(sp, victim->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return victim;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    struct list_head *node_, *safe_;
    size_t cnt = 0;
    list_for_each_safe(node_, safe_, head)
        cnt++;
    return cnt;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    if (list_is_singular(head)) {
        element_t *victim = list_first_entry(head, element_t, list);
        free(victim->value);
        free(victim);
        list_del_init(head->next);
        return true;
    }
    // 1 -> 2 -> 3 -> 4 -> 5 -> NULL
    // 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> NULL
    struct list_head *slow = head->next, *fast;
    for (fast = slow; fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;
    element_t *entry = list_entry(slow, element_t, list);
    list_del(slow);
    free(entry->value);
    free(entry);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    if (list_is_singular(head))
        return true;
    // Init delete_queue_head.
    struct list_head *delete_queue_head = q_new();
    element_t *curr_compared = list_first_entry(head, element_t, list);
    struct list_head *last = head;
    for (struct list_head *curr = head->next->next; curr != head;
         curr = curr->next) {
        // if del_first == true, then delete last->next node.
        bool del_first = false;
        for (; curr != head && strcmp(list_entry(curr, element_t, list)->value,
                                      curr_compared->value) == 0;) {
            del_first = true;
            struct list_head *tmp_ = curr->next;
            list_move(curr, delete_queue_head);
            curr = tmp_;
        }
        if (del_first) {
            struct list_head *tmp = last->next->next;
            list_move(last->next, delete_queue_head);
            last = tmp->prev;
        } else
            last = curr->prev;
        if (curr == head)
            break;
        curr_compared = list_entry(curr, element_t, list);
    }
    q_free(delete_queue_head);
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *first, *second, *prev;
    prev = head;
    first = second = head->next;
    for (second = first->next; first != head && first->next != head;
         first = first->next, second = first->next) {
        list_move(second, prev);
        prev = first;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *node, *safe;
    list_for_each_safe(node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head) || k == 1)
        return;

    // Call list_move(node, last) on nodes in the intervals.
    // last node will be updated at the end of each intervals.
    struct list_head *last = head;
    struct list_head *node = NULL, *safe = NULL;

    int num_intervals = q_size(head) / k;

    int cnt = 0;
    // head -> e -> d -> b -> a -> head
    list_for_each_safe(node, safe, head) {
        // node points to e, safe points to d
        // last points to head
        if (cnt == num_intervals)
            break;

        for (int i = 0; i < k; ++i) {
            list_move(node, last);
            node = safe;        // node points to b
            safe = node->next;  // safe points to a
        }

        cnt++;
        last = node->prev;
        safe = node;
    }
}

/* Merge Two Sorted Lists. */
void merge_two_sorted_list(struct list_head *head,
                           struct list_head *left,
                           struct list_head *right)
{
    if (list_empty(left) && list_empty(right))
        return;
    if (list_empty(left) && !list_empty(right))
        list_splice_tail_init(right, head);
    if (list_empty(right) && !list_empty(left))
        list_splice_tail_init(left, head);

    while (!list_empty(left) && !list_empty(right)) {
        element_t *left_entry = list_first_entry(left, element_t, list);
        element_t *right_entry = list_first_entry(right, element_t, list);
        if (strcmp(left_entry->value, right_entry->value) <= 0)
            list_move_tail(left->next, head); /* Add left to the first. */
        else
            list_move_tail(right->next, head);
    }
    /* Transplant the remaining list. */
    if (!list_empty(left))
        list_splice_tail_init(left, head);
    if (!list_empty(right))
        list_splice_tail_init(right, head);
}

/* Recursive Merge Sort Implementation.
   Default is sorting in ascending order. */
void merge_sort(struct list_head *head)
{
    // head -> b -> r -> a -> head
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    LIST_HEAD(left);
    LIST_HEAD(right);

    /* Cut the list to two halves. */
    struct list_head *slow, *fast;
    slow = fast = head->next;
    for (fast = fast->next; fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;

    list_cut_position(&left, head, slow);
    list_splice_tail_init(head, &right);
    merge_sort(&left);
    merge_sort(&right);
    merge_two_sorted_list(head, &left, &right);
}

#define IS_GREATER(oper)                             \
    strcmp(list_entry(node, element_t, list)->value, \
           list_entry(pivot_node, element_t, list)->value) oper 0

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    merge_sort(head);
    if (descend)
        q_reverse(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;

    q_reverse(head);

    char *max_value = NULL;
    struct list_head *node, *safe;

    list_for_each_safe(node, safe, head) {
        element_t *e = list_entry(node, element_t, list);
        if (!max_value || strcmp(e->value, max_value) < 0)
            max_value = e->value;
        else {
            list_del(node);
            free(e->value);
            free(e);
        }
    }

    q_reverse(head);
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;

    q_reverse(head);

    char *max_value = NULL;
    struct list_head *node, *safe;

    list_for_each_safe(node, safe, head) {
        element_t *e = list_entry(node, element_t, list);
        if (!max_value || strcmp(e->value, max_value) > 0)
            max_value = e->value;
        else {
            list_del(node);
            free(e->value);
            free(e);
        }
    }

    q_reverse(head);
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return list_entry(head->next, queue_contex_t, chain)->size;
    struct list_head *first_queue =
        list_entry(head->next, queue_contex_t, chain)->q;

    struct list_head *queue_ptr = head->next->next;
    while (queue_ptr != head) {
        struct list_head *curr_queue =
            list_entry(queue_ptr, queue_contex_t, chain)->q;
        list_splice_init(curr_queue, first_queue);
        list_entry(queue_ptr, queue_contex_t, chain)->size = 0;
        queue_ptr = queue_ptr->next;
    }
    q_sort(first_queue, descend);
    list_entry(first_queue, queue_contex_t, chain)->size = q_size(first_queue);
    return list_entry(first_queue, queue_contex_t, chain)->size;
}
