/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.32 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information

Licensor:                 SEGGER Software GmbH
Licensed to:              ARM Ltd, 110 Fulbourn Road, CB1 9NJ Cambridge, UK
Licensed SEGGER software: emWin
License number:           GUI-00181
License model:            LES-SLA-20007, Agreement, effective since October 1st 2011 
Licensed product:         MDK-ARM Professional
Licensed platform:        ARM7/9, Cortex-M/R4
Licensed number of seats: -
----------------------------------------------------------------------
File        : GUI_SPY_X_StartServer.c
Purpose     : Start of emWinSPY server via RTT and/or TCP/IP.
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "OS.h"
#include "GUI.h"
//#include "Taskprio.h"

#define USE_RTT   1
#define USE_TCP   0

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// Select interface to be used
//
#ifndef   USE_RTT
  #define USE_RTT (1)
#endif
#ifndef   USE_TCP
  #define USE_TCP (1)
#endif

#if (USE_TCP == 1)
  #include "IP.h"         // BSD socket interface
#endif

#if (USE_RTT == 1)
  #include "SEGGER_RTT.h" // RTT interface
#endif

//
// Check selection
//
#if (USE_TCP == 0) && (USE_RTT == 0)
  #error Please select TCP or RTT
#endif

//
// Port definition for TCP connection
//
#if (USE_TCP == 1)
  #define PORT 2468 // emWinSPY-Port
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#if (USE_RTT == 1)

#define  APP_CFG_TASK_RTT_PRIO        (OS_CFG_PRIO_MAX - 5)
#define  APP_CFG_TASK_RTT_STK_SIZE    512u

//
// embOS Stack area of the server
//
//static OS_STACKPTR int _StackSPYServer_RTT[0x200];
static  CPU_STK  AppTaskRTTStk[APP_CFG_TASK_RTT_STK_SIZE];

//
// embOS Task-control-block of the server
//
//static OS_TASK _SPYServer_TCB_RTT;
static  OS_TCB   AppTaskRTTTCB;

//
// Up- and down buffer for RTT
//

static char _acBufferUp  [0x200];
static char _acBufferDown[0x20];

static int _IndexUp;
static int _IndexDown;

#endif

#if (USE_TCP == 1)

//
// embOS Stack area of the server
//
static OS_STACKPTR int _StackSPYServer_TCP[0x200];

//
// embOS Task-control-block of the server
//
static OS_TASK _SPYServer_TCB_TCP;

#endif

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _Send_RTT
*
*  Function description:
*    Sending data via RTT
*
*  Return value:
*    On success number of successfully transmitted bytes, otherwise -1.
*/
#if (USE_RTT == 1)
static int _Send_RTT(const U8 * buf, int len, void * pConnectInfo) {
  int r;

  GUI_USE_PARA(pConnectInfo);
  do {
    r = SEGGER_RTT_Write(_IndexUp, buf, len);
  } while (r != len);
  return r;
}

/*********************************************************************
*
*       _Recv_RTT
*
*  Function description:
*    Receiving data via RTT
*
*  Return value:
*    On success number of successfully received bytes, otherwise -1.
*/
static int _Recv_RTT(U8 * buf, int len, void * pConnectInfo) {
  int r;

  GUI_USE_PARA(pConnectInfo);
  while (SEGGER_RTT_HasData(_IndexDown) == 0) {
    GUI_X_Delay(10);
  };
  r = SEGGER_RTT_Read(_IndexDown, buf, len);
  return r;
}

/*********************************************************************
*
*       _ServerTask_RTT
*
*  Function description:
*    This task  waits for an incoming connection. If a connection
*    has been established it calls GUI_SPY_Process() which should
*    return if the connection is closed.
*/
static void _ServerTask_RTT(void) {
  while (1) {
    //
    // Execute emWinSPY, returns after connection is closed
    //
    GUI_SPY_Process(_Send_RTT, _Recv_RTT, NULL);
    //
    // Wait a while...
    //
    GUI_X_Delay(10);
  }
}
#endif

/*********************************************************************
*
*       _ListenAtTcpAddr
*
* Starts listening at the given TCP port.
*/
#if (USE_TCP == 1)
static int _ListenAtTcpAddr(unsigned short Port) {
  int sock;
  struct sockaddr_in addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(Port);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  listen(sock, 1);
  return sock;
}

