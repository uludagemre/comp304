/**
 * virtmem.c 
 * Written by Michael Ballantyne 
 * Modified by Didem Unat
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TLB_SIZE 16
#define PAGES 256
#define PAGE_MASK 255
#define FRAMES 64

#define PAGE_SIZE 256
#define OFFSET_BITS 8
#define OFFSET_MASK 255

#define MEMORY_SIZE FRAMES *PAGE_SIZE

// Max number of characters per line of input file to read.
#define BUFFER_SIZE 10

struct tlbentry
{
  unsigned char logical;
  unsigned char physical;
};

typedef struct pagequeue
{
  int *queueArray;
  int size;
  int numberOfFrames;
  int head;
  int tail;
} PageQueue;

PageQueue *initQueue(int numberOfFrames)
{
  PageQueue *queue = (PageQueue *)malloc(sizeof(PageQueue));
  queue->size = 0;
  queue->numberOfFrames = numberOfFrames;
  queue->head = 0;
  queue->tail = numberOfFrames - 1;
  queue->queueArray = (int *)malloc(queue->numberOfFrames * sizeof(int));
  return queue;
}

void enqueue(PageQueue *queue, int data)
{
  if (!(queue->numberOfFrames == queue->size))
  {
    queue->tail++;
    queue->tail %= queue->numberOfFrames;
    queue->queueArray[queue->tail] = data;
    queue->size++;
  }
  else
  {
    return;
  }
}

int dequeue(PageQueue *queue)
{
  if (!(queue->size == 0))
  {
    int element = queue->queueArray[queue->head];
    queue->head++;
    queue->head %= queue->numberOfFrames;
    queue->size--;
    return element;
  }
  else
  {
    return -1;
  }
}

bool dataExists(PageQueue *queue, int data)
{
  if (!(queue->size == 0))
  {
    for (int i = queue->head; i < queue->head + queue->size; i++)
    {
      if (queue->queueArray[i] == data)
        return true;
    }
  }
  return false;
}

// TLB is kept track of as a circular array, with the oldest element being overwritten once the TLB is full.
struct tlbentry tlb[TLB_SIZE];
// number of inserts into TLB that have been completed. Use as tlbindex % TLB_SIZE for the index of the next TLB line to use.
int tlbindex = 0;

// pagetable[logical_page] is the physical page number for logical page. Value is -1 if that logical page isn't yet in the table.
int pagetable[PAGES];

signed char main_memory[MEMORY_SIZE];

PageQueue *queue;

// Pointer to memory mapped backing file
signed char *backing;

int max(int a, int b)
{
  if (a > b)
    return a;
  return b;
}

/* Returns the physical address from TLB or -1 if not present. */
int search_tlb(unsigned char logical_page)
{
  int i;
  for (i = max((tlbindex - TLB_SIZE), 0); i < tlbindex; i++)
  {
    struct tlbentry *entry = &tlb[i % TLB_SIZE];

    if (entry->logical == logical_page)
    {
      return entry->physical;
    }
  }

  return -1;
}

/* Adds the specified mapping to the TLB, replacing the oldest mapping (FIFO replacement). */
void add_to_tlb(unsigned char logical, unsigned char physical)
{
  struct tlbentry *entry = &tlb[tlbindex % TLB_SIZE];
  tlbindex++;
  entry->logical = logical;
  entry->physical = physical;
}

int main(int argc, const char *argv[])
{
  if (argc != 5)
  {
    fprintf(stderr, "Usage ./virtmem backingstore input -p replacementpolicy\n");
    exit(1);
  }
  else if (strcmp(argv[3], "-p") != 0)
  {
    fprintf(stderr, "Usage ./virtmem backingstore input -p replacementpolicy\n");
    exit(1);
  }

  const char *backing_filename = argv[1];
  int backing_fd = open(backing_filename, O_RDONLY);
  backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0);

  const char *input_filename = argv[2];
  FILE *input_fp = fopen(input_filename, "r");

  int rp = atoi(argv[4]);

  // Fill page table entries with -1 for initially empty table.
  int i;
  for (i = 0; i < PAGES; i++)
  {
    pagetable[i] = -1;
  }

  queue = initQueue(FRAMES);

  // Character buffer for reading lines of input file.
  char buffer[BUFFER_SIZE];

  // Data we need to keep track of to compute stats at end.
  int total_addresses = 0;
  int tlb_hits = 0;
  int page_faults = 0;

  // Number of the next unallocated physical page in main memory
  unsigned char free_page = 0;

  while (fgets(buffer, BUFFER_SIZE, input_fp) != NULL)
  {
    total_addresses++;
    int logical_address = atoi(buffer);
    int offset = logical_address & OFFSET_MASK;
    int logical_page = (logical_address >> OFFSET_BITS) & PAGE_MASK;

    int physical_page = search_tlb(logical_page);
    // TLB hit
    if (physical_page != -1)
    {
      tlb_hits++;
      // TLB miss
    }
    else
    {
      physical_page = pagetable[logical_page];

      // Page fault
      if (physical_page == -1)
      {
        page_faults++;

        if (free_page < FRAMES)
        {
          physical_page = free_page;
          free_page++;
          if (rp == 0)
          { // FIFO
            enqueue(queue, physical_page);
          }
          else
          { // LRU
            // save_physical_page_for_lru(physical_page);
            printf("asd");
          }
        }
        else
        {
          if (rp == 0)
          { // FIFO
            physical_page = dequeue(queue);
            enqueue(queue, physical_page);
          }
          else
          { // LRU
            // physical_page = get_physical_page_for_lru();
            // save_physical_page_for_lru(physical_page);
            printf("asd");
          }
        }
        // Copy page from backing file into physical memory
        memcpy(main_memory + physical_page * PAGE_SIZE, backing + logical_page * PAGE_SIZE, PAGE_SIZE);

        pagetable[logical_page] = physical_page;
      }

      add_to_tlb(logical_page, physical_page);
    }

    int physical_address = (physical_page << OFFSET_BITS) | offset;
    signed char value = main_memory[physical_page * PAGE_SIZE + offset];

    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, value);
  }

  printf("Number of Translated Addresses = %d\n", total_addresses);
  printf("Page Faults = %d\n", page_faults);
  printf("Page Fault Rate = %.3f\n", page_faults / (1. * total_addresses));
  printf("TLB Hits = %d\n", tlb_hits);
  printf("TLB Hit Rate = %.3f\n", tlb_hits / (1. * total_addresses));

  return 0;
}
