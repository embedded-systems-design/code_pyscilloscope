/***************************************************************************//**
* \file myUSB_cdc.c
* \version 3.20
*
* \brief
*  This file contains the USB CDC class request handler.
*
* Related Document:
*  Universal Serial Bus Class Definitions for Communication Devices Version 1.1
*
********************************************************************************
* \copyright
* Copyright 2012-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "myUSB_cdc.h"
#include "myUSB_pvt.h"
#include "cyapicallbacks.h"

#if defined(myUSB_ENABLE_CDC_CLASS)

/*******************************************************************************
*    CDC Variables
*******************************************************************************/

/*PUBLIC*/
/** Contains the current line coding structure. The host sets it using a
 * SET_LINE_CODING request and returns it to the user code using the
 * myUSB_GetDTERate(), myUSB_GetCharFormat(),
 * myUSB_GetParityType(), and myUSB_GetDataBits() APIs.
 * It is an array of 2 elements for COM port 1 and COM port 2 for MultiCOM port
 * support. In case of 1 COM port, data is in 0 element.*/
volatile uint8 myUSB_linesCoding[myUSB_MAX_MULTI_COM_NUM][myUSB_LINE_CODING_SIZE] =
{
    /*COM Port 1*/
    {
    0x00u, 0xC2u, 0x01u, 0x00u,     /* Data terminal rate 115200 */
    0x00u,                          /* 1 Stop bit */
    0x00u,                          /* None parity */
    0x08u                           /* 8 data bits */
    },
    /*COM Port 2*/
    {
    0x00u, 0xC2u, 0x01u, 0x00u,     /* Data terminal rate 115200 */
    0x00u,                          /* 1 Stop bit */
    0x00u,                          /* None parity */
    0x08u                           /* 8 data bits */
    }
};

/**Used as a flag for the myUSB_IsLineChanged() API, to inform it that the
 * host has been sent a request to change line coding or control bitmap. It is 
 * an array of 2 elements for COM port 1 and COM port 2 for MultiCOM port 
 * support. In case of 1 COM port, data is in 0 element.*/
volatile uint8  myUSB_linesChanged[myUSB_MAX_MULTI_COM_NUM];
/** Contains the current control-signal bitmap. The host sets it using a
 * SET_CONTROL_LINE request and returns it to the user code using the 
 * myUSB_GetLineControl() API. It is an array of 2 elements for COM 
 * port 1 and COM port 2 for MultiCOM port support. In case of 1 COM port, data 
 * is in 0 element.*/
volatile uint16 myUSB_linesControlBitmap[myUSB_MAX_MULTI_COM_NUM];
/** Contains the 16-bit serial state value that was sent using the 
 * \ref myUSB_SendSerialState() API. . It is an array of 2 elements 
 * for COM port 1 and COM port 2 for MultiCOM port support. In case of 1 COM 
 * port, data is in 0 element.*/
volatile uint16 myUSB_serialStateBitmap[myUSB_MAX_MULTI_COM_NUM];
/** Contains the data IN endpoint number. It is initialized after a 
 * SET_CONFIGURATION request based on a user descriptor. It is used in CDC APIs 
 * to send data to the PC. It is an array of 2 elements for COM port 1 and COM 
 * port 2 for MultiCOM port support. In case of 1 COM port, data is in 0 element.*/
volatile uint8  myUSB_cdcDataInEp[myUSB_MAX_MULTI_COM_NUM];
/** Contains the data OUT endpoint number. It is initialized after a 
 * SET_CONFIGURATION request based on user descriptor. It is used in CDC APIs to 
 * receive data from the PC. It is an array of 2 elements for COM port 1 and COM  
 * port 2 for MultiCOM port support. In case of 1 COM port, data is in 0 element.*/
volatile uint8  myUSB_cdcDataOutEp[myUSB_MAX_MULTI_COM_NUM];
/** Contains the data IN endpoint number for COMMUNICATION interface. It is 
 * initialized after a SET_CONFIGURATION request based on a user descriptor. It 
 * is used in CDC APIs to send data to the PC. It is an array of 2 elements for 
 * COM port 1 and COM port 2 for MultiCOM port support. In case of 1 COM port, 
 * data is in 0 element.*/
volatile uint8  myUSB_cdcCommInInterruptEp[myUSB_MAX_MULTI_COM_NUM];

/*PRIVATE*/

#define myUSB_CDC_IN_EP      (0u)
#define myUSB_CDC_OUT_EP     (1u)
#define myUSB_CDC_NOTE_EP    (2u)

#define myUSB_CDC_EP_MASK    (0x01u)

#define myUSB_GET_EP_COM_NUM(cdcComNums, epType) (((cdcComNums) >> (epType)) & myUSB_CDC_EP_MASK)


