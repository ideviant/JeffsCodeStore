/*****************************************************************************
 * 4x4_dyn_range.c:
 *****************************************************************************
 * Copyright (C) 2010 ,All right reserved
 * Authors: JianFeng Zheng
 *
 * 本程序用于寻找H.264标准中4x4变换和量化的动态范围。
 * 动态范围受量化过程中移位补偿的影响，见quant_4x4()函数
 *****************************************************************************
 * static void quant_4x4( int dct[4][4], int quant_mf[4][4], int quant[4][4], int qp )
 * {
 *  int i;
 *  int i_qbits = 15 + qp/6;
 *  int f = 1<<i_qbits;
 *  f = f/2;
 *  for( i = 0; i < 16; i++ )
 * 		QUANT_ONE( quant[0][i], dct[0][i], quant_mf[0][i] );
 * }
 *****************************************************************************
 * f = f/2; 时，反变换后的最大值为 31360
 * f = f/3; 时，反变换后的最大值为 32512
 * f = f/6; 时，反变换后的最大值为 33792
 *****************************************************************************/

#include <stdio.h>
#include <math.h>


#define QUANT_ONE( ret, coef, mf ) \
{ \
	if( (coef) > 0 ) \
	(ret) = ( f + (coef) * (mf) ) >> i_qbits; \
	else \
	(ret) = - ( ( f - (coef) * (mf) ) >> i_qbits ); \
}

static void quant_4x4( int dct[4][4], int quant_mf[4][4], int quant[4][4], int qp )
{
	int i;
	int i_qbits = 15 + qp/6;
	int f = 1<<i_qbits;
	f = f/2;
	for( i = 0; i < 16; i++ )
		QUANT_ONE( quant[0][i], dct[0][i], quant_mf[0][i] );
}

static void signMatrix( int m, unsigned int in, int sign[4][4] )
{
	static int bit16[4][4] = {
		{0x0001,0x0002,0x0004,0x0008},
		{0x0010,0x0020,0x0040,0x0080},
		{0x0100,0x0200,0x0400,0x0800},
		{0x1000,0x2000,0x4000,0x8000}
	};

	short val[2];
	val[0] = abs(m);
	val[1] = -val[0];

	for(int y=0;y<4;y++)
	{
		for(int x=0;x<4;x++)
		{
			sign[y][x] = val[ (in & bit16[y][x]) == bit16[y][x] ];
		}
	}
}

static void dctCore( int d[4][4], int dct[4][4], int dct_l[4][4] )
{
	int tmp[4][4];
	int i;

	for( i = 0; i < 4; i++ )
	{
		const int s03 = d[i][0] + d[i][3];
		const int s12 = d[i][1] + d[i][2];
		const int d03 = d[i][0] - d[i][3];
		const int d12 = d[i][1] - d[i][2];

		tmp[0][i] =   s03 +   s12;
		tmp[1][i] = ((d03 * 5)>>1) + d12;
		tmp[2][i] =   s03 -   s12;
		tmp[3][i] =   d03 - ((d12 * 5)>>1);
	}

	for( i = 0; i < 4; i++ )
	{
		const int s03 = tmp[i][0] + tmp[i][3];
		const int s12 = tmp[i][1] + tmp[i][2];
		const int d03 = tmp[i][0] - tmp[i][3];
		const int d12 = tmp[i][1] - tmp[i][2];

		dct[i][0] =   s03 +   s12;
		dct[i][1] = ((d03 * 5)>>1) + d12;
		dct[i][2] =   s03 -   s12;
		dct[i][3] =   d03 - ((d12 * 5)>>1);
	}
}


static void idctCore( int dct[4][4], int idct1[4][4], int idct2[4][4] )
{
	int i;

	for( i = 0; i < 4; i++ )
	{
		const int s02 =  dct[0][i]  +  dct[2][i];
		const int d02 =  dct[0][i]  -  dct[2][i];
		const int s13 =((dct[1][i] * 5)>>1)   +  dct[3][i];
		const int d13 =  dct[1][i]  -((dct[3][i]*5)>>1);

		idct1[i][0] = s02 + s13;
		idct1[i][1] = d02 + d13;
		idct1[i][2] = d02 - d13;
		idct1[i][3] = s02 - s13;
	}

	for( i = 0; i < 4; i++ )
	{
		const int s02 =  idct1[0][i]  +  idct1[2][i];
		const int d02 =  idct1[0][i]  -  idct1[2][i];
		const int s13 =((idct1[1][i] * 5)>>1)   +  idct1[3][i];
		const int d13 =  idct1[1][i]  -((idct1[3][i]*5)>>1);

		idct2[0][i] = s02 + s13;
		idct2[1][i] = d02 + d13;
		idct2[2][i] = d02 - d13;
		idct2[3][i] = s02 - s13;
	}
}

