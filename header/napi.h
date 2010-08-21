#ifndef _NAPI_H_
#define _NAPI_H_

#include <netdb.h>
#include "EZX_TypeDefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NAPI_MAX_DEFAULT_PROFILE_NUMBER    15
#define NAPI_MAX_PROFILE_NUMBER            NAPI_GetMaxProfiles()

#define NAPI_MAX_PHONE_NUMBER_LENGTH        44
#define NAPI_PPP_USER_NAME_LENGTH           32
#define NAPI_PPP_PASSWORD_LENGTH            32

#define NAPI_GPRS_APN_LENGTH                100
#define NAPI_GPRS_USER_NAME_LENGTH          100
#define NAPI_GPRS_PASSWORD_LENGTH           100

#define NAPI_PROFILE_OPERATOR_LOCKED        0x0001
#define NAPI_PROFILE_WAP_SERVICE            0x0002
#define NAPI_PROFILE_IPSEC_SETTING          0x0004
#define NAPI_PROFILE_DEFAULT_HOLD           0x0008
#define NAPI_PROFILE_ENABLE_SWITCH          0x0010
#define NAPI_PROFILE_PREFER_WIFI            0x0020
#define NAPI_PROFILE_WIFI_BGSCAN            0x0040

#define NAPI_MAX_LINK_NAME_LENGTH           NAPI_GPRS_APN_LENGTH
#define NAPI_MAX_PROFILE_NAME_LENGTH        64

#define NAPI_MAX_DOMAIN_LENGTH              100
#define NAPI_MAX_IFNAME_LENGTH              16

#define NAPI_LINK_GPRS           1
#define NAPI_LINK_CSD            2
#define NAPI_LINK_WIFI           3
#define NAPI_LINK_IPSEC          4
#define  MAX_PROFILE_NUMBER   NAPI_MAX_PROFILE_NUMBER

typedef struct  _NAPI_PROFILE_S
{
    INT8    version[4];
    UINT32  attrib;

    INT8    gprsApn[NAPI_GPRS_APN_LENGTH];
    INT8    gprsUser[NAPI_GPRS_USER_NAME_LENGTH];
    INT8    gprsPasswd[NAPI_GPRS_PASSWORD_LENGTH];
    UINT32  gprsLocalIP;       
    UINT32  gprsLocalDNS1;
    UINT32  gprsLocalDNS2;
    UINT16  gprsLingerTimer;   
   
    UINT8   gprsMiniQoS[5];
    UINT8   gprsReqQoS[5];
    
    UINT16  gprsMiniDataRate[4];
    UINT16  gprsMiniExtraQos[8];

    UINT16  gprsReqDataRate[4];
    UINT16  gprsReqExtraQos[8];

    UINT8   gprsQosIndex;
    
    INT8    DialNumber[NAPI_MAX_PHONE_NUMBER_LENGTH];
    INT8    csdUser[NAPI_PPP_USER_NAME_LENGTH];
    INT8    csdPasswd[NAPI_PPP_PASSWORD_LENGTH];
    UINT32  csdLocalIP;
    UINT32  csdLocalDNS1;
    UINT32  csdLocalDNS2;
    UINT16  csdLingerTimer;    
    UINT8   postLogin;
    UINT8   csdSpeed;         

    INT8    csd2DialNumber[NAPI_MAX_PHONE_NUMBER_LENGTH];
    INT8    csd2User[NAPI_PPP_USER_NAME_LENGTH];
    INT8    csd2Passwd[NAPI_PPP_PASSWORD_LENGTH];
    UINT32  csd2LocalIP;
    UINT32  csd2LocalDNS1;
    UINT32  csd2LocalDNS2;
    UINT16  csd2LingerTimer;    
    UINT8   csd2postLogin;
    UINT8   csd2Speed;        
#ifdef MAKE_DATANETWORK_PROXY_URL_ENABLE
    INT8    proxy1[ NAPI_MAX_DOMAIN_LENGTH ];
#else
    UINT32 proxy1;
#endif
    UINT16  port1;
    INT8    domain1[ NAPI_MAX_DOMAIN_LENGTH ];
    UINT8   serviceType1;
#ifdef MAKE_DATANETWORK_PROXY_URL_ENABLE
    INT8    proxy2[ NAPI_MAX_DOMAIN_LENGTH ];
#else
    UINT32 proxy2;
#endif
    UINT16  port2;
    INT8    domain2[ NAPI_MAX_DOMAIN_LENGTH ];
    UINT8   serviceType2;
} NAPI_PROFILE_S;