/***************************************
*     Static Function Prototypes
***************************************/
#if (myUSB_ENABLE_CDC_CLASS_API != 0u)
    static uint16 myUSB_StrLen(const char8 string[]) ;
    static t_myUSB_cdc_notification myUSB_serialStateNotification =
    {

        myUSB_SERIAL_STATE_REQUEST_TYPE, /* bRequestType    */
        myUSB_SERIAL_STATE,              /* bNotification   */
        0u,                                         /* wValue          */
        0u,                                         /* wValueMSB       */
        0u,                                         /* wIndex          */
        0u,                                         /* wIndexMSB       */
        myUSB_SERIAL_STATE_LENGTH,       /* wLength         */
        0u,                                         /* wLengthMSB      */
        0u,                                         /* wSerialState    */
        0u,                                         /* wSerialStateMSB */
    };
    static uint8 myUSB_activeCom = 0u;
#endif /* (myUSB_ENABLE_CDC_CLASS_API != 0u) */


/***************************************
* Custom Declarations
***************************************/

/* `#START CDC_CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */


/*******************************************************************************
* Function Name: myUSB_DispatchCDCClassRqst
****************************************************************************//**
*
*  This routine dispatches CDC class requests.
*
* \return
*  requestHandled
*
* \globalvars
*   myUSB_linesCoding: Contains the current line coding structure.
*     It is set by the Host using SET_LINE_CODING request and returned to the
*     user code by the USBFS_GetDTERate(), USBFS_GetCharFormat(),
*     USBFS_GetParityType(), USBFS_GetDataBits() APIs.
*   myUSB_linesControlBitmap: Contains the current control signal
*     bitmap. It is set by the Host using SET_CONTROL_LINE request and returned
*     to the user code by the USBFS_GetLineControl() API.
*   myUSB_linesChanged: This variable is used as a flag for the
*     USBFS_IsLineChanged() API, to be aware that Host has been sent request
*     for changing Line Coding or Control Bitmap.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_DispatchCDCClassRqst(void) 
{
    uint8 requestHandled = myUSB_FALSE;
    uint8 comPort;

    comPort = myUSB_GetInterfaceComPort((uint8)myUSB_wIndexLoReg);

    /* Check request direction: D2H or H2D. */
    if (0u != (myUSB_bmRequestTypeReg & myUSB_RQST_DIR_D2H))
    {
        /* Handle direction from device to host. */

        switch (myUSB_bRequestReg)
        {
            case myUSB_CDC_GET_LINE_CODING:
                myUSB_currentTD.count = myUSB_LINE_CODING_SIZE;
                myUSB_currentTD.pData = myUSB_linesCoding[comPort];
                requestHandled  = myUSB_InitControlRead();
                break;

            /* `#START CDC_READ_REQUESTS` Place other request handler here */

            /* `#END` */

            default:
            /* Do not handle this request unless callback is defined. */
            #ifdef myUSB_DISPATCH_CDC_CLASS_CDC_READ_REQUESTS_CALLBACK
                requestHandled = myUSB_DispatchCDCClass_CDC_READ_REQUESTS_Callback();
            #endif /* (myUSB_DISPATCH_CDC_CLASS_CDC_READ_REQUESTS_CALLBACK) */
                break;
        }
    }
    else
    {
        /* Handle direction from host to device. */

        switch (myUSB_bRequestReg)
        {
            case myUSB_CDC_SET_LINE_CODING:
                myUSB_currentTD.count  = myUSB_LINE_CODING_SIZE;
                myUSB_currentTD.pData  = myUSB_linesCoding[comPort];
                myUSB_linesChanged[comPort] |= myUSB_LINE_CODING_CHANGED;

                requestHandled = myUSB_InitControlWrite();
                break;

            case myUSB_CDC_SET_CONTROL_LINE_STATE:
                myUSB_linesControlBitmap[comPort] = (uint8) myUSB_wValueLoReg;
                myUSB_linesChanged[comPort]      |= myUSB_LINE_CONTROL_CHANGED;

                requestHandled = myUSB_InitNoDataControlTransfer();
                break;

            /* `#START CDC_WRITE_REQUESTS` Place other request handler here */

            /* `#END` */

            default:
                /* Do not handle this request unless callback is defined. */
            #ifdef myUSB_DISPATCH_CDC_CLASS_CDC_WRITE_REQUESTS_CALLBACK
                requestHandled = myUSB_DispatchCDCClass_CDC_WRITE_REQUESTS_Callback();
            #endif /* (myUSB_DISPATCH_CDC_CLASS_CDC_WRITE_REQUESTS_CALLBACK) */
                break;
        }
    }

    return(requestHandled);
}


