#ifndef MIDDLEBOX_TCPREORDER_HH
#define MIDDLEBOX_TCPREORDER_HH

#include <click/config.h>
#include <click/element.hh>
#include <clicknet/tcp.h>
#include <clicknet/ip.h>
#include <click/batchelement.hh>
#include <click/multithread.hh>
#include "batchfcb.hh"
#include "tcpreordernode.hh"
#include "memorypool.hh"
#include "fcb.hh"
#include "tcpelement.hh"

#define TCPREORDER_POOL_SIZE 20

CLICK_DECLS

/*
=c

TCPReorder(FLOWDIRECTION [, MERGESORT])

=s middlebox

reorder TCP packets

=d

This element reorder TCP packets before sending them on its first output.

=item FLOWDIRECTION

ID of the path for the connection (0 or 1). The return path must have the other ID.
Thus, each direction of a TCP connection has a different ID.

=item MERGESORT

Only used with batching. Add each packets of the batch at the beginning of the list and reorder
the list using merge sort instead of adding each packet of the batch directly at the right position.

Complexity to process a batch with this option: O((n + k) * log (n + k))
Complexity to process a batch without this option: O(k * n)

Where k is the number of packets in the batch and n is the number of packets in the waiting list

Default value: true.

=a TCPIn, TCPOut */

class TCPReorder : public BatchElement, public TCPElement
{
public:
    TCPReorder() CLICK_COLD;
    ~TCPReorder() CLICK_COLD;

    // Click related methods
    const char *class_name() const        { return "TCPReorder"; }
    const char *port_count() const        { return PORTS_1_1X2; }
    const char *processing() const        { return PUSH; }

    int configure(Vector<String>&, ErrorHandler*) CLICK_COLD;

    void push_packet(int, Packet*);

    #if HAVE_BATCH
    void push_batch(int, PacketBatch *batch);
    #endif

private:
    void processPacket(struct fcb *fcb, Packet* packet);
    void processPacketBatch(struct fcb *fcb, PacketBatch* batch);
    void putPacketInList(struct fcb *fcb, Packet* packet);
    void sendEligiblePackets(struct fcb *fcb);
    tcp_seq_t getSequenceNumber(Packet* packet);
    void checkFirstPacket(struct fcb *fcb, Packet* packet);
    void flushList(struct fcb *fcb);
    void flushListFrom(struct fcb *fcb, struct TCPPacketListNode *toKeep,
        struct TCPPacketListNode *toRemove);
    bool checkRetransmission(struct fcb *fcb, Packet* packet);
    tcp_seq_t getNextSequenceNumber(Packet* packet);
    TCPPacketListNode* sortList(TCPPacketListNode *list);

    unsigned int flowDirection;
    bool mergeSort;

    per_thread<MemoryPool<struct TCPPacketListNode>> pool;
};

CLICK_ENDDECLS

#endif
