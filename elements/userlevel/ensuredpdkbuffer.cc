// -*- c-basic-offset: 4; related-file-name: "ensuredpdkbuffer.hh" -*-
/*
 * ensuredpdkbuffer.{cc,hh} - Ensure DPDK Buffer
 * Tom Barbette
 *
 * Copyright (c) 2015 University of Liège
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include <click/args.hh>
#include <click/dpdkdevice.hh>
#include "ensuredpdkbuffer.hh"

CLICK_DECLS


EnsureDPDKBuffer::EnsureDPDKBuffer() : _force(false), _extra_headroom(0), _noalloc(false), _warn_count(0)
{
}

EnsureDPDKBuffer::~EnsureDPDKBuffer()
{
    //TODO : make this work even without FromDPDK/ToDPDK
}


int
EnsureDPDKBuffer::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (Args(conf, this, errh)
    .read_p("FORCE_COPY", _force)
    .read_p("EXTRA_HEADROOM", _extra_headroom)
    .read("FAIL",_noalloc)
    .complete() < 0)
    return -1;

    if (_force && _noalloc) {
        return errh->error("You cannot have both FORCE_COPY and FAIL at the same time");
    }

    return 0;
}

int
EnsureDPDKBuffer::initialize(ErrorHandler *errh)
{
    return DPDKDevice::initialize(errh);
}


inline Packet*
EnsureDPDKBuffer::smaction(Packet* p) {
    if (!_force && (DPDKDevice::is_dpdk_buffer(p))) {
        return p;
    } else if (_noalloc) {
        p->kill();
        if (_warn_count++ < 5)
            click_chatter("%p{element}: Not a DPDK packet", this);
        return 0;
    } else {
        struct rte_mbuf* mbuf = DPDKDevice::get_pkt();
        if (!mbuf) {
            p->kill();
            if (_warn_count++ < 5)
                click_chatter("%s: No more DPDK buffer! Dropping packet.", name().c_str());
            return 0;
        }
        WritablePacket* q = WritablePacket::make(
                (unsigned char*)mbuf->buf_addr,
                DPDKDevice::MBUF_DATA_SIZE,
                DPDKDevice::free_pkt,
                (void*)mbuf);
        if (!q) {
            p->kill();
            rte_pktmbuf_free(mbuf);
            return 0;
        } else if (q->copy(p,rte_pktmbuf_headroom(mbuf) + _extra_headroom)) {
            p->kill();
            return q;
        } else {
            click_chatter("[%s] Could not ensure buffer for packet with length %d bytes. Dropped!", name().c_str(), p->length());
            if (p->length() <= (DPDKDevice::MBUF_DATA_SIZE - RTE_PKTMBUF_HEADROOM)) {
                p->kill();
            }
            q->kill();
            return 0;
        }
    }
}

#if HAVE_BATCH
PacketBatch*
EnsureDPDKBuffer::simple_action_batch(PacketBatch *head)
{
#if HAVE_ZEROCOPY
    EXECUTE_FOR_EACH_PACKET_DROPPABLE(smaction,head,[](Packet* p) {click_chatter("No more DPDK buffer! Dropping packet %p!", p);});
#endif
    return head;
}
#endif
Packet*
EnsureDPDKBuffer::simple_action(Packet* p) {
#if HAVE_ZEROCOPY
    return smaction(p);
#else
    return p;
#endif
}




CLICK_ENDDECLS
ELEMENT_REQUIRES(userlevel dpdk)
EXPORT_ELEMENT(EnsureDPDKBuffer)