/***************************************************************************
* Function Name: myUSB_GetInterfaceComPort
************************************************************************//**
*   \internal
*  Internal function which gets number of COM port by specified interface
*   number.
*
* \param uint8 interface
*  Interface number
*
* \return
*  COM port number (0 or 1) or error 0xFF
*
***************************************************************************/
uint8 myUSB_GetInterfaceComPort(uint8 interface) 
{
    uint8 comPort = 0u;
    uint8 res = 0xFFu;
    uint8 notEp;

    while (comPort < myUSB_MAX_MULTI_COM_NUM)
    {
        notEp = myUSB_cdcCommInInterruptEp[comPort];

        if (myUSB_EP[notEp].interface == interface)
        {
            res = comPort;
            comPort = myUSB_MAX_MULTI_COM_NUM;
        }

        comPort++;
    }
    return (res);
}


/***************************************
* Optional CDC APIs
***************************************/
#if (myUSB_ENABLE_CDC_CLASS_API != 0u)
/***************************************************************************
* Function Name: myUSB_CDC_Init
************************************************************************//**
*
*  This function initializes the CDC interface to be ready to receive data
*  from the PC. The API set active communication port to 0 in the case of 
*  multiple communication port support.This API should be called after the 
*  device has been started and configured using USBUART_Start() API to 
*  initialize and start the USBFS component operation. Then call the 
*  USBUART_GetConfiguration() API to wait until the host has enumerated and 
*  configured the device. For example:
*
*  \snippet /USBFS_sut_02.cydsn/main.c wait for enumeration
*
* \return
*  cystatus:
*   Return Value    Description
*   myUSB_SUCCESS   CDC interface was initialized correctly
*   myUSB_FAILURE   CDC interface was not initialized
*
* \globalvars
*   myUSB_linesChanged: Initialized to zero.
*   myUSB_cdcDataOutEp: Used as an OUT endpoint number.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint8 myUSB_CDC_Init(void) 
{
    uint8 comPort;
    uint8 outEp;
    uint8 ret = myUSB_SUCCESS;

    myUSB_activeCom = 0u;
    myUSB_linesChanged[myUSB_COM_PORT1] = 0u;
    myUSB_linesChanged[myUSB_COM_PORT2] = 0u;

    for(comPort = 0u; comPort<myUSB_MAX_MULTI_COM_NUM; comPort++)
    {
        outEp = myUSB_cdcDataOutEp[comPort];
        if((0u != outEp) && (myUSB_MAX_EP > outEp))
        {
            myUSB_EnableOutEP(outEp);
        }

    }

    /* COM Port 1 should be correct to proceed. */
    if ((0u == myUSB_cdcDataInEp[myUSB_COM_PORT1]) \
            || (0u == myUSB_cdcDataOutEp[myUSB_COM_PORT1]) \
            || (0u ==  myUSB_cdcCommInInterruptEp[myUSB_COM_PORT1])
            || (myUSB_cdcDataInEp[myUSB_COM_PORT1] >= myUSB_MAX_EP)
            || (myUSB_cdcDataOutEp[myUSB_COM_PORT1] >= myUSB_MAX_EP)
            || (myUSB_cdcCommInInterruptEp[myUSB_COM_PORT1] >= myUSB_MAX_EP))
    {
        ret = myUSB_FAILURE;
    }

    return (ret);
}


/*******************************************************************************
* Function Name: myUSB_PutData
****************************************************************************//**
*
*  This function sends a specified number of bytes from the location specified
*  by a pointer to the PC. The myUSB_CDCIsReady() function should be
*  called before sending new data, to be sure that the previous data has
*  finished sending.
*  If the last sent packet is less than maximum packet size the USB transfer
*  of this short packet will identify the end of the segment. If the last sent
*  packet is exactly maximum packet size, it shall be followed by a zero-length
*  packet (which is a short packet) to assure the end of segment is properly
*  identified. To send zero-length packet, use myUSB_PutData() API
*  with length parameter set to zero.
*
*  \param pData: pointer to the buffer containing data to be sent.
*  \param length: Specifies the number of bytes to send from the pData
*  buffer. Maximum length will be limited by the maximum packet
*  size for the endpoint. Data will be lost if length is greater than Max
*  Packet Size.
*
* \globalvars
*
*   myUSB_cdcDataInEp: CDC IN endpoint number used for sending
*     data.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_PutData(const uint8* pData, uint16 length) 
{
    uint8 epNumber = myUSB_cdcDataInEp[myUSB_activeCom];

    /* Limit length to maximum packet size for endpoint. */
    if (length > myUSB_EP[epNumber].bufferSize)
    {
        /* Caution: Data will be lost if length is greater than Max Packet Size. */
        length = myUSB_EP[epNumber].bufferSize;

        /* Halt CPU in debug mode */
        CYASSERT(0u != 0u);
    }

    myUSB_LoadInEP(epNumber, pData, length);
}


