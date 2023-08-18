/*
*********************************************************************************************************
*
*	模块名称 : 中值滤波器
*	文件名称 : MidFilter.c
*	版    本 : V1.0
*	说    明 : 中值滤波器实现
*	修改记录 :
*		版本号  日期         作者       说明
*		V1.0    2021-09-05  Eric2013   正式发布
*
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"			 /* 底层硬件驱动 */
#include "arm_math.h"



#define TEST_LENGTH_SAMPLES  1024    		/* 采样点数 */

#define MidFilterOrder  16     				/* 滤波阶数 */
#define ZeroSize        MidFilterOrder

float32_t DstDate[TEST_LENGTH_SAMPLES];     /* 滤波后数据 */

static float32_t SortData[MidFilterOrder];  /* 滤波排序 */ 
static float32_t TempDate[ZeroSize + TEST_LENGTH_SAMPLES +ZeroSize] = {0}; /* 滤波阶段用到的临时变量 */

const float32_t testdata[1024]=
{
0.268834,
0.916943,
-1.129423,
0.431087,
0.159383,
-0.653844,
-0.216796,
0.171312,
1.789198,
1.384719,
-0.674943,
1.517462,
0.362702,
-0.031527,
0.357371,
-0.102483,
-0.062072,
0.744849,
0.704517,
0.708596,
0.335749,
-0.603743,
0.358619,
0.815118,
0.244447,
0.517347,
0.363443,
-0.151720,
0.146936,
-0.393641,
0.444198,
-0.573535,
-0.534435,
-0.404749,
-1.472142,
0.719190,
0.162595,
-0.377464,
0.685149,
-0.855758,
-0.051121,
-0.120724,
0.159603,
0.156429,
-0.432440,
-0.015026,
-0.082440,
0.313854,
0.546633,
0.554637,
-0.431826,
0.038680,
-0.607059,
-0.556750,
-0.003425,
0.766315,
-0.384833,
0.185689,
-0.112792,
0.558678,
-0.544532,
0.016279,
0.276264,
0.550305,
0.772106,
0.042966,
-0.745795,
-0.371151,
-0.530791,
1.175229,
-0.307801,
0.374038,
-0.096209,
0.444305,
-0.382425,
-0.701134,
-0.711188,
0.244097,
-0.088688,
-0.098027,
0.709655,
0.145792,
0.098906,
0.793850,
-0.402233,
0.348312,
0.417544,
-0.121858,
0.107835,
-0.582922,
-0.573976,
0.052437,
0.361127,
1.292746,
-0.333445,
0.093666,
-0.041247,
-0.966511,
-0.219483,
-0.897339,
20.420188,
19.555984,
20.050046,
19.727736,
20.151760,
19.699837,
20.244983,
20.369682,
20.855944,
19.902938,
18.930822,
19.580206,
20.677297,
19.463922,
20.480477,
20.062025,
20.718348,
19.019550,
19.901151,
19.396077,
-0.545996,
-1.587391,
-1.310514,
-2.529090,
-2.234308,
-2.136235,
-1.450788,
-2.138936,
-1.649229,
-3.025908,
-2.176925,
-2.411793,
-2.788529,
-1.746013,
-1.859008,
-1.983260,
-2.666839,
-1.436254,
-1.824910,
-2.149533,
-1.988555,
-2.130998,
-2.875106,
-2.142825,
-2.415683,
-2.489603,
-2.578201,
-2.266779,
-3.001318,
-1.517885,
5.260030,
4.989986,
4.982614,
4.600918,
5.509343,
4.933391,
4.642735,
5.675693,
4.887614,
4.705485,
4.853123,
4.576037,
4.439936,
6.263000,
5.827749,
5.153768,
4.371441,
4.567266,
4.911733,
5.395708,
4.333998,
3.835066,
4.275451,
5.166755,
5.195677,
5.225840,
4.934858,
5.091845,
4.761923,
5.431011,
4.319153,
5.227515,
4.575645,
4.832557,
5.276392,
5.519545,
4.441181,
5.630329,
5.330072,
4.966067,
4.902389,
4.891197,
4.848446,
5.011523,
5.025645,
5.413031,
5.763488,
5.233457,
4.895143,
5.312595,
5.091614,
4.485116,
5.474611,
5.153531,
5.067587,
5.257623,
5.130703,
4.529257,
4.918831,
4.926973,
4.733994,
5.841052,
4.562135,
4.758092,
4.643998,
4.412894,
4.903880,
4.862965,
5.765036,
4.875488,
4.467893,
5.801729,
5.617340,
4.885187,
4.246920,
4.777686,
4.922029,
5.138034,
4.869418,
5.221711,
-4.804053,
-5.625339,
-5.473980,
-5.370553,
-5.253909,
-5.160288,
-4.993765,
-6.514589,
-5.228507,
-4.378776,
-5.533351,
-4.533136,
-4.824839,
-5.014503,
-4.908774,
-5.782528,
-5.042270,
-4.198027,
-4.950826,
-4.979313,
-5.367085,
-5.015407,
-4.883826,
-4.786806,
-5.186404,
-5.118227,
-3.988155,
-6.129177,
-3.885277,
-4.831218,
9.500030,
8.167918,
8.704983,
8.860968,
9.211358,
8.164900,
9.235817,
8.393576,
9.033095,
9.326178,
9.163530,
9.541317,
9.503039,
8.674546,
9.128528,
8.527811,
8.339106,
9.462413,
9.000025,
8.972541,
9.455564,
9.297292,
9.175101,
9.625126,
9.464895,
9.119882,
8.654819,
8.674223,
9.596051,
8.194085,
8.987769,
8.025576,
9.510249,
9.430858,
9.000581,
8.964581,
7.756858,
9.290586,
7.903783,
7.840360,
9.039967,
8.525760,
9.205745,
9.338489,
9.428866,
8.654420,
9.224689,
9.050317,
9.413035,
9.268079,
9.448944,
8.934031,
8.926399,
9.503887,
7.938172,
8.747707,
8.364703,
8.808708,
9.324340,
9.412864,
8.492528,
8.764465,
9.068512,
8.854068,
9.150909,
9.199965,
8.535019,
8.911585,
7.933953,
9.572681,
8.685455,
8.398075,
8.873028,
8.285677,
8.989571,
8.719668,
10.088889,
9.569233,
7.751557,
9.220663,
8.300931,
8.872472,
9.082202,
9.373867,
8.863477,
9.788150,
8.759531,
9.163756,
9.332367,
9.042594,
9.440476,
9.161607,
8.607927,
8.097313,
9.929296,
8.697735,
9.051680,
9.281583,
9.056798,
8.547637,
8.766143,
8.937555,
9.739479,
8.569592,
9.392334,
9.154312,
8.883070,
8.471514,
8.857930,
8.956655,
8.265302,
9.096091,
8.588853,
8.952880,
9.168107,
8.547673,
8.855872,
9.175031,
8.082070,
9.517988,
10.212231,
9.479700,
8.842114,
9.214311,
8.482008,
9.938933,
9.470352,
9.393673,
8.562063,
9.159975,
8.720853,
8.844285,
8.714995,
8.487133,
8.545627,
8.895051,
8.150568,
9.303800,
8.941101,
9.349580,
-3.865176,
-3.752856,
-4.741561,
-4.510132,
-4.223498,
-3.945171,
-3.435632,
-4.144982,
-3.369225,
-3.762288,
-3.412942,
-3.936526,
-4.328408,
-4.740700,
-3.922256,
-3.590724,
-4.146294,
-4.270393,
-4.154321,
-4.548297,
-4.246505,
-4.090370,
-3.977079,
-4.031892,
-3.694332,
-3.945341,
-3.092992,
-3.843988,
-3.097753,
-4.361561,
-3.736726,
-4.130125,
-3.699929,
-3.703035,
-5.093011,
-4.663522,
-4.720507,
-3.799078,
-3.264899,
-4.163407,
3.406162,
3.272770,
2.474184,
3.198733,
2.624053,
3.758133,
2.983717,
3.818000,
2.787471,
3.294717,
2.968604,
1.989021,
2.508934,
3.306256,
2.972557,
2.440634,
2.686811,
3.124759,
2.503490,
3.487475,
2.679645,
3.904431,
2.460067,
3.099595,
2.239487,
2.638184,
2.703375,
3.200668,
3.471067,
3.150243,
2.813465,
3.407744,
3.399443,
3.060103,
3.285624,
3.206398,
2.506519,
3.379784,
2.671399,
2.698041,
3.088473,
2.846248,
2.934090,
3.297679,
3.523416,
2.901021,
3.163839,
2.880849,
3.114798,
3.219999,
2.691567,
3.137418,
3.300551,
3.046154,
3.864921,
2.695721,
2.631470,
2.125060,
3.455241,
3.433541,
2.960054,
3.449238,
3.091852,
3.145395,
3.056472,
3.219976,
3.050831,
4.393668,
2.416667,
2.072850,
2.429659,
2.453328,
2.783195,
2.915765,
2.890733,
3.270667,
3.194633,
3.375614,
3.889128,
3.611531,
2.358372,
1.835523,
3.450966,
2.082181,
3.033378,
3.017740,
4.113584,
2.965393,
2.746338,
3.117905,
3.122902,
3.035023,
2.695710,
2.388703,
3.158250,
2.328565,
2.483908,
3.665608,
2.790548,
2.929839,
3.449911,
2.849944,
3.514683,
2.827467,
3.506401,
3.314667,
2.893492,
2.567151,
2.478446,
2.864966,
2.780929,
2.795663,
3.491773,
2.851151,
3.571839,
2.734190,
3.486283,
2.738875,
3.088289,
3.485369,
2.793014,
2.780865,
4.001695,
3.475497,
2.783998,
3.324470,
2.819962,
3.352943,
3.707925,
2.197742,
3.514427,
3.728984,
3.023736,
3.873128,
3.077694,
2.381440,
1.903253,
2.833296,
3.356772,
3.158704,
3.206805,
2.711457,
3.072001,
2.180667,
2.619955,
2.590603,
3.259864,
2.992920,
2.422235,
2.995238,
2.655095,
2.666650,
3.432075,
3.056710,
3.199181,
3.441985,
3.090129,
3.275427,
3.341482,
3.585304,
3.237930,
3.706116,
3.011304,
2.976065,
3.850667,
2.745144,
2.998573,
3.459934,
3.074904,
3.702467,
3.517061,
3.145785,
2.611151,
3.283348,
2.308689,
3.122237,
3.404219,
3.106521,
3.439839,
4.019438,
3.461966,
3.133459,
3.320831,
3.212743,
2.342638,
2.791794,
3.612344,
2.978208,
3.291212,
2.496750,
3.032258,
3.300146,
2.319243,
3.173796,
2.909078,
2.530233,
2.981233,
2.051848,
1.936012,
2.411538,
2.504734,
2.413484,
2.137286,
3.144114,
2.202908,
3.055109,
3.393533,
2.998887,
3.046554,
2.810921,
2.258662,
2.978091,
3.480413,
3.869122,
2.784897,
2.186339,
3.083174,
3.188133,
2.886525,
2.425544,
13.012166,
10.820238,
11.745014,
11.339187,
11.681936,
12.158926,
12.069024,
11.644632,
12.388502,
12.311197,
12.323690,
11.787184,
12.524290,
12.330354,
13.254386,
12.531730,
12.578461,
12.026489,
11.355807,
11.814389,
11.621104,
11.718016,
12.277569,
11.721611,
11.552443,
11.795336,
11.919557,
12.204667,
11.523682,
12.158659,
12.039010,
12.662193,
11.893415,
11.932761,
11.414322,
11.307369,
12.155254,
11.875255,
12.251872,
11.553669,
12.954256,
12.061115,
12.523517,
11.886540,
11.918749,
12.345026,
12.277878,
11.439872,
11.233653,
11.451066,
11.292113,
12.029785,
11.794375,
11.815995,
11.319518,
12.389784,
12.219706,
11.955189,
12.510590,
11.563010,
12.207350,
12.174221,
12.174627,
11.635376,
12.163420,
11.742559,
11.551777,
11.398366,
12.518908,
11.577028,
11.913543,
11.395674,
11.851437,
10.383981,
11.456520,
11.286782,
11.492775,
11.893366,
11.837326,
12.972199,
11.714113,
11.874984,
11.215342,
11.761309,
11.331012,
12.015150,
12.426543,
12.202127,
11.649690,
11.184729,
12.730007,
13.025021,
12.060250,
11.505049,
12.598886,
11.703672,
11.765095,
12.443189,
11.307390,
11.021623,
5.210342,
5.200369,
5.047571,
5.248342,
5.541120,
5.485224,
4.715715,
5.404986,
5.086624,
4.747229,
4.403347,
5.323485,
4.823189,
5.023217,
4.603526,
4.224743,
5.085793,
4.968930,
5.599514,
5.400852,
25.526652,
24.625562,
24.531837,
24.365457,
25.248990,
26.394541,
25.363786,
24.613468,
25.418317,
24.435835,
24.287765,
25.358721,
24.611047,
25.157993,
25.703268,
25.200562,
25.464830,
24.197099,
25.330768,
26.069251,
25.270570,
24.229561,
24.898429,
24.750017,
25.191512,
25.206018,
25.202746,
24.818110,
24.700364,
24.705206,
7.426770,
6.073496,
6.896348,
7.135189,
6.673614,
7.238614,
6.964340,
6.530849,
7.080682,
6.865909,
6.795064,
6.644339,
7.030723,
6.076935,
6.800833,
6.728226,
6.544051,
7.326349,
6.632864,
7.270317,
7.487920,
6.921565,
7.138900,
7.319759,
6.959511,
7.270435,
6.368718,
7.555212,
6.505219,
6.085582,
7.692249,
6.968637,
7.224461,
6.818371,
6.489708,
5.463506,
7.313139,
6.856658,
6.901329,
7.202803,
6.290326,
6.635277,
7.573664,
7.298932,
6.359359,
5.898368,
6.714377,
7.106998,
7.471188,
7.046863,
6.438844,
7.153079,
6.413833,
6.519517,
6.673132,
6.385303,
6.864517,
6.550025,
6.857157,
6.768789,
6.795107,
6.748231,
7.616649,
7.305153,
7.029536,
6.266527,
6.187098,
6.017624,
8.302598,
7.486187,
7.128490,
6.512880,
6.426818,
7.273820,
7.782542,
6.153328,
6.775301,
6.957854,
6.004001,
7.420623,
6.792671,
7.956090,
6.804551,
7.204591,
6.428786,
6.687568,
6.415639,
7.196288,
7.650920,
6.703179,
7.218188,
6.747819,
7.051054,
7.598125,
7.060141,
6.481578,
6.571448,
6.915063,
6.904166,
6.567092,
7.090332,
7.633264,
6.874415,
6.897715,
5.899239,
6.612743,
6.303364,
6.806883,
7.262793,
7.761635,
7.899247,
6.941558,
6.839902,
7.408758,
7.245080,
7.382626,
7.389140,
6.259847,
7.270182,
6.954230,
6.619874,
6.653202,
7.640729,
6.595131,
6.381591,
7.107343,
8.005386,
7.012777,
7.154150,
6.530876,
7.837108,
7.062494,
7.265051,
6.523966,
7.427021,
7.194573,
6.421999,
7.019870,
6.774701,
7.054624,
6.874724,
6.905049,
6.483543,
6.838354,
7.383263,
7.872337,
6.419740,
8.188706,
7.763039,
7.084254,
6.849397,
6.650673,
7.416385,
6.652697,
6.769059,
7.441809,
7.217972,
7.448374,
7.252366,
6.799551,
6.743076,
7.398184,
6.664405,
7.593330,
7.395351,
7.143861,
7.001613,
7.182809,
8.763339,
6.943782,
6.221703,
7.957551,
7.304923,
6.676044,
8.308667,
7.275475,
7.147102,
6.611078,
6.467535,
6.115793,
6.788540,
6.473449,
7.323878,
6.841186,
7.884496,
7.755291,
7.082005,
6.858618,
7.576083,
6.426746,
7.336849,
6.665444,
6.799839,
6.664099,
7.287815,
6.610953,
6.468219,
7.276489,
6.788286,
7.180794,
6.824055,
7.134770,
5.717775,
7.232932,
7.926780,
7.519645,
7.455448,
6.880134,
7.090499,
7.122125,
7.048196,
6.584766,
6.823874,
6.912612,

};

