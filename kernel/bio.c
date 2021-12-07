// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#define HASHSIZE 35
extern uint ticks;

struct spinlock steallock;


struct {
  //struct spinlock lock;
  struct buf buf[NBUF];
  struct spinlock slock;
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least
} bcache[HASHSIZE];


uint64
hashfuc(uint64 key)
{
  return key % HASHSIZE;
}

void
binit(void)
{
  struct buf *b; 
  // Create linked list of buffer
  for (int i =0;i<HASHSIZE;i++){
    initlock(&bcache[i].slock,"bcache");
    for(b = bcache[i].buf; b < bcache[i].buf+NBUF; b++){
      //initsleeplock(&b->lock, "buffer");
      b->timesteamp = 0;
      b->bucket = i;
      //bcache[i].buf[j].timesteamp = 0;
      //bcache[i].buf[j].bucket =i;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  struct buf *tmp = 0;
  int k = hashfuc(blockno);
  acquire(&bcache[k].slock);

  //holdhashlock(blockno);
  //acquire(&bcache.lock);

  // Is the block already cached?
  for(b = &bcache[k].buf[0]; b < &bcache[k].buf[0]+NBUF; b++){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      b->timesteamp += ticks;
      //release(&bcache.lock);
      
      release(&bcache[k].slock);

      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  uint minstamp=~0;
  //printf("minstamp : %d",minstamp);

  for(b = &bcache[k].buf[0]; b < &bcache[k].buf[0]+NBUF; b++)
  {
    if (b->timesteamp < minstamp && b->refcnt == 0)
    {
      minstamp = b->timesteamp;
      tmp = b;
    }
  }


  if (tmp->refcnt == 0){
  tmp->dev = dev;
  tmp->blockno = blockno;
  tmp->valid = 0;
  tmp->refcnt = 1;
  tmp->timesteamp = ticks;
  //release(&bcache.lock);

  release(&bcache[k].slock);
  acquiresleep(&tmp->lock);
  return tmp;
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache[b->bucket].slock);
  b->refcnt--;
  release(&bcache[b->bucket].slock);
  
}

void
bpin(struct buf *b) {
  acquire(&bcache[b->bucket].slock);
  b->refcnt++;
  release(&bcache[b->bucket].slock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache[b->bucket].slock);
  b->refcnt--;
  release(&bcache[b->bucket].slock);
}