/*******************************************************************************
* Function Name: myUSB_StrLen
****************************************************************************//**
*
*  Calculates length of a null terminated string.
*
*  \param string: pointer to the string.
*
* \return
*  Length of the string
*
*******************************************************************************/
static uint16 myUSB_StrLen(const char8 string[]) 
{
    uint16 len = 0u;

    while (string[len] != (char8)0)
    {
        len++;
    }

    return ((uint16) len);
}


/***************************************************************************
* Function Name: myUSB_PutString
************************************************************************//**
*
*  This function sends a null terminated string to the PC. This function will
*  block if there is not enough memory to place the whole string. It will block
*  until the entire string has been written to the transmit buffer.
*  The myUSB_CDCIsReady() function should be called before
*  sending data with a new call to myUSB_PutString(), to be sure
*  that the previous data has finished sending. This function sends
*  zero-length packet automatically, if the length of the last packet, sent
*  by this API, is equal to Max Packet Size
*
*  \param string: pointer to the string to be sent to the PC.
*
* \globalvars
*
*   myUSB_cdcDataInEp: CDC IN endpoint number used for sending
*     data.
*
* \reentrant
*  No.
*
***************************************************************************/
void myUSB_PutString(const char8 string[]) 
{
    uint16 strLength;
    uint16 sendLength;
    uint16 bufIndex = 0u;

    uint8  epNumber = myUSB_cdcDataInEp[myUSB_activeCom];

    /* Get length string length (it is terminated with zero). */
    strLength = myUSB_StrLen(string);

    do
    {
        /* Limit length to maximum packet size of endpoint. */
        sendLength = (strLength > myUSB_EP[epNumber].bufferSize) ?
                                  myUSB_EP[epNumber].bufferSize : strLength;

        /* Load IN endpoint and expose it to host. */
        myUSB_LoadInEP(epNumber, (const uint8 *)&string[bufIndex], sendLength);
        strLength -= sendLength;

        /* If more data are present to send or full packet was sent */
        if ((strLength > 0u) || (sendLength == myUSB_EP[epNumber].bufferSize))
        {
            bufIndex += sendLength;

            /* Wait until host read data from IN endpoint buffer. */
            while (myUSB_IN_BUFFER_FULL == myUSB_EP[epNumber].apiEpState)
            {
            }

            /* If last packet is exactly maximum packet size, it shall be followed
            * by a zero-length packet to assure the end of segment is properly
            * identified by the terminal.
            */
            if (0u == strLength)
            {
                myUSB_LoadInEP(epNumber, NULL, 0u);
            }
        }
    }
    while (strLength > 0u);
}


/***************************************************************************
* Function Name: myUSB_PutChar
************************************************************************//**
*
*  This function writes a single character to the PC at a time. This is an
*  inefficient way to send large amounts of data.
*
*  \param txDataByte: Character to be sent to the PC.
*
*  \globalvars
*
*   myUSB_cdcDataInEp: CDC IN endpoint number used for sending
*     data.
*
*  \reentrant
*  No.
*
***************************************************************************/
void myUSB_PutChar(char8 txDataByte) 
{
    uint8 dataByte;
    dataByte = (uint8) txDataByte;

    myUSB_LoadInEP(myUSB_cdcDataInEp[myUSB_activeCom], &dataByte, 1u);
}


/*******************************************************************************
* Function Name: myUSB_PutCRLF
****************************************************************************//**
*
*  This function sends a carriage return (0x0D) and line feed (0x0A) to the
*  PC. This APIis provided to mimic API provided by our other UART components
*
* \globalvars
*
*   myUSB_cdcDataInEp: CDC IN endpoint number used for sending
*     data.
*
* \reentrant
*  No.
*
*******************************************************************************/
void myUSB_PutCRLF(void) 
{
    const uint8 CYCODE txData[] = {0x0Du, 0x0Au};

    myUSB_LoadInEP(myUSB_cdcDataInEp[myUSB_activeCom], (const uint8 *)txData, 2u);
}


/*******************************************************************************
* Function Name: myUSB_GetCount
****************************************************************************//**
*
*  This function returns the number of bytes that were received from the PC.
*  The returned length value should be passed to myUSB_GetData() as
*  a parameter to read all received data. If all of the received data is not
*  read at one time by the myUSB_GetData() API, the unread data will
*  be lost.
*
* \return
*  Returns the number of received bytes. The maximum amount of received data at
*  a time is limited by the maximum packet size for the endpoint.
*
* \globalvars
*   myUSB_cdcDataOutEp: CDC OUT endpoint number used.
*
*******************************************************************************/
uint16 myUSB_GetCount(void) 
{
    uint16 bytesCount;

    uint8  epNumber = myUSB_cdcDataOutEp[myUSB_activeCom];

    if (myUSB_OUT_BUFFER_FULL == myUSB_EP[epNumber].apiEpState)
    {
        bytesCount = myUSB_GetEPCount(epNumber);
    }
    else
    {
        bytesCount = 0u;
    }

    return (bytesCount);
}