/**
 *  \brief init quant matrix
 */
static void QX( int qp, int qMatrix[4][4], int dqMatrix[4][4] )
{
	static int qmc[6][3]={
		{13107,	3616,	6884},
		{11916,	3287,	6258},
		{10082,	2781,	5296},
		{9362,	2583,	4917},
		{8192,	2260,	4303},
		{7282,	2009,	3825}
	};
	static int dqmc[6][3]={
		{10, 3, 5},
		{11, 3, 6},
		{13, 4, 7},
		{14, 4, 7},
		{16, 5, 8},
		{18, 5, 9}
	};
	
	int i_qp = qp%6;
	for( int i = 0; i < 16; i++ )
	{
		qMatrix[0][i] = qmc[i_qp][2];
		dqMatrix[0][i] = dqmc[i_qp][2];
	}

	qMatrix[0][0] = qMatrix[0][2] = qMatrix[2][0] = qMatrix[2][2] = qmc[i_qp][0];
	qMatrix[1][1] = qMatrix[1][3] = qMatrix[3][1] = qMatrix[3][3] = qmc[i_qp][1];

	dqMatrix[0][0] = dqMatrix[0][2] = dqMatrix[2][0] = dqMatrix[2][2] = dqmc[i_qp][0];
	dqMatrix[1][1] = dqMatrix[1][3] = dqMatrix[3][1] = dqMatrix[3][3] = dqmc[i_qp][1];
}

static void showQuantMx()
{
	int qp;
	int qMatrix[4][4];
	int dqMatrix[4][4];
	static int qmc[6][3]={
		{13107,	3616,	6884},
		{11916,	3287,	6258},
		{10082,	2781,	5296},
		{9362,	2583,	4917},
		{8192,	2260,	4303},
		{7282,	2009,	3825}
	};
	static int dqmc[6][3]={
		{10, 3, 5},
		{11, 3, 6},
		{13, 4, 7},
		{14, 4, 7},
		{16, 5, 8},
		{18, 5, 9}
	};

	for( int i_qp=0; i_qp<6; i_qp++)
	{
		for( int i = 0; i < 16; i++ )
			qMatrix[0][i] = qmc[i_qp][2];

		qMatrix[0][0] = qMatrix[0][2] = qMatrix[2][0] = qMatrix[2][2] = qmc[i_qp][0];
		qMatrix[1][1] = qMatrix[1][3] = qMatrix[3][1] = qMatrix[3][3] = qmc[i_qp][1];

		printf("{ ");
		for(int y=0;y<4;y++)
			printf("{ %5d, %5d, %5d, %5d }, \n",qMatrix[y][0],qMatrix[y][1],qMatrix[y][2],qMatrix[y][3]);
		printf(" }\n");
	}

	for( int i_qp=0; i_qp<6; i_qp++)
	{
		for( int i = 0; i < 16; i++ )
			dqMatrix[0][i] = dqmc[i_qp][2];

		dqMatrix[0][0] = dqMatrix[0][2] = dqMatrix[2][0] = dqMatrix[2][2] = dqmc[i_qp][0];
		dqMatrix[1][1] = dqMatrix[1][3] = dqMatrix[3][1] = dqMatrix[3][3] = dqmc[i_qp][1];

		printf("{ ");
		for(int y=0;y<4;y++)
			printf("{ %2d, %2d, %2d, %2d }, \n",dqMatrix[y][0],dqMatrix[y][1],dqMatrix[y][2],dqMatrix[y][3]);
		printf(" }\n");
	}
}

static void dequant_4x4( int quant[4][4], int dequant_mf[4][4], int dequant[4][4], int qp )
{
	int i;
	int i_qbits = qp/6;
	for( i = 0; i < 16; i++ )
		dequant[0][i] = ( (quant[0][i]*dequant_mf[0][i]) << i_qbits );
}

