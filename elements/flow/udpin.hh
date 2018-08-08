#ifndef MIDDLEBOX_UDPIn_HH
#define MIDDLEBOX_UDPIn_HH
#include "ipelement.hh"
#include "stackelement.hh"
#include <click/element.hh>

CLICK_DECLS

class UDPIn : public StackElement, public IPElement
{
public:
    /** @brief Construct an UDPIn element
     */
    UDPIn() CLICK_COLD;

    const char *class_name() const        { return "UDPIn"; }
    const char *port_count() const        { return PORTS_1_1; }
    const char *processing() const        { return PROCESSING_A_AH; }

    FLOW_ELEMENT_DEFINE_SESSION_CONTEXT("12/0/ffffffff 16/0/ffffffff 20/0/ffff 22/0/ffff", FLOW_UDP);

    int configure(Vector<String> &, ErrorHandler *) CLICK_COLD;

    void push_batch(int, PacketBatch*) override;

    virtual void removeBytes(WritablePacket*, uint32_t, uint32_t) override;
    virtual WritablePacket* insertBytes(WritablePacket*, uint32_t,
         uint32_t) override CLICK_WARN_UNUSED_RESULT;
};

CLICK_ENDDECLS
#endif