/*******************************************************************************
* Function Name: myUSB_DataIsReady
****************************************************************************//**
*
*  This function returns a non-zero value if the component received data or
*  received zero-length packet. The myUSB_GetAll() or
*  myUSB_GetData() API should be called to read data from the buffer
*  and reinitialize the OUT endpoint even when a zero-length packet is
*  received. These APIs will return zero value when zero-length packet is
*  received.
*
* \return
*  If the OUT packet is received, this function returns a non-zero value.
*  Otherwise, it returns zero.
*
* \globalvars
*   myUSB_cdcDataOutEp: CDC OUT endpoint number used.
*
*******************************************************************************/
uint8 myUSB_DataIsReady(void) 
{
    return (myUSB_GetEPState(myUSB_cdcDataOutEp[myUSB_activeCom]));
}


/*******************************************************************************
* Function Name: myUSB_CDCIsReady
****************************************************************************//**
*
*  This function returns a non-zero value if the component is ready to send more
*  data to the PC; otherwise, it returns zero. The function should be called
*  before sending new data when using any of the following APIs:
*  myUSB_PutData(),myUSB_PutString(),
*  myUSB_PutChar or myUSB_PutCRLF(),
*  to be sure that the previous data has finished sending.
*
* \return
*  If the buffer can accept new data, this function returns a non-zero value.
*  Otherwise, it returns zero.
*
* \globalvars
*   myUSB_cdcDataInEp: CDC IN endpoint number used.
*
*******************************************************************************/
uint8 myUSB_CDCIsReady(void) 
{
    return (myUSB_GetEPState(myUSB_cdcDataInEp[myUSB_activeCom]));
}


/***************************************************************************
* Function Name: myUSB_GetData
************************************************************************//**
*
*  This function gets a specified number of bytes from the input buffer and
*  places them in a data array specified by the passed pointer.
*  The myUSB_DataIsReady() API should be called first, to be sure
*  that data is received from the host. If all received data will not be read at
*  once, the unread data will be lost. The myUSB_GetData() API should
*  be called to get the number of bytes that were received.
*
*  \param pData: Pointer to the data array where data will be placed.
*  \param Length: Number of bytes to read into the data array from the RX buffer.
*          Maximum length is limited by the the number of received bytes
*          or 64 bytes.
*
* \return
*         Number of bytes which function moves from endpoint RAM into the
*         data array. The function moves fewer than the requested number
*         of bytes if the host sends fewer bytes than requested or sends
*         zero-length packet.
*
* \globalvars
*   myUSB_cdcDataOutEp: CDC OUT endpoint number used.
*
* \reentrant
*  No.
*
***************************************************************************/
uint16 myUSB_GetData(uint8* pData, uint16 length) 
{
    uint8 epNumber = myUSB_cdcDataOutEp[myUSB_activeCom];

    /* Read data from OUT endpoint buffer. */
    length = myUSB_ReadOutEP(epNumber, pData, length);

#if (myUSB_EP_MANAGEMENT_DMA_MANUAL)
    /* Wait until DMA complete transferring data from OUT endpoint buffer. */
    while (myUSB_OUT_BUFFER_FULL == myUSB_GetEPState(epNumber))
    {
    }

    /* Enable OUT endpoint to communicate with host. */
    myUSB_EnableOutEP(epNumber);
#endif /* (myUSB_EP_MANAGEMENT_DMA_MANUAL) */

    return (length);
}


/*******************************************************************************
* Function Name: myUSB_GetAll
****************************************************************************//**
*
*  This function gets all bytes of received data from the input buffer and
*  places them into a specified data array. The
*  myUSB_DataIsReady() API should be called first, to be sure
*  that data is received from the host.
*
*  \param pData: Pointer to the data array where data will be placed.
*
* \return
*  Number of bytes received. The maximum amount of the received at a time
*  data is 64 bytes.
*
* \globalvars
*   - \ref myUSB_cdcDataOutEp: CDC OUT endpoint number used.
*   - \ref myUSB_EP[].bufferSize: EP max packet size is used as a
*     length to read all data from the EP buffer.
*
* \reentrant
*  No.
*
*******************************************************************************/
uint16 myUSB_GetAll(uint8* pData) 
{
    uint8 epNumber = myUSB_cdcDataOutEp[myUSB_activeCom];
    uint16 dataLength;

    /* Read data from OUT endpoint buffer. */
    dataLength = myUSB_ReadOutEP(epNumber, pData, myUSB_EP[epNumber].bufferSize);

#if (myUSB_EP_MANAGEMENT_DMA_MANUAL)
    /* Wait until DMA complete transferring data from OUT endpoint buffer. */
    while (myUSB_OUT_BUFFER_FULL == myUSB_GetEPState(epNumber))
    {
    }

    /* Enable OUT endpoint to communicate with host. */
    myUSB_EnableOutEP(epNumber);
#endif /* (myUSB_EP_MANAGEMENT_DMA_MANUAL) */

    return (dataLength);
}


