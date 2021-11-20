//rcelyte's scuffed C enum reflection helper, 80ch GOLFED edition; DO NOT TOUCH
// 0x3ff0000,0x87fffffe,0x7fffffe
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define reflect(e,v) _reflect(reflect_##e,#e "_",v)
static const char*_reflect(const char *e,const char *t,uint32_t v){static const
uint32_t s[]={0,67043328,2281701374,134217726,0,0,0,0};static char o[128];const
char *p=e;for(uint32_t i=0;i!=v&&*e;++e){if(*e==44){++i;p=e+1;}else if(*e==61){
;while(*e&&!(s[*e>>5]>>(*e&31)&1))++e;i=atoll(e);if(i==v){e=p;break;}}}for(;*e;
++e){if(s[*e>>5]>>(*e&31)&1){uint32_t h=strlen(t);if(!strncmp(e,t,h))e+=h;uint32_t
l=0;while(s[e[l]>>5]>>(e[l]&31)&1)++l;sprintf(o,"%.*s",l,e);return o;}}sprintf(
o,"%u",v);return o;}