#define NAPI_IPSEC_DATA_OFFSET  sizeof(NAPI_PROFILE_S)
#define NAPI_ALL_STATE_FLAG                0x00000001
#define NAPI_DETAIL_STATE_FLAG             0x00000002
#define NAPI_CONNECTED_FLAG                0x00000004

#define NAPI_STATE_CALLING_MSGID          0x01
#define NAPI_STATE_NEGOTIATING_MSGID      0x02
#define NAPI_STATE_CONNECTED_MSGID        0x03
#define NAPI_STATE_DISCONNECTING_MSGID    0x04
#define NAPI_STATE_IDLE_MSGID             0x05

#define NAPI_RESULT_SUCCESS_MSGID         0x10
#define NAPI_RESULT_FAIL_MSGID            0x11

#define NAPI_LINK_BROKEN_MSGID            0x20

#define NAPI_LINK_SWITCH_MSGID            0x30

#define NAPI_RAW_DATA_MSGID               0x40

#define NAPI_LINK_IS_WAPSERVICE          0x02
#define NAPI_LINK_IS_TUNNELON            0x04
#define NAPI_LINK_IS_HOLD                0x08

#define NAPI_MCODE_NO_MEMORY             0x0001
#define NAPI_MCODE_LINK_NOTEXIST         0x0002
#define NAPI_MCODE_LINKID_BUSY           0x0003
#define NAPI_MCODE_PROFILE_ERROR         0x0004
#define NAPI_MCODE_UNKNOWN_VER           0x0005
#define NAPI_MCODE_SYSTEM_BUSY           0x0006
#define NAPI_MCODE_GPRS_FEATUREOFF       0x0007
#define NAPI_MCODE_GPRS_DISABLED         0x0008
#define NAPI_MCODE_GPRS_NETERROR         0x0009
#define NAPI_MCODE_UNKNOWN_REASON        0x000A
#define NAPI_MCODE_OPERATION_CANCEL      0x000B
#define NAPI_MCODE_PPPD_QUIT             0x000C
#define NAPI_MCODE_OPLOCAL_CANCEL        0x000D
#define NAPI_MCODE_WIFI_ERROR            0x000E
#define NAPI_MCODE_IKE_ERROR             0x000F
#define NAPI_MCODE_FIXED_DIAL             0x0010

#define NAPI_SUBCODE_IKE_ERROR_NORMAL         0x0000
#define NAPI_SUBCODE_IKE_ERROR_EAPSIM         0x0001

#define NAPI_SERVICE_UNKNOWN                  0x0000
#define NAPI_SERVICE_WAP_CONN_LESS            0x0001
#define NAPI_SERVICE_WAP_CONN_ORIENTED        0x0002
#define NAPI_SERVICE_WAP_CONN_LESS_SECURE     0x0003
#define NAPI_SERVICE_WAP_CONN_ORIENTED_SECURE 0x0004
#define NAPI_SERVICE_WAP_HTTP                 0x0005
#define NAPI_SERVICE_WAP_HTTP_SECURE          0x0006