/***************************************************************************
* Function Name: myUSB_GetChar
************************************************************************//**
*
*  This function reads one byte of received data from the buffer. If more than
*  one byte has been received from the host, the rest of the data will be lost.
*
* \return
*  Received one character.
*
* \globalvars
*   myUSB_cdcDataOutEp: CDC OUT endpoint number used.
*
* \reentrant
*  No.
*
***************************************************************************/
uint8 myUSB_GetChar(void) 
{
     uint8 rxData;
     uint8 epNumber = myUSB_cdcDataOutEp[myUSB_activeCom];

    (void) myUSB_ReadOutEP(epNumber, &rxData, 1u);

#if (myUSB_EP_MANAGEMENT_DMA_MANUAL)
    /* Wait until DMA complete transferring data from OUT endpoint buffer. */
    while (myUSB_OUT_BUFFER_FULL == myUSB_GetEPState(epNumber))
    {
    }

    /* Enable OUT endpoint to communicate with host. */
    myUSB_EnableOutEP(epNumber);
#endif /* (myUSB_EP_MANAGEMENT_DMA_MANUAL) */

    return (rxData);
}


/*******************************************************************************
* Function Name: myUSB_IsLineChanged
****************************************************************************//**
*
*  This function returns clear on read status of the line. It returns not zero
*  value when the host sends updated coding or control information to the
*  device. The myUSB_GetDTERate(), myUSB_GetCharFormat()
*  or myUSB_GetParityType() or myUSB_GetDataBits() API
*  should be called to read data coding information.
*  The myUSB_GetLineControl() API should be called to read line
*  control information.
*
* \return
*  If SET_LINE_CODING or CDC_SET_CONTROL_LINE_STATE requests are received, it
*  returns a non-zero value. Otherwise, it returns zero.
*  Return Value                 | Description
*  -----------------------------|--------------------------
*  USBUART_LINE_CODING_CHANGED  | Line coding changed
*  USBUART_LINE_CONTROL_CHANGED |   Line control changed
*
* \globalvars
*  - \ref myUSB_transferState: it is checked to be sure then OUT
*    data phase has been complete, and data written to the lineCoding or
*    Control Bitmap buffer.
*  - \ref myUSB_linesChanged: used as a flag to be aware that
*    Host has been sent request for changing Line Coding or Control Bitmap.
*
*******************************************************************************/
uint8 myUSB_IsLineChanged(void) 
{
    uint8 state = 0u;

    /* transferState is checked to be sure then OUT data phase has been complete */
    if (myUSB_transferState == myUSB_TRANS_STATE_IDLE)
    {
        if (myUSB_linesChanged[myUSB_activeCom] != 0u)
        {
            state = myUSB_linesChanged[myUSB_activeCom];
            myUSB_linesChanged[myUSB_activeCom] = 0u;
        }
    }

    return (state);
}


/***************************************************************************
* Function Name: myUSB_GetDTERate
************************************************************************//**
*
*  This function returns the data terminal rate set for this port in bits
*  per second.
*
* \return
*  Returns a uint32 value of the data rate in bits per second.
*
* \globalvars
*  myUSB_linesCoding: First four bytes converted to uint32
*    depend on compiler, and returned as a data rate.
*
*******************************************************************************/
uint32 myUSB_GetDTERate(void) 
{
    uint32 rate;

    rate = myUSB_linesCoding[myUSB_activeCom][myUSB_LINE_CODING_RATE + 3u];
    rate = (rate << 8u) | myUSB_linesCoding[myUSB_activeCom][myUSB_LINE_CODING_RATE + 2u];
    rate = (rate << 8u) | myUSB_linesCoding[myUSB_activeCom][myUSB_LINE_CODING_RATE + 1u];
    rate = (rate << 8u) | myUSB_linesCoding[myUSB_activeCom][myUSB_LINE_CODING_RATE];

    return (rate);
}


/*******************************************************************************
* Function Name: myUSB_GetCharFormat
****************************************************************************//**
*
*  Returns the number of stop bits.
*
* \return
*  Returns the number of stop bits.
*  Return               |Value Description
*  ---------------------|-------------------
*  USBUART_1_STOPBIT    | 1 stop bit
*  USBUART_1_5_STOPBITS | 1,5 stop bits
*  USBUART_2_STOPBITS   | 2 stop bits
*
*
* \globalvars
*  myUSB_linesCoding: used to get a parameter.
*
*******************************************************************************/
uint8 myUSB_GetCharFormat(void) 
{
    return (myUSB_linesCoding[myUSB_activeCom][myUSB_LINE_CODING_STOP_BITS]);
}


