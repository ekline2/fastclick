#ifndef IPRRMAPPER_HH
#define IPRRMAPPER_HH
#include "elements/ip/iprewriter.hh"

/*
 * =c
 * IPRoundRobinMapper(PATTERN1, ..., PATTERNn)
 * =io
 * None
 * =d
 *
 * Works in tandem with IPRewriter to provide round-robin rewriting. This is
 * useful, for example, in load-balancing applications. Implements the
 * IPMapper interface.
 *
 * Responds to mapping requests from an IPRewriter by trying the PATTERNs in
 * round-robin order and returning the first successfully created mapping.
 *
 * =a IPRewriter IPRewriterPatterns */

class IPRoundRobinMapper : public Element, public IPMapper {

  Vector<IPRewriter::Pattern *> _patterns;
  Vector<int> _forward_outputs;
  Vector<int> _reverse_outputs;
  int _last_pattern;
  
 public:

  IPRoundRobinMapper()			{ }

  const char *class_name() const	{ return "IPRoundRobinMapper"; }
  void *cast(const char *);
  
  IPRoundRobinMapper *clone() const	{ return new IPRoundRobinMapper; }
  int configure_phase() const		{ return IPRewriter::CONFIGURE_PHASE_MAPPER; }
  int configure(const Vector<String> &, ErrorHandler *);
  void uninitialize();
  
  void notify_rewriter(IPRewriter *, ErrorHandler *);
  IPRewriter::Mapping *get_map(bool, const IPFlowID &, IPRewriter *);
  
};

#endif