typedef struct  _NAPI_NOTIFY_MSG_S
{
    UINT16  msgId;
    UINT8   linkId;
    UINT8   linkType;
    INT8    linkName[NAPI_MAX_LINK_NAME_LENGTH];
    UINT32  phyId;
    union
    {
        UINT32  flags;
        struct
        {
            UINT16  maincode;
            UINT16  subcode;
        } resultIndi;
        UINT32  reserve;
    } u;
} NAPI_NOTIFY_MSG_S;
#define NAPIFLAGS      u.flags
#define NAPIMAINCODE   u.resultIndi.maincode
#define NAPISUBCODE    u.resultIndi.subcode
#define NAPIRAWDATA    linkName

typedef enum _NAPI_INFO_TYPE_E
{
    NAPI_DEVICE_NAME_EV,
    NAPI_MAC_ADDRESS_EV,
    NAPI_IP_ADDRESS_EV,
    NAPI_WIFI_AP_MAC_ADDRESS_EV
}NAPI_INFO_TYPE_E;

#define NAPI_ERROR_NOTINIT    (-1)
#define NAPI_ERROR_INVALID    (-2)
#define NAPI_ERROR_ALREADY    (-3)
#define NAPI_ERROR_SOCK       (-4)
#define NAPI_ERROR_NAMELENGTH (-5)
#define NAPI_ERROR_NORESOURCE (-6)
#define NAPI_ERROR_PROFILE    (-7)
#define NAPI_ERROR_BIND       (-8)
#define NAPI_ERROR_MAXLIMITED (-9)
#define NAPI_ERROR_OTHER      (-100)

INT32   NAPI_Init( UINT32 flag );

INT32   NAPI_OpenLink( INT8* profileName );

INT32   NAPI_Attach(
            UINT8   linkType,
            INT8*   linkName );

INT32   NAPI_Hold(
            UINT8   linkType,
            INT8*   linkName );

INT32   NAPI_Drop(
            UINT8   linkType,
            INT8*   linkName );

INT32   NAPI_CloseLink( INT32 linkid );

void    NAPI_Fini( void );

INT32   NAPI_BindSocket2Link(
            int     sockfd,
            INT32   linkid );

INT32   NAPI_GetDeviceInfo(
            INT32               linkid,
            NAPI_INFO_TYPE_E    request,
            void*               pParam,
            UINT32              paramLength);

INT32   NAPI_ReadProfile(
            INT8*               profileName,
            NAPI_PROFILE_S*     pBasic,
            INT8*               pExt,
            UINT32              size );
            
INT32   NAPI_CreateProfile(
            INT8*               profileName,
            NAPI_PROFILE_S*     pBasic,
            INT8*               pExt,
            UINT32              size );

INT32   NAPI_ListAllProfile(
            INT8*       profileNameList,
            UINT32*     counter );

INT32   NAPI_ModifyProfile(
            INT8*               profileName,
            NAPI_PROFILE_S*     pBasic,
            INT8*               pExt,
            UINT32              size );

INT32 NAPI_RenameProfile(INT8* curProfileName, INT8* newProfileName);

INT32   NAPI_DeleteProfile( INT8* profileName );

INT32   NAPI_IsProfileValid( INT8* profileName );

INT32 NAPI_GetProfileIDByName( INT8* profileName, UINT32 * pProfileID );

INT32 NAPI_GetProfileNameByID( UINT32 profileID, INT8* profileName );

INT32 NAPI_SetProfileNameByID( UINT32 profileID, INT8* profileName );

INT32 NAPI_BuildIPSecTunnel(INT32 parentLinkId, const INT8* pIKEConfig);

#define NAPI_TearIPSecTunnel(tunnelId) \
        NAPI_CloseLink(tunnelId)

INT32 NAPI_UmaBuildSecureTunnel(INT32 parentLinkId, const char* pServAddr);

#define NAPI_UmaTearSecureTunnel(tunnelId) \
        NAPI_CloseLink(tunnelId)

UINT32 NAPI_GetMaxProfiles();

INT32 NAPI_DnsResolverConfig(int link);

#ifdef __cplusplus
}
#endif
#endif