static int maxItem( int matrix[52][4][4], int *pQP )
{
	int val = 0;
	int max_qp = 0;
	int (*p)[4];

	for (int qp=0;qp<52;qp++)
	{
		p = matrix[qp];
		for( int i = 0; i < 16; i++ )
		{
			if (abs(p[0][i])>val)
			{
				val = abs(p[0][i]);
				max_qp = qp;
			}
		}
	}

	if (pQP)
		*pQP = max_qp;

	return val;	
}

typedef struct dynatic{
	int val,qp,sign;
}dyn_t;

static void show_matrix( char *name, int m[4][4])
{
	printf("%s=\n",name);
	for(int y=0;y<4;y++)
	{
		for(int x=0;x<4;x++)
		{
			printf("%8d ",m[y][x]);
		}
		printf("\n");
	}
	printf("\n");
}

static void dc_trans( int dc[4][4], int hadamard1[4][4], int hadamard2[4][4] )
{
	int tmp[4][4];
	int s01, s23;
	int d01, d23;
	int i;

	for( i = 0; i < 4; i++ )
	{
		s01 = dc[i][0] + dc[i][1];
		d01 = dc[i][0] - dc[i][1];
		s23 = dc[i][2] + dc[i][3];
		d23 = dc[i][2] - dc[i][3];

		tmp[0][i] = s01 + s23;
		tmp[1][i] = s01 - s23;
		tmp[2][i] = d01 - d23;
		tmp[3][i] = d01 + d23;
	}

	for( i = 0; i < 4; i++ )
	{
		s01 = tmp[i][0] + tmp[i][1];
		d01 = tmp[i][0] - tmp[i][1];
		s23 = tmp[i][2] + tmp[i][3];
		d23 = tmp[i][2] - tmp[i][3];

		hadamard2[i][0] = ( s01 + s23 + 1 ) >> 1;
		hadamard2[i][1] = ( s01 - s23 + 1 ) >> 1;
		hadamard2[i][2] = ( d01 - d23 + 1 ) >> 1;
		hadamard2[i][3] = ( d01 + d23 + 1 ) >> 1;
	}
}

static void quant_4x4_dc( int hadamard2[4][4], int quant_mf[4][4], int quant_dc[4][4], int qp )
{
	int i;
	int i_qbits = 16 + qp/6;
	int f = 1<<i_qbits;
	f = f/2;
	for( i = 0; i < 16; i++ )
		quant_dc[0][i] = ( f + hadamard2[0][i] * quant_mf[0][0] ) >> (i_qbits);
}

static void dc_trans_inv( int quant_dc[4][4], int idc1[4][4], int idc2[4][4] )
{
	int s01, s23;
	int d01, d23;
	int i;

	for( i = 0; i < 4; i++ )
	{
		s01 = quant_dc[i][0] + quant_dc[i][1];
		d01 = quant_dc[i][0] - quant_dc[i][1];
		s23 = quant_dc[i][2] + quant_dc[i][3];
		d23 = quant_dc[i][2] - quant_dc[i][3];

		idc1[0][i] = s01 + s23;
		idc1[1][i] = s01 - s23;
		idc1[2][i] = d01 - d23;
		idc1[3][i] = d01 + d23;
	}

	for( i = 0; i < 4; i++ )
	{
		s01 = idc1[i][0] + idc1[i][1];
		d01 = idc1[i][0] - idc1[i][1];
		s23 = idc1[i][2] + idc1[i][3];
		d23 = idc1[i][2] - idc1[i][3];

		idc2[i][0] = s01 + s23;
		idc2[i][1] = s01 - s23;
		idc2[i][2] = d01 - d23;
		idc2[i][3] = d01 + d23;
	}
}
static void dequant_4x4_dc( int idc2[4][4], int dequant_mf[4][4], int dequant_dc[4][4], int qp )
{
	int i;
	int i_qbits = qp/6 - 2 ;
	for( i = 0; i < 16; i++ )
	{
		if (i_qbits>=0)
			dequant_dc[0][i] = (idc2[0][i] * dequant_mf[0][0]) << (i_qbits);
		else
			dequant_dc[0][i] = (idc2[0][i] * dequant_mf[0][0]) >> (-i_qbits);
	}
}