/*********************************************************************
*
*       _Send_TCP
*
*  Function description:
*    Sending data via TCP
*
*  Return value:
*    On success number of successfully transmitted bytes, otherwise -1.
*/
static int _Send_TCP(const U8 * buf, int len, void * pConnectInfo) {
  int r;

  r = send((long)pConnectInfo, (const char *)buf, len, 0);
  return r;
}

/*********************************************************************
*
*       _Recv_TCP
*
*  Function description:
*    Receiving data via TCP
*
*  Return value:
*    On success number of successfully received bytes, otherwise -1.
*/
static int _Recv_TCP(U8 * buf, int len, void * pConnectInfo) {
    int r;

  r = recv((long)pConnectInfo, (char *)buf, len, 0);
  return r;
}

/*********************************************************************
*
*       _ServerTask_TCP
*
*  Function description:
*    This task  waits for an incoming connection. If a connection
*    has been established it calls GUI_SPY_Process() which should
*    return if the connection is closed.
*/
static void _ServerTask_TCP(void) {
  static struct sockaddr_in Addr;
  int s, Sock, AddrLen;

  //
  // Loop until we get a socket into listening state
  //
  do {
    s = _ListenAtTcpAddr(PORT);
    if (s != -1) {
      break;
    }
    OS_Delay(100); // Try again
  } while (1);
  //
  // Loop once per client and create a thread for the actual server
  //
  while (1) {
    //
    // Wait for an incoming connection
    //
    AddrLen = sizeof(Addr);
    if ((Sock = accept(s, (struct sockaddr*)&Addr, &AddrLen)) == SOCKET_ERROR) {
      continue; // Error
    }
    //
    // Execute emWinSPY
    //
    GUI_SPY_Process(_Send_TCP, _Recv_TCP, (void *)Sock);
    //
    // Close the connection
    //
    closesocket(Sock);
    memset(&Addr, 0, sizeof(struct sockaddr_in));
  }
}
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SPY_X_StartServer
*/
int GUI_SPY_X_StartServer(void) {
#if (USE_RTT == 1)
   OS_ERR err;
  //
  // Allocate buffers for RTT
  //
  _IndexUp   = SEGGER_RTT_AllocUpBuffer  ("emWinSPY", _acBufferUp,   sizeof(_acBufferUp),   SEGGER_RTT_MODE_NO_BLOCK_SKIP);
  if (_IndexUp < 0) {
    return 1;
  }
  _IndexDown = SEGGER_RTT_AllocDownBuffer("emWinSPY", _acBufferDown, sizeof(_acBufferDown), SEGGER_RTT_MODE_NO_BLOCK_SKIP);
  if (_IndexDown < 0) {
    return 1;
  }
  //
  // Create task for RTT Server
  //
//  OS_CREATETASK(&_SPYServer_TCB_RTT, "SPY-Server(RTT)", _ServerTask_RTT, 100, _StackSPYServer_RTT);
  
	OSTaskCreate((OS_TCB       *)&AppTaskRTTTCB,              
				 (CPU_CHAR     *)"App Server RTT",
				 (OS_TASK_PTR   )_ServerTask_RTT, 
				 (void         *)0,
				 (OS_PRIO       )APP_CFG_TASK_RTT_PRIO,
				 (CPU_STK      *)&AppTaskRTTStk[0],
				 (CPU_STK_SIZE  )APP_CFG_TASK_RTT_STK_SIZE / 10,
				 (CPU_STK_SIZE  )APP_CFG_TASK_RTT_STK_SIZE,
				 (OS_MSG_QTY    )0,
				 (OS_TICK       )0,
				 (void         *)0,
				 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
				 (OS_ERR       *)&err);		
#endif
#if (USE_TCP == 1)
  //
  // Create task for TCP/IP server
  //
  OS_CREATETASK(&_SPYServer_TCB_TCP, "SPY-Server(TCP)", _ServerTask_TCP, 230, _StackSPYServer_TCP);
#endif
  //
  // O.k., server(s) started
  //
  return 0;
}

/*************************** End of file ****************************/
