/* 

    TestImageFilter
    
    Test program for MMX filter routines

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "SDL/SDL_imageFilter.h"

void setup_src(unsigned char *src1, unsigned char *src2)
{
 int i;
 
 src1[0]=1;
 src1[2]=1; 
 src1[1]=4;
 src1[3]=3;
 src1[4]=33;
 for (i=5; i<14; i++) src1[i]=i;
 src1[14]=8;

 src2[0]=1;
 src2[1]=3;
 src2[2]=3; 
 src2[3]=2;
 src2[4]=44;
 for (i=5; i<14; i++) src2[i]=14-i;
 src2[14]=10;
}

void print_result(char *label,unsigned char *src1, unsigned char *src2, unsigned char *dst) 
{
 int i;
 char blabel[80];
 
 memset((void *)blabel,(int)' ',80);
 blabel[strlen(label)]=0;
 printf ("%s   pos   %2d %2d %2d %2d %2d %2d %2d %2d %2d .. %2d\n",blabel,0,1,2,3,4,5,6,7,8,14);
 printf ("%s   src1  %02x %02x %02x %02x %02x %02x %02x %02x %02x .. %02x\n",blabel,src1[0],src1[1],src1[2],src1[3],src1[4],src1[5],src1[6],src1[7],src1[8],src1[14]);
 if (src2) {
  printf ("%s   src2  %02x %02x %02x %02x %02x %02x %02x %02x %02x .. %02x\n",blabel,src2[0],src2[1],src2[2],src2[3],src2[4],src2[5],src2[6],src2[7],src2[8],src2[14]);
 }
 printf ("%s   dest  %02x %02x %02x %02x %02x %02x %02x %02x %02x .. %02x\n",label, dst[0], dst[1], dst[2], dst[3], dst[4], dst[5], dst[6], dst[7], dst[8],dst[14]);
 printf ("\n");
}

void print_compare(unsigned char *dst1, unsigned char *dst2) 
{ 
 if (bcmp(dst1,dst2,15)==0) {
  printf ("OK\n");
 } else {
  printf ("ERROR\n");
 }
}

void print_line() {
 printf ("------------------------------------------------------------------------\n\n\n");
}

/* ----------- main ---------- */

