#include "str.h"
#include "byte.h"
#include "scan.h"
#include "exit.h"
#include "stralloc.h"
#include "buffer.h"
#include "strerr.h"
#include "uint16.h"
#include "response.h"
#include "printpacket.h"
#include "parsetype.h"
#include "dns.h"

#define FATAL "tinydns-get: fatal: "

void usage(void)
{
  strerr_die1x(100,"tinydns-get: usage: tinydns-get type name");
}
void oops(void)
{
  strerr_die2sys(111,FATAL,"unable to parse: ");
}

char type[2];
static char *q;

static stralloc out;

main(int argc,char **argv)
{
  uint16 u16;

  if (!*argv) usage();

  if (!*++argv) usage();
  if (!parsetype(*argv,type)) usage();

  if (!*++argv) usage();
  if (!dns_domain_fromdot(&q,*argv,str_len(*argv))) oops();

  if (!stralloc_copys(&out,"")) oops();
  uint16_unpack_big(type,&u16);
  if (!stralloc_catulong0(&out,u16,0)) oops();
  if (!stralloc_cats(&out," ")) oops();
  if (!dns_domain_todot_cat(&out,q)) oops();
  if (!stralloc_cats(&out,":\n")) oops();

  if (!response_query(q,type)) oops();
  response[3] &= ~128;
  response[2] &= ~1;
  response[2] |= 4;
  case_lowerb(q,dns_domain_length(q));

  if (byte_equal(type,2,DNS_T_AXFR)) {
    response[3] &= ~15;
    response[3] |= 4;
  }
  else
    if (!respond(q,type,"\0\0\0\0")) goto DONE;

  if (!printpacket_cat(&out,response,response_len)) oops();

  DONE:
  buffer_putflush(buffer_1,out.s,out.len);
  _exit(0);
}