static void dyn_dc44( int quant_mf[52][4][4], int dequant_mf[52][4][4] )
{
	int dc_level = 4080;
	dyn_t dyn_dc_q={},dyn_idc1={},dyn_idc2={},dyn_dc_dq={};
	int max_val, max_qp;

	int sign[4][4];
	int hadamard2[4][4];
	int quant_dc[52][4][4];
	int idc1[52][4][4];
	int idc2[52][4][4];
	int dequant_dc[52][4][4];

	for(int i = 0;i<0x10000;i++)
	{
		signMatrix( dc_level, i, sign );
		dc_trans(sign, 0, hadamard2);
		for (int qp=0;qp<52;qp++)
		{
			quant_4x4_dc(hadamard2, quant_mf[qp], quant_dc[qp], qp);
			dc_trans_inv(quant_dc[qp], idc1[qp], idc2[qp]);
			dequant_4x4_dc(idc2[qp], dequant_mf[qp], dequant_dc[qp], qp);
		}

		max_val = maxItem( quant_dc, &max_qp );
		if (max_val>dyn_dc_q.val)
		{
			dyn_dc_q.val = max_val;
			dyn_dc_q.qp = max_qp;
			dyn_dc_q.sign = i;
		}

		max_val = maxItem( idc1, &max_qp );
		if (max_val>dyn_idc1.val)
		{
			dyn_idc1.val = max_val;
			dyn_idc1.qp = max_qp;
			dyn_idc1.sign = i;
		}

		max_val = maxItem( idc2, &max_qp );
		if (max_val>dyn_idc2.val)
		{
			dyn_idc2.val = max_val;
			dyn_idc2.qp = max_qp;
			dyn_idc2.sign = i;
		}

		max_val = maxItem( dequant_dc, &max_qp );
		if (max_val>dyn_dc_dq.val)
		{
			dyn_dc_dq.val = max_val;
			dyn_dc_dq.qp = max_qp;
			dyn_dc_dq.sign = i;
		}
	}

	// 量化的最大值
	printf( "*********************************************\n");
	printf( "\tmax_dc_quant=%d, qp=%d\n", dyn_dc_q.val, dyn_dc_q.qp );
	printf( "*********************************************\n");
	max_qp = dyn_dc_q.qp;
	signMatrix( dc_level, dyn_dc_q.sign, sign );
	dc_trans(sign, 0, hadamard2);
	quant_4x4_dc(hadamard2, quant_mf[max_qp], quant_dc[max_qp], max_qp);
	dc_trans_inv(quant_dc[max_qp], idc1[max_qp], idc2[max_qp]);
	dequant_4x4_dc(idc2[max_qp], dequant_mf[max_qp], dequant_dc[max_qp], max_qp);
	show_matrix("sign",sign);
	show_matrix("hadamard2",hadamard2);
	show_matrix("quant_dc",quant_dc[max_qp]);
	show_matrix("idc1",idc1[max_qp]);
	show_matrix("idc2",idc2[max_qp]);
	show_matrix("dequant_dc",dequant_dc[max_qp]);

	// 反Hadamard1的最大值
	printf( "*********************************************\n");
	printf( "\tmax_idc1=%d, qp=%d\n", dyn_idc1.val, dyn_idc1.qp );
	printf( "*********************************************\n");
	max_qp = dyn_idc1.qp;
	signMatrix( dc_level, dyn_dc_dq.sign, sign );
	dc_trans(sign, 0, hadamard2);
	quant_4x4_dc(hadamard2, quant_mf[max_qp], quant_dc[max_qp], max_qp);
	dc_trans_inv(quant_dc[max_qp], idc1[max_qp], idc2[max_qp]);
	dequant_4x4_dc(idc2[max_qp], dequant_mf[max_qp], dequant_dc[max_qp], max_qp);
	show_matrix("sign",sign);
	show_matrix("hadamard2",hadamard2);
	show_matrix("quant_dc",quant_dc[max_qp]);
	show_matrix("idc1",idc1[max_qp]);
	show_matrix("idc2",idc2[max_qp]);
	show_matrix("dequant_dc",dequant_dc[max_qp]);

	// 反Hadamard2的最大值
	printf( "*********************************************\n");
	printf( "\tmax_idc2=%d, qp=%d\n", dyn_idc2.val, dyn_idc2.qp );
	printf( "*********************************************\n");
	max_qp = dyn_idc2.qp;
	signMatrix( dc_level, dyn_dc_dq.sign, sign );
	dc_trans(sign, 0, hadamard2);
	quant_4x4_dc(hadamard2, quant_mf[max_qp], quant_dc[max_qp], max_qp);
	dc_trans_inv(quant_dc[max_qp], idc1[max_qp], idc2[max_qp]);
	dequant_4x4_dc(idc2[max_qp], dequant_mf[max_qp], dequant_dc[max_qp], max_qp);
	show_matrix("sign",sign);
	show_matrix("hadamard2",hadamard2);
	show_matrix("quant_dc",quant_dc[max_qp]);
	show_matrix("idc1",idc1[max_qp]);
	show_matrix("idc2",idc2[max_qp]);
	show_matrix("dequant_dc",dequant_dc[max_qp]);

	// 反量化后的最大值
	printf( "*********************************************\n");
	printf( "\tmax_dc_dequant=%d, qp=%d\n", dyn_dc_dq.val, dyn_dc_dq.qp );
	printf( "*********************************************\n");
	max_qp = dyn_dc_dq.qp;
	signMatrix( dc_level, dyn_dc_dq.sign, sign );
	dc_trans(sign, 0, hadamard2);
	quant_4x4_dc(hadamard2, quant_mf[max_qp], quant_dc[max_qp], max_qp);
	dc_trans_inv(quant_dc[max_qp], idc1[max_qp], idc2[max_qp]);
	dequant_4x4_dc(idc2[max_qp], dequant_mf[max_qp], dequant_dc[max_qp], max_qp);
	show_matrix("sign",sign);
	show_matrix("hadamard2",hadamard2);
	show_matrix("quant_dc",quant_dc[max_qp]);
	show_matrix("idc1",idc1[max_qp]);
	show_matrix("idc2",idc2[max_qp]);
	show_matrix("dequant_dc",dequant_dc[max_qp]);
}

