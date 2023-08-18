/*
*********************************************************************************************************
*
*	ģ������ : ��̬�ڴ����
*	�ļ����� : rtx_memory.h
*	��    �� : V1.0
*	˵    �� : ��RTX5�Ķ�̬�ڴ�����������, ���Թ������ڴ��
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2018-04-09   Eric2013   ��RTX5�Ķ�̬�ڴ�����������
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
/*
 * Copyright (c) 2013-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * -----------------------------------------------------------------------------
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       RTX Library definitions
 *
 * -----------------------------------------------------------------------------
 */

#ifndef RTX_LIB_H_
#define RTX_LIB_H_

#include <string.h>
#include <stdint.h>
#include "stm32h7xx.h"


//  Memory Pool Header structure
typedef struct {
  uint32_t size;                // Memory Pool size
  uint32_t used;                // Used Memory
} mem_head_t;

//  Memory Block Header structure
typedef struct mem_block_s {
  struct mem_block_s *next;     // Next Memory Block in list
  uint32_t            info;     // Block Info or max used Memory (in last block)
} mem_block_t;

//  Memory Block Info: Length = <31:2>:'00', Type = <1:0>
#define MB_INFO_LEN_MASK        0xFFFFFFFCU     // Length mask
#define MB_INFO_TYPE_MASK       0x00000003U     // Type mask

//  Memory Head Pointer
__STATIC_INLINE mem_head_t *MemHeadPtr (void *mem) {
  //lint -e{9079} -e{9087} "conversion from pointer to void to pointer to other type" [MISRA Note 6]
  return ((mem_head_t *)mem);
}

//  Memory Block Pointer
__STATIC_INLINE mem_block_t *MemBlockPtr (void *mem, uint32_t offset) {
  uint32_t     addr;
  mem_block_t *ptr;

  //lint --e{923} --e{9078} "cast between pointer and unsigned int" [MISRA Note 8]
  addr = (uint32_t)mem + offset;
  ptr  = (mem_block_t *)addr;

  return ptr;
}

//  ==== Library functions ====

// Memory Heap Library functions
extern uint32_t osRtxMemoryInit (void *mem, uint32_t size);
extern void    *osRtxMemoryAlloc(void *mem, uint32_t size, uint32_t type);
extern uint32_t osRtxMemoryFree (void *mem, void *block);


#endif  // RTX_LIB_H_