int main(int argc, char *argv[])
{

 unsigned char src1[15],src2[15],dstm[15],dstc[15];

 /* SDL_imageFilter Test */

 printf ("TestImageFilter\n\n");
 printf ("Testing an array of 15 bytes - first 8 bytes should be processed\n");
 printf ("by MMX or C code, the last 7 bytes only by C code.\n\n");
 

 print_line();

 SDL_imageFilterMMXon();
 
 setup_src(src1, src2); 
 SDL_imageFilterBitAnd ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15); 
 print_result ("MMX BitAnd", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2); 
 SDL_imageFilterBitAnd ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15); 
 print_result (" C  BitAnd", src1, src2, dstc);

 print_compare(dstm,dstc);
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterBitOr ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("MMX BitOr", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterBitOr ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  BitOr", src1, src2, dstc);

 print_compare(dstm,dstc);
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterAdd ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("MMX Add", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterAdd ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  Add", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterAbsDiff ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("MMX AbsDiff", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterAbsDiff ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  AbsDiff", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterMean ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("MMX Mean", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterMean ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  Mean", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterSub ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("MMX Sub", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterSub ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  Sub", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterMult ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("MMX Mult", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterMult ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  Mult", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterMultNor ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("ASM MultNor", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterMultNor ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  MultNor", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterMultDivby2 ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("MMX MultDivby2", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterMultDivby2 ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  MultDivby2", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterMultDivby4 ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("MMX MultDivby4", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterMultDivby4 ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  MultDivby4", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();
 
 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterDiv ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstm,15);
 print_result ("ASM Div", src1, src2, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterDiv ((unsigned char *)src1,(unsigned char *)src2,(unsigned char *)dstc,15);
 print_result (" C  Div", src1, src2, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterBitNegation ((unsigned char *)src1,(unsigned char *)dstm,15);
 print_result ("MMX BitNegation", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterBitNegation ((unsigned char *)src1,(unsigned char *)dstc,15);
 print_result (" C  BitNegation", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterAddByte ((unsigned char *)src1,(unsigned char *)dstm,15, 3);
 print_result ("MMX AddByte(3)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterAddByte ((unsigned char *)src1,(unsigned char *)dstc,15, 3);
 print_result (" C  AddByte(3)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterAddByteToHalf ((unsigned char *)src1,(unsigned char *)dstm,15, 3);
 print_result ("MMX AddByteToHalf(3)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterAddByteToHalf ((unsigned char *)src1,(unsigned char *)dstc,15, 3);
 print_result (" C  AddByteToHalf(3)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterSubByte ((unsigned char *)src1,(unsigned char *)dstm,15, 3);
 print_result ("MMX SubByte(3)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterSubByte ((unsigned char *)src1,(unsigned char *)dstc,15, 3);
 print_result (" C  SubByte(3)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterShiftRight ((unsigned char *)src1,(unsigned char *)dstm,15, 1);
 print_result ("MMX ShiftRight(1)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterShiftRight ((unsigned char *)src1,(unsigned char *)dstc,15, 1);
 print_result (" C  ShiftRight(1)", src1, NULL, dstc);

 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterMultByByte ((unsigned char *)src1,(unsigned char *)dstm,15, 3);
 print_result ("MMX MultByByte(3)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterMultByByte ((unsigned char *)src1,(unsigned char *)dstc,15, 3);
 print_result (" C  MultByByte(3)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterShiftRightAndMultByByte ((unsigned char *)src1,(unsigned char *)dstm,15, 1, 3);
 print_result ("MMX ShiftRightAndMultByByte(1,3)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterShiftRightAndMultByByte ((unsigned char *)src1,(unsigned char *)dstc,15, 1, 3);
 print_result (" C  ShuftRightAndMultByByte(1,3)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterShiftLeftByte ((unsigned char *)src1,(unsigned char *)dstm,15, 3);
 print_result ("MMX ShiftLeftByte(3)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterShiftLeftByte ((unsigned char *)src1,(unsigned char *)dstc,15, 3);
 print_result (" C  ShiftLeftByte(3)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterShiftLeft ((unsigned char *)src1,(unsigned char *)dstm,15, 3);
 print_result ("MMX ShiftLeft(3)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterShiftLeft ((unsigned char *)src1,(unsigned char *)dstc,15, 3);
 print_result (" C  ShiftLeft(3)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterBinarizeUsingThreshold ((unsigned char *)src1,(unsigned char *)dstm,15, 2);
 print_result ("MMX BinarizeUsingThreshold(2)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterBinarizeUsingThreshold ((unsigned char *)src1,(unsigned char *)dstc,15, 2);
 print_result (" C  BinarizeUsingThreshold(2)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterClipToRange ((unsigned char *)src1,(unsigned char *)dstm,15, 1,7);
 print_result ("MMX ClipToRange(1,7)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterClipToRange ((unsigned char *)src1,(unsigned char *)dstc,15, 1,7);
 print_result (" C  ClipToRange(1,7)", src1, NULL, dstc);


 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterNormalizeLinear ((unsigned char *)src1,(unsigned char *)dstm,15, 0,33,0,255);
 print_result ("MMX NormalizeLinear(0,33,0,255)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterNormalizeLinear ((unsigned char *)src1,(unsigned char *)dstc,15, 0,33,0,255);
 print_result (" C  NormalizeLinear(0,33,0,255)", src1, NULL, dstc);

 print_compare(dstm,dstc); 
 print_line();

 /* Uint functions */

 /* Disabled, since broken */
 /*

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterAddUint ((unsigned char *)src1,(unsigned char *)dstm,15, 0x01020304);
 print_result ("MMX AddUint(0x01020304)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterAddUint ((unsigned char *)src1,(unsigned char *)dstc,15, 0x01020304);
 print_result (" C  AddUint(0x01020304)", src1, NULL, dstc);

 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterSubUint ((unsigned char *)src1,(unsigned char *)dstm,15, 0x01020304);
 print_result ("MMX SubUint(0x01020304)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterSubUint ((unsigned char *)src1,(unsigned char *)dstc,15, 0x01020304);
 print_result (" C  SubUint(0x01020304)", src1, NULL, dstc);

 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterShiftRightUint ((unsigned char *)src1,(unsigned char *)dstm,15, 4);
 print_result ("MMX ShiftRightUint(4)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterShiftRightUint ((unsigned char *)src1,(unsigned char *)dstc,15, 4);
 print_result (" C  ShiftRightUint(4)", src1, NULL, dstc);

 print_compare(dstm,dstc); 
 print_line();

 SDL_imageFilterMMXon();

 setup_src(src1, src2);
 SDL_imageFilterShiftLeftUint ((unsigned char *)src1,(unsigned char *)dstm,15, 4);
 print_result ("MMX ShiftLeftUint(4)", src1, NULL, dstm);

 SDL_imageFilterMMXoff();

 setup_src(src1, src2);
 SDL_imageFilterShiftLeftUint ((unsigned char *)src1,(unsigned char *)dstc,15, 4);
 print_result (" C  ShiftLeftUint(4)", src1, NULL, dstc);

 print_compare(dstm,dstc); 
 print_line();
 
 */
 
 exit(0);
}

