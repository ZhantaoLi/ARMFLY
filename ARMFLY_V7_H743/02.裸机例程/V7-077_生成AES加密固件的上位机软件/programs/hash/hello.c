/*
 *  Classic "Hello, world" demonstration program
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdlib.h>
#include <stdio.h>
#define mbedtls_printf       printf
#define mbedtls_exit         exit
#define MBEDTLS_EXIT_SUCCESS EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE EXIT_FAILURE
#endif

#if defined(MBEDTLS_MD5_C)
#include "mbedtls/md5.h"
#endif

#if !defined(MBEDTLS_MD5_C)
int main(void)
{
	mbedtls_printf("MBEDTLS_MD5_C not defined.\n");
	return(0);
}
#else

#include "mbedtls/aes.h"
#include "mbedtls/platform_util.h"

unsigned char key[16] =
{
	0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
unsigned char iv[16] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

unsigned char iv1[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

mbedtls_aes_context aes;

/// <summary>
/// ������Ҫ�޸ģ�ע��Դ�ļ�Ҫ��16�ֽ�����������������������ʹ��������Hex Editor Neo��0
/// </summary>
char SourceFile[] = "C:\\Users\\51246\\Desktop\\AES\\app.bin";     /* Դ�ļ�·�� */
char DestFile[] = "C:\\Users\\51246\\Desktop\\AES\\aes.bin";       /* ���ܺ��ļ�·�� */
char ReSourceFile[] = "C:\\Users\\51246\\Desktop\\AES\\app1.bin";  /* ���ܵ��ļ��ٽ����·�� */

int main(void)
{
	int i, ret;
	FILE* fin, * fout;
	int size = 0;
	int bw = 0;
	int readcount = 0, remain = 0;

	mbedtls_aes_init(&aes);

	mbedtls_printf("IVֵ��\r\n");
	for (int i = 0; i < 16; i++)
	{
		mbedtls_printf("%02x ", iv[i]);
	}
	mbedtls_printf("\r\n");

	mbedtls_printf("KEYֵ��\r\n");
	for (int i = 0; i < 16; i++)
	{
		mbedtls_printf("%02x ", key[i]);
	}
	mbedtls_printf("\r\n=======================================================\r\n");

	/************************************��ȡ�ļ�*************************************/
	fin = fopen(SourceFile, "rb");
	if (fin != NULL)
	{
		/* �ļ��򿪳ɹ� */
		mbedtls_printf("���ļ��ɹ�\r\n");
	}
	else
	{
		mbedtls_printf("���ļ�ʧ��, �����ļ�������\r\n");
		goto exit;
	}

	fseek(fin, 0, SEEK_END);
	size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	mbedtls_printf("�ļ���С = %d\r\n", size);

	char* input = malloc(size);
	char* output = malloc(size);

	do
	{
		bw = fread(&input[readcount], 1, size, fin);
		readcount += bw;
	} while (readcount < size);

	mbedtls_printf("readcount = %d\r\n", readcount);
	fclose(fin);

	/************************************���ܲ���*************************************/
	mbedtls_aes_init(&aes);
	mbedtls_aes_setkey_enc(&aes, key, 128);
	mbedtls_aes_crypt_cbc(&aes, 1, size, iv, input, output);

	fout = fopen(DestFile, "wb");
	if (fwrite((char*)output, 1, size, fout) != size)
	{
		mbedtls_fprintf(stderr, "fwrite(%ld bytes) failed\n", (long)size);
		goto exit;
	}
	{
		mbedtls_printf("�������\r\n");
	}
	fclose(fout);

	/************************************���ܲ���*************************************/
	mbedtls_aes_setkey_dec(&aes, key, 128);
	mbedtls_aes_crypt_cbc(&aes, 0, size, iv1, &output[0], &input[0]);

	fout = fopen(ReSourceFile, "wb");
	if (fwrite((char*)input, 1, size, fout) != size)
	{
		mbedtls_fprintf(stderr, "fwrite(%ld bytes) failed\n", (long)size);
		goto exit;
	}
	{
		mbedtls_printf("�������\r\n");
	}
	fclose(fout);

	/************************************�Ƚ��Ƿ���ͬ*************************************/
	fin = fopen(SourceFile, "rb");
	fread(&output[0], 1, size, fin);
	fclose(fin);
	if (memcmp(input, output, size) == 0)
	{
		mbedtls_printf("�ӽ��ܳɹ�\r\n");
	}
	else
	{
		mbedtls_printf("�ӽ���ʧ��\r\n");
	}

exit:
	if (fin != NULL)
	{
		fclose(fin);
	}

	if (fout != NULL)
	{
		fclose(fout);
	}

	free((char*)input);
	free((char*)output);

	return(MBEDTLS_EXIT_SUCCESS);

}
#endif /* MBEDTLS_MD5_C */