/*******************************************************************************
* Function Name: myUSB_GetParityType
****************************************************************************//**
*
*  This function returns the parity type for the CDC port.
*
* \return
*  Returns the parity type.
*   Return               | Value Description
*  ----------------------|-------------------
*  USBUART_PARITY_NONE   | 1 stop bit
*  USBUART_PARITY_ODD    | 1,5 stop bits
*  USBUART_PARITY_EVEN   | 2 stop bits
*  USBUART_PARITY_MARK   | Mark
*  USBUART_PARITY_SPACE  | Space
*
* \globalvars
*  myUSB_linesCoding: used to get a parameter.
*
*******************************************************************************/
uint8 myUSB_GetParityType(void) 
{
    return (myUSB_linesCoding[myUSB_activeCom][myUSB_LINE_CODING_PARITY]);
}


/***************************************************************************
* Function Name: myUSB_GetDataBits
************************************************************************//**
*
*  This function returns the number of data bits for the CDC port.
*
* \return
*  Returns the number of data bits.
*  The number of data bits can be 5, 6, 7, 8 or 16.
*
* \globalvars
*  myUSB_linesCoding: used to get a parameter.
*
*******************************************************************************/
uint8 myUSB_GetDataBits(void) 
{
    return (myUSB_linesCoding[myUSB_activeCom][myUSB_LINE_CODING_DATA_BITS]);
}


/***************************************************************************
* Function Name: myUSB_GetLineControl
************************************************************************//**
*
*  This function returns Line control bitmap that the host sends to the
*  device.
*
* \return
*  Returns Line control bitmap.
*  Return                   |Value Notes
*  -------------------------|-----------------------------------------------
*  USBUART_LINE_CONTROL_DTR |Indicates that a DTR signal is present. This signal corresponds to V.24 signal 108/2 and RS232 signal DTR.
*  USBUART_LINE_CONTROL_RTS |Carrier control for half-duplex modems. This signal corresponds to V.24 signal 105 and RS232 signal RTS.
*  RESERVED                 |The rest of the bits are reserved.
*
*  *Note* Some terminal emulation programs do not properly handle these
*  control signals. They update information about DTR and RTS state only
*  when the RTS signal changes the state.
*
* \globalvars
*  myUSB_linesControlBitmap: used to get a parameter.
*
*******************************************************************************/
uint16 myUSB_GetLineControl(void) 
{
    return (myUSB_linesControlBitmap[myUSB_activeCom]);
}


/*******************************************************************************
* Function Name: myUSB_SendSerialState
****************************************************************************//**
*
*  Sends the serial state notification to the host using the interrupt
*  endpoint for the COM port selected using the API SetComPort().The
*  myUSB_NotificationIsReady() API must be called to check if the
*  Component is ready to send more serial state to the host. The API will
*  not send the notification data if the interrupt endpoint Max Packet Size
*  is less than the required 10 bytes.
*
* \param uint16 serialState
*  16-bit value that will be sent from the device to the
*  host as SERIAL_STATE notification using the IN interrupt endpoint. Refer
*  to revision 1.2 of the CDC PSTN Subclass specification for bit field
*  definitions of the 16-bit serial state value.
*
*******************************************************************************/
void myUSB_SendSerialState (uint16 serialState) 
{
    uint8 epNumber = myUSB_cdcCommInInterruptEp[myUSB_activeCom];

    if(myUSB_SERIAL_STATE_SIZE <= myUSB_EP[epNumber].bufferSize)
    {
        /* Save current SERIAL_STATE bitmap. */
        myUSB_serialStateBitmap[myUSB_activeCom] = serialState;

        /* Add interface number */
        myUSB_serialStateNotification.wIndex = myUSB_EP[epNumber].interface;

        /*Form SERIAL_STATE data*/
        myUSB_serialStateNotification.wSerialState =    LO8(myUSB_serialStateBitmap[myUSB_activeCom]);
        myUSB_serialStateNotification.wSerialStateMSB = HI8(myUSB_serialStateBitmap[myUSB_activeCom]);

        myUSB_LoadInEP(epNumber, (uint8 *) &myUSB_serialStateNotification, sizeof(myUSB_serialStateNotification));
    }
}


/*******************************************************************************
* Function Name: myUSB_GetSerialState
****************************************************************************//**
*
*  This function returns the current serial state value for the COM port
*  selected using the API SetComPort().
*
* \return
*  16-bit serial state value. Refer to revision 1.2 of the CDC PSTN Subclass
*  specification for bit field definitions of the 16-bit serial state value.
*
*******************************************************************************/
uint16 myUSB_GetSerialState(void) 
{
    return myUSB_serialStateBitmap[myUSB_activeCom];
}


