#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h> // terminal ui

typedef struct block
{
    size_t size;
    int free;
    struct block *next;
} block_t;

#define BLOCK_SIZE sizeof(block_t)

static block_t *head = NULL;

void my_free(void *ptr);
void *my_malloc(size_t size);
void *my_realloc(void *ptr, size_t new_size);
void draw_heap(int start_row);
char check_free_mem(void **ptrs, int count, char *freed_list)

    int main()
{

    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // used
    init_pair(2, COLOR_RED, COLOR_BLACK);   // free
    cbreak();                               // read input without enter
    echo();
    // an array that will be malloced each time a new element gets added, it will consist of 1 and 0s where each one reprisents a true or false statement.By that
    // we will print on the screen the numbers that can be freed or realloced.
    // if I do free and then malloc back I into 3 memories I will get 0-5 which is wrong.

    int choice;
    void *p;
    void **ptrs = NULL;
    int capacity = 0;
    int count = 0;
    int i = 0;
    int free_array[100]; // contains the freed
    int sum_count = 0;
    while (1)
    {
        mvprintw(0, 0, "| OPTIONS |                              ");
        mvprintw(1, 0, "1. malloc()                              ");
        mvprintw(2, 0, "2. free()                                ");
        mvprintw(3, 0, "3. realloc()                             ");
        mvprintw(4, 0, "4. exit                                  ");
        mvprintw(5, 0, "> ");
        clrtoeol();
        refresh();
        scanw("%d", &choice);

        if (choice == 1)
        {
            size_t size;
            mvprintw(8, 0, "                                     ");
            mvprintw(9, 0, "                                     ");
            mvprintw(7, 0, "Give the size to malloc: ");
            clrtoeol();
            refresh();
            scanw("%zu", &size);
            p = my_malloc(size);
            mvprintw(7, 0, "New pointer available: %p          ", p);

            int reused = 0;
            // looking for free spots to reuse before adding a new one
            for (int j = 0; j < count; j++)
            {
                if (ptrs[j] == NULL)
                {
                    ptrs[j] = p;
                    reused = 1;
                    break;
                }
            }

            if (!reused)
            {
                if (count == capacity)
                {
                    capacity = (capacity == 0) ? 4 : capacity * 2;
                    ptrs = realloc(ptrs, capacity * sizeof(void *));
                }
                ptrs[count++] = p;
            }
        }
        else if (choice == 2)
        {
            mvprintw(8, 0, "                                            ");
            mvprintw(9, 0, "                                            ");

            if (count == 0)
            {
                mvprintw(7, 0, "No memory available.                ");
                refresh();
            }
            else
            {
                int idx;
                mvprintw(7, 0, "Which slot to free (0 to %d):                 ", count - 1);
                clrtoeol();
                char freed_list[100] = "Already free: ";
                int any_free = 0;
                for (int i = 0; i < count; i++)
                {
                    if (ptrs[i] == NULL)
                    {
                        char num[16];
                        snprintf(num, sizeof(num), "%d ", i);
                        strcat(freed_list, num);
                        any_free = 1;
                    }
                }

                mvprintw(8, 0, "                                                                        ");
                if (any_free)
                    mvprintw(8, 0, "%s", freed_list);

                refresh();
                scanw("%d", &idx);

                while (idx < 0 || idx >= count || ptrs[idx] == NULL)
                {
                    mvprintw(9, 0, "Invalid or already-freed slot,give a new slot        ");
                    refresh();
                    scanw("%d", &idx);
                }

                if (idx == ptrs[count])
                    mvprintw(8, 0, "%c", check_free_mem(ptrs, count, freed_list));
                mvprintw(7, 0, "slot is free                                        ");
                // mvprintw(8, 0, "                                                    ");
                mvprintw(9, 0, "                                                    ");
                my_free(ptrs[idx]);
                ptrs[idx] = NULL;
                if (idx == count - 1)
                    count--;
            }
        }
        else if (choice == 3)
        {
            mvprintw(8, 0, "                                       ");
            mvprintw(9, 0, "                                       ");
            int idx = -1;
            size_t new_size = 0;

            if (count > 0)
            {
                mvprintw(7, 0, "Which slot to realloc (0-%d): ", count - 1);
                clrtoeol();
                refresh();
                scanw("%d", &idx);
                while (idx > count - 1)
                {
                    mvprintw(8, 0, " ");
                    mvprintw(7, 0, "Please select from (0-%d)   \n", count - 1);
                    scanw("%d", &idx);
                }
                mvprintw(8, 0, "New size: ");
                clrtoeol();
                refresh();
                scanw("%zu", &new_size);
            }

            if (idx < 0 || idx >= count || ptrs[idx] == NULL)
            {
                mvprintw(9, 0, "Invalid slot.                       ");
                refresh();
            }
            else
            {
                ptrs[idx] = my_realloc(ptrs[idx], new_size);
                mvprintw(9, 0, "realloc finished successfully.        ");
            }
        }
        else if (choice == 4)
        {
            endwin();
            return 0;
        }
        else
            mvprintw(7, 0, "Wrong option, try again.              ");

        draw_heap(12);
        refresh();
    }
    endwin();
    return 0;
}