/*
*********************************************************************************************************
*	函 数 名: MidFilterBlock
*	功能说明: 中值滤波器，对一段数据的中值滤波。
*	形    参: pSrc 源数据地址。
*             pDst 滤波后数据地址。
*             blockSize 数据个数，至少为2。
*             order 至少2阶。
*	返 回 值: 无
*********************************************************************************************************
*/
void MidFilterBlock(float32_t *pSrc, float32_t *pDst, uint32_t blockSize, uint32_t order)
{
	arm_sort_instance_f32 S;
	uint32_t N, i;
	
	
	S.dir = ARM_SORT_ASCENDING;
	S.alg = ARM_SORT_QUICK;
	
	
	N = order / 2;
	
	/* 数据幅值给临时缓冲 */
	for(i =0; i < blockSize; i++)
	{
		TempDate[i + ZeroSize] = pSrc[i];
	}
	
	
	/* 求每个数据点的中值 */
	for(i =0; i < blockSize; i++)
	{
		/* 排序 */
		arm_sort_f32(&S, &TempDate[i + ZeroSize - N], &SortData[0], order);
		
		/* 奇数 */
		if(N)
		{
			pDst[i] = SortData[N];
		}
		/* 偶数 */
		else
		{
			pDst[i] = (SortData[N] + SortData[N-1])/2;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MidFilterRT
*	功能说明: 中值滤波器，用于逐个数据实时滤波。
*	形    参: pSrc 源数据地址。
*             pDst 滤波后数据地址。
*             ucFlag 1表示首次滤波，后面继续滤波，需将其设置为0。
*             order 至少2阶。
*	返 回 值: 无
*********************************************************************************************************
*/
void MidFilterRT(float32_t *pSrc, float32_t *pDst, uint8_t ucFlag, uint32_t order)
{
	arm_sort_instance_f32 S;
	uint32_t N, i;
	
	static uint32_t CountFlag = 0;
	
	
	S.dir = ARM_SORT_ASCENDING;
	S.alg = ARM_SORT_QUICK;
	
	N = order / 2;
	
	/* 首次滤波先清零 */
	if(ucFlag == 1)
	{
		CountFlag = 0;
	}
	
	/* 填充数据 */
	if(CountFlag < order)
	{
		TempDate[CountFlag] = pSrc[0];
		CountFlag++;
	}
	else
	{
		for(i =0; i < order - 1; i++)
		{
			TempDate[i] = TempDate[1 + i];  
		}
		TempDate[order - 1] = pSrc[0];
	}
	
	/* 排序 */
	arm_sort_f32(&S, &TempDate[0], &SortData[0], order);
	
	/* 奇数 */
	if(N)
	{
		pDst[0] = SortData[N];
	}
	/* 偶数 */
	else
	{
		pDst[0] = (SortData[N] + SortData[N-1])/2;
	}
}

/*
*********************************************************************************************************
*	函 数 名: MidFilterBlockTest
*	功能说明: 整块数据滤波测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MidFilterBlockTest(void)
{

	MidFilterBlock((float32_t *)&testdata[0], &DstDate[0], TEST_LENGTH_SAMPLES, MidFilterOrder);

	for(int i = 0; i < TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f\r\n", testdata[i], DstDate[i]);
	}
}

/*
*********************************************************************************************************
*	函 数 名: MidFilterOneByOneTest
*	功能说明: 逐个数据滤波测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MidFilterOneByOneTest(void)
{
	float32_t  *inputF32, *outputF32;
	
	inputF32 = (float32_t  *)&testdata[0];
	outputF32 = &DstDate[0];
	
	/* 从头开始，先滤第1个数据 */
	MidFilterRT(inputF32 , outputF32, 1, MidFilterOrder);
	
	/* 逐次滤波后续数据 */
	for(int i = 1; i < TEST_LENGTH_SAMPLES; i++)
	{
		MidFilterRT(inputF32 + i , outputF32 + i, 0, MidFilterOrder);
	}
	
	for(int i = 0; i < TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f\r\n", testdata[i], DstDate[i]);
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