static void dyn_dc22( int quant_mf[52][4][4], int dequant_mf[52][4][4] )
{
	int dc_level = 16320;
	int quant[52]={};
	int dequant[52]={};
	dyn_t dc_quant={},dc_dequant={};

	for (int qp=0;qp<52;qp++)
	{
		int i_qbits = 16 + qp/6;
		int f = 1<<i_qbits;
		f = f/2;

		quant[qp] = ( f + dc_level * quant_mf[qp][0][0] ) >> (i_qbits);
		if (quant[qp]>dc_quant.val)
		{
			dc_quant.val = quant[qp];
			dc_quant.qp = qp;
		}

		i_qbits = qp/6 - 1 ;
		if (i_qbits>=0)
			dequant[qp] = (quant[qp] * dequant_mf[qp][0][0]) << (i_qbits);
		else
			dequant[qp] = (quant[qp] * dequant_mf[qp][0][0]) >> (-i_qbits);

		if (dequant[qp]>dc_dequant.val)
		{
			dc_dequant.val = dequant[qp];
			dc_dequant.qp = qp;
		}
	}

	printf( "*********************************************\n");
	printf( "\tmax_dc22_quant=%d, qp=%d\n", dc_quant.val, dc_quant.qp );
	printf( "\tmax_dc22_dequant=%d, qp=%d\n", dc_dequant.val, dc_dequant.qp );
	printf( "*********************************************\n");
}

