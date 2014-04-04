/*
 ============================================================================
 Name        : test.c
 Author      : zj
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


float myexp(float x){
	int i,k,m,t;
	int xm=(int)x;
	float sum;
	float e ;
	float ef;
	float z ;
	float sub=x-xm;
	m=1;      //阶乘算法分母
	e=1.0;  //e的xm
	ef=1.0;
	t=30;      //算法精度
	z=1;  //分子初始化
	sum=1;
	//  printf("x=%f\n",x);
	//  printf("sub=%f\n",sub);
	if (xm<0) {     //判断xm是否大于0？
		xm=(-xm);
		for(k=0;k<xm;k++){
			ef*=2.718281;
			}
		//printf("ef=%f\n",ef);
		e/=ef;
	}
	else {
		for(k=0;k<xm;k++){
			e*=2.718281;
		}
	}
		// printf("e=%f\n",e);
		//  printf("xm=%d\n",xm);
	for(i=1;i<t;i++){
		m*=i;
		z*=sub;
		sum+=z/m;
	}
	return sum*e;
}
