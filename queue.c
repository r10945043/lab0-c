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
    char *curr_str = list_first_entry(head, element_t, list)->value;
    struct list_head *next, *curr;
    curr = head->next;
    for (next = curr->next; next != head; next = curr->next) {
        if (strcmp(list_entry(next, element_t, list)->value, curr_str) == 0) {
            element_t *victim = list_entry(next, element_t, list);
            list_del(next);
            free(victim);
        } else {
            curr = curr->next;
            curr_str = list_entry(curr, element_t, list)->value;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *slow, *fast;
    slow = fast = head->next;
    // If the number of nodes is odd, than leave the last node in its
    // original position.
    for (fast = fast->next; fast != head && fast->next != head;
         fast = fast->next->next) {
        // 1. slow points to first node, fast points to second node.
        // 2. record prev, next of current pairs.
        // 3. swap(slow, fast).
        // 4. prev->next = first, first->prev = prev,
        //    first->next = second, second->prev = first,
        //    second->next = next, next->prev = second.
        struct list_head *prev = slow->prev;
        struct list_head *next = fast->next;
        struct list_head *tmp = slow;
        slow = fast;
        fast = tmp;
        prev->next = fast;
        fast->prev = prev;
        fast->next = slow;
        slow->prev = fast;
        slow->next = next;
        next->prev = slow;
        fast = slow;
        slow = next;
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

struct list_head *q_reverse_partial(struct list_head *head)
{
    struct list_head *prev = NULL, *next;
    while (head) {
        next = head->next;
        head->next = prev;
        next->next = head;
        head->prev = next;
        if (prev)
            prev->prev = head;
        prev = head;
        head = next;
    }
    return prev;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head) || k == 1)
        return;

    struct list_head *start, *end, *prev = NULL;
    start = end = head->next;
    for (; end != head;) {
        for (int i = 0; end != head && i < k - 1; ++i, end = end->next)
            ;
        struct list_head *next_list_head = end->next;
        end->next = NULL;
        struct list_head *reversed = q_reverse_partial(start);
        // Need to record the last element of last group.
        if (start == head->next) {
            head->next = reversed;
            reversed->prev = head;
        } else {
            reversed->prev = prev;
            prev->next = reversed;
        }
        start->next = next_list_head;
        next_list_head->prev = start;
        prev = start;
        start = end = next_list_head;
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
    size_t org_queue_size = q_size(head);
    size_t del_cnt = 0;
    /* element_t->value is of char* type! */
    q_reverse(head);
    struct list_head **tmp, *cprd, *next;
    tmp = &(head)->next;
    for (; *tmp;) {
        cprd = (*tmp)->next;
        if (!cprd)
            break;
        next = cprd->next;
        if (strcmp(list_entry(*tmp, element_t, list)->value,
                   list_entry(cprd, element_t, list)->value) > 0) {
            /* cprd node needs to be deleted. */
            (*tmp)->next = next;
            if (next)
                next->prev = *tmp;
            element_t *victim = list_entry(cprd, element_t, list);
            list_del(cprd);
            free(victim);
            del_cnt++;
            continue;
        }
        *tmp = (*tmp)->next;
    }
    q_reverse(head);
    return org_queue_size - del_cnt;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    size_t org_queue_size = q_size(head);
    size_t del_cnt = 0;
    /* element_t->value is of char* type! */
    q_reverse(head);
    struct list_head **tmp, *cprd, *next;
    tmp = &(head)->next;
    for (; *tmp;) {
        cprd = (*tmp)->next;
        if (!cprd)
            break;
        next = cprd->next;
        if (strcmp(list_entry(*tmp, element_t, list)->value,
                   list_entry(cprd, element_t, list)->value) < 0) {
            /* cprd node needs to be deleted. */
            (*tmp)->next = next;
            if (next)
                next->prev = *tmp;
            element_t *victim = list_entry(cprd, element_t, list);
            list_del(cprd);
            free(victim);
            del_cnt++;
            continue;
        }
        *tmp = (*tmp)->next;
    }
    q_reverse(head);
    return org_queue_size - del_cnt;
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