static void dyn_ac( int quant_mf[52][4][4], int dequant_mf[52][4][4] )
{
	int ac_max = 255;
	dyn_t dyn_dq={},dyn_idct1={},dyn_idct2={};
	int max_val, max_qp;

	int sign[4][4];
	int dct[4][4];
	int quant[52][4][4];
	int dequant[52][4][4];
	int idct1[52][4][4];
	int idct2[52][4][4];

	for(int i = 0;i<0x10000;i++)
	{
		signMatrix( ac_max, i, sign );
		dctCore(sign, dct, 0);
		for (int qp=0;qp<52;qp++)
		{
			quant_4x4(dct, quant_mf[qp], quant[qp], qp);
			dequant_4x4(quant[qp], dequant_mf[qp], dequant[qp], qp);
			idctCore(dequant[qp], idct1[qp], idct2[qp]);
		}

		max_val = maxItem( dequant, &max_qp );
		if (max_val>dyn_dq.val)
		{
			dyn_dq.val = max_val;
			dyn_dq.qp = max_qp;
			dyn_dq.sign = i;
		}

		max_val = maxItem( idct1, &max_qp );
		if (max_val>dyn_idct1.val)
		{
			dyn_idct1.val = max_val;
			dyn_idct1.qp = max_qp;
			dyn_idct1.sign = i;
		}

		max_val = maxItem( idct2, &max_qp );
		if (max_val>dyn_idct2.val)
		{
			dyn_idct2.val = max_val;
			dyn_idct2.qp = max_qp;
			dyn_idct2.sign = i;
		}
	}

	// 反量化后的最大值
	printf( "*********************************************\n");
	printf( "\tmax_dequant=%d, qp=%d\n", dyn_dq.val, dyn_dq.qp );
	printf( "*********************************************\n");
	max_qp = dyn_dq.qp;
	signMatrix( ac_max, dyn_dq.sign, sign );
	dctCore(sign, dct, 0);
	quant_4x4(dct, quant_mf[max_qp], quant[max_qp], max_qp);
	dequant_4x4(quant[max_qp], dequant_mf[max_qp], dequant[max_qp], max_qp);
	idctCore(dequant[max_qp], idct1[max_qp], idct2[max_qp]);
	show_matrix("sign",sign);
	show_matrix("dct",dct);
	show_matrix("quant",quant[max_qp]);
	show_matrix("dequant",dequant[max_qp]);
	show_matrix("idct1",idct1[max_qp]);
	show_matrix("idct2",idct2[max_qp]);

	// 反变换第一步（反变换矩阵左乘反量化矩阵）的最大值
	printf( "\n*********************************************\n");
	printf( "\tmax_idct1=%d, qp=%d\n", dyn_idct1.val, dyn_idct1.qp );
	printf( "*********************************************\n");
	max_qp = dyn_idct1.qp;
	signMatrix( ac_max, dyn_idct1.sign, sign );
	dctCore(sign, dct, 0);
	quant_4x4(dct, quant_mf[max_qp], quant[max_qp], max_qp);
	dequant_4x4(quant[max_qp], dequant_mf[max_qp], dequant[max_qp], max_qp);
	idctCore(dequant[max_qp], idct1[max_qp], idct2[max_qp]);
	show_matrix("sign",sign);
	show_matrix("dct",dct);
	show_matrix("quant",quant[max_qp]);
	show_matrix("dequant",dequant[max_qp]);
	show_matrix("idct1",idct1[max_qp]);
	show_matrix("idct2",idct2[max_qp]);

	// 反变换后的最大值
	printf( "\n*********************************************\n");
	printf( "\tmax_idct2=%d, qp=%d\n", dyn_idct2.val, dyn_idct2.qp );
	printf( "*********************************************\n");
	max_qp = dyn_idct2.qp;
	signMatrix( ac_max, dyn_idct2.sign, sign );
	dctCore(sign, dct, 0);
	quant_4x4(dct, quant_mf[max_qp], quant[max_qp], max_qp);
	dequant_4x4(quant[max_qp], dequant_mf[max_qp], dequant[max_qp], max_qp);
	idctCore(dequant[max_qp], idct1[max_qp], idct2[max_qp]);
	show_matrix("sign",sign);
	show_matrix("dct",dct);
	show_matrix("quant",quant[max_qp]);
	show_matrix("dequant",dequant[max_qp]);
	show_matrix("idct1",idct1[max_qp]);
	show_matrix("idct2",idct2[max_qp]);
}

/**
 * show dynamic range (max value) in h264 4x4-DCT
 */
int idct4_dyn(int argc, char* argv[])
{
	int quant_mf[52][4][4];
	int dequant_mf[52][4][4];
	for (int qp=0;qp<52;qp++)
		QX(qp,quant_mf[qp],dequant_mf[qp]);

	dyn_dc44( quant_mf, dequant_mf );
	dyn_ac( quant_mf, dequant_mf );
	//showQuantMx();

	return 0;
}