void *my_malloc(size_t size)
{
    block_t *curr = head;
    block_t *last = NULL;

    // search existing blocks for a free one that fits
    while (curr)
    {
        if (curr->free && curr->size >= size)
        {
            if (curr->size >= size + BLOCK_SIZE) // in case we find a huge block of memory we split it
            {
                block_t *new_free = (block_t *)((char *)(curr + 1) + size);
                new_free->size = curr->size - size - BLOCK_SIZE; // original block's total usable space, minus what we're keeping for the caller, minus room for the new header
                // e.g.the current size is 700, if we free it we do 700- 10(size)-24(new headers) = 666
                new_free->free = 1;
                new_free->next = curr->next; // cope before overwriting
                curr->size = size;           // shrink
                curr->next = new_free;
            }
            curr->free = 0;
            return (void *)(curr + 1); //+1=shift by sizeof(type) bytes
        }
        last = curr;
        curr = curr->next;
    }

    // ask for mem
    block_t *new_block = sbrk(BLOCK_SIZE + size);
    if (new_block == (void *)-1)
        return NULL; // sbrk failed

    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL;

    if (last)
    {
        last->next = new_block;
    }
    else
    {
        head = new_block; // this is the very first alloc
    }

    return (void *)(new_block + 1);
}

void my_free(void *ptr)
{
    if (!ptr)
        return;

    block_t *block = (block_t *)ptr - 1; // step back from payload
    block->free = 1;                     // mark reusable
}

void *my_realloc(void *ptr, size_t new_size)
{
    if (ptr == NULL)
        return my_malloc(new_size);

    if (new_size == 0)
    {
        my_free(ptr);
        return NULL;
    }

    block_t *block = (block_t *)ptr - 1;

    if (block->size >= new_size)
    {
        return ptr;
    }

    void *new_ptr = my_malloc(new_size);
    if (new_ptr == NULL)
        return NULL; // σε περιπτωση αποτυχιας να εχουμε ακομα τα δεδομενα.

    memcpy(new_ptr, ptr, block->size);
    my_free(ptr);
    return new_ptr;
}

void draw_heap(int start_row)
{
    // clear a generous chunk of the display region first, so old rows
    // from a longer previous heap don't linger when blocks disappear
    for (int i = start_row; i < start_row + 30; i++)
    {
        move(i, 0);
        clrtoeol();
    }

    mvprintw(start_row, 0, "--- Heap State ---");
    int row = start_row + 1;

    block_t *curr = head;
    int i = 0;

    if (!curr)
    {
        mvprintw(row, 0, "(empty - no allocations yet)");
        return;
    }

    while (curr)
    {
        int pair = curr->free ? 2 : 1;
        attron(COLOR_PAIR(pair));
        mvprintw(row, 0, "Block %d | addr: %p | size: %-5zu | %s", i, (void *)curr, curr->size, curr->free ? "FREE" : "USED");
        attroff(COLOR_PAIR(pair));

        row++;
        curr = curr->next;
        i++;
    }
}

char check_free_mem(void **ptrs, int count, char *freed_list)
{
    // keep in mind pts mind need to be past as ***, as it might not change the array on mem but a copy(prb thats what will happen)
    // check_free_mem will return the last number(as char) and then print it on the (0-%c).By that freed_list must comee to 0.

    // απο το τελος του πινακα πρεπει να εχω ενα δεν ισχυει μεχρις οτου να υπαρξει !=NULL
    int notNull = 0;
    int sum = 0;
    for (int i = count; i > 0; i--)
    {
        if (ptrs[i] != NULL)
            notNull = 1;
        else
            sum++;
    }

    if (notNull == 1)
        return ptrs[count - sum - 1];
}