/*******************************************************************************
* Function Name: myUSB_NotificationIsReady
****************************************************************************//**
*
*  This function returns a non-zero value if the Component is ready to send
*  more notifications to the host; otherwise, it returns zero. The function
*  should be called before sending new notifications when using
*  myUSB_SendSerialState() to ensure that any previous
*  notification data has been already sent to the host.
*
* \return
*  If the buffer can accept new data(endpoint buffer not full), this
*  function returns a non-zero value. Otherwise, it returns zero.
*
* \globalvars
*   myUSB_cdcDataInEp: CDC IN endpoint number used.
*
*******************************************************************************/
uint8 myUSB_NotificationIsReady(void) 
{
    return (myUSB_EP[myUSB_cdcCommInInterruptEp[myUSB_activeCom]].apiEpState);
}


/*******************************************************************************
* Function Name: myUSB_SetComPort
****************************************************************************//**
*
*  This function allows the user to select from one of the two COM ports
*  they wish to address in the instance of having multiple COM ports
*  instantiated though the use of a composite device. Once set, all future
*  function calls related to the USBUART will be affected. This addressed
*  COM port can be changed during run time.
*
* \param comNumber
*  Contains the COM interface the user wishes to address. Value can either
*  be 0 or 1 since a maximum of only 2 COM ports can be supported. Note that
*  this COM port number is not the COM port number assigned on the PC side
*  for the UART communication. If a value greater than 1 is passed, the
*  function returns without performing any action.
*
*******************************************************************************/
void myUSB_SetComPort(uint8 comNumber) 
{
    if ((myUSB_activeCom != comNumber) && \
            (comNumber < myUSB_MAX_MULTI_COM_NUM ))
    {
        myUSB_activeCom = comNumber;
    }
}


/*******************************************************************************
* Function Name: myUSB_GetComPort
****************************************************************************//**
*
*  This function returns the current selected COM port that the user is
*  currently addressing in the instance of having multiple COM ports
*  instantiated though the use of a composite device.
*
* \return
*  Returns the currently selected COM port. Value can either be 0 or 1 since
*  a maximum of only 2 COM ports can be supported. . Note that this COM port
*  number is not the COM port number assigned on the PC side for the UART
*  communication.
*
*******************************************************************************/
uint8 myUSB_GetComPort(void) 
{
    return (myUSB_activeCom);
}


#endif  /* (myUSB_ENABLE_CDC_CLASS_API) */


/***************************************************************************
* Function Name: myUSB_Cdc_EpInit
************************************************************************//**
*
*  \internal
*  This routine decide type of endpoint (IN, OUT, Notification) and same to
*   appropriate global variables  according to COM port number.
*   myUSB_cdcDataInEp[], myUSB_cdcCommInInterruptEp[],
*   myUSB_cdcDataOutEp[]
*
* \param pEP: Pointer to structure with current EP description.
* \param epNum: EP number
* \param cdcComNums: Bit array of current COM ports for CDC IN, OUT,
*        and notification EPs(0 - COM port 1, 1- COM port 2)
*
* \return
*  Updated cdcComNums
*
* \reentrant
*  No.
*
***************************************************************************/
uint8 myUSB_Cdc_EpInit(const T_myUSB_EP_SETTINGS_BLOCK CYCODE *pEP, uint8 epNum, uint8 cdcComNums) 
{
    uint8 epType;

    epType = pEP->attributes & myUSB_EP_TYPE_MASK;

    if (0u != (pEP->addr & myUSB_DIR_IN))
    {
        if (epType != myUSB_EP_TYPE_INT)
        {
            myUSB_cdcDataInEp[myUSB_GET_EP_COM_NUM(cdcComNums, myUSB_CDC_IN_EP)] = epNum;
            cdcComNums |= (uint8)(myUSB_COM_PORT2 << myUSB_CDC_IN_EP);
        }
        else
        {

            myUSB_cdcCommInInterruptEp[myUSB_GET_EP_COM_NUM(cdcComNums, myUSB_CDC_NOTE_EP)] = epNum;
            cdcComNums |= (uint8)(myUSB_COM_PORT2 << myUSB_CDC_NOTE_EP);
        }
    }
    else
    {
        if (epType != myUSB_EP_TYPE_INT)
        {
            myUSB_cdcDataOutEp[myUSB_GET_EP_COM_NUM(cdcComNums, myUSB_CDC_OUT_EP)] = epNum;
            cdcComNums |= (uint8)(myUSB_COM_PORT2 << myUSB_CDC_OUT_EP);
        }
    }
    return (cdcComNums);
}


/*******************************************************************************
* Additional user functions supporting CDC Requests
********************************************************************************/

/* `#START CDC_FUNCTIONS` Place any additional functions here */

/* `#END` */

#endif  /* (myUSB_ENABLE_CDC_CLASS) */


/* [] END OF FILE */
