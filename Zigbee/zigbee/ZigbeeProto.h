#ifndef __ZIGBEE_PROTO_H__
#define __ZIGBEE_PROTO_H__

#include <string>
#include <stdio.h>
//#include "serial.h"
#include "zigbeeInterface.h"

#include "zigbee_utility.h"

#include <thread>


#define SL_START_CHAR 0x01
#define SL_ESC_CHAR 0x02
#define SL_END_CHAR 0x03


typedef enum {
	OLD_HEARTBEAT_FORMAT = 0x0F02,
	OLD_HEARTBEAT_FORMAT_ATTRID = 0xFF02,
	NEW_HEARTBEAT_FORMAT = 0x00008f06,
	NEW_HEARTBEAT_FORMAT_ATTRID = 0xFF01,
} heartbeat_format_e;

//#define OLD_HEARTBEAT_FORMAT 0x0F02
//#define OLD_HEARTBEAT_FORMAT_ATTRID 0xFF02
//#define NEW_HEARTBEAT_FORMAT 0x00008f06
//#define NEW_HEARTBEAT_FORMAT_ATTRID 0xFF01
#define BUFSIZE 1024


//#define XDEBUG_DEFINE_FUNC_INFO(func) do { zigbee_log_info("Enter %s", #func);} while (0)


#define MAX_MESSAGE_LEN 300

//#define COM_PORT "/dev/ttyUSB0"

#define __android__ 1

#ifdef __cplusplus
extern "C" {
#endif

extern int get_report_instance();
extern int onReport(char* msg, int len);

union u_retLen {
  uint32_t uiVal;
  float fVal;
};

typedef enum {
  E_SL_OK,
  E_SL_ERROR,
  E_SL_NOMESSAGE,
  E_SL_ERROR_SERIAL,
  E_SL_ERROR_NOMEM,
} teSL_Status;

typedef enum {
  E_STATE_RX_WAIT_START,
  E_STATE_RX_WAIT_TYPEMSB,
  E_STATE_RX_WAIT_TYPELSB,
  E_STATE_RX_WAIT_LENMSB,
  E_STATE_RX_WAIT_LENLSB,
  E_STATE_RX_WAIT_CRC,
  E_STATE_RX_WAIT_DATA,
} teSL_RxState;

typedef enum {
  E_SL_READ = 0x00,
  E_SL_WRITE = 0x01,
  E_SL_REPORT = 0x02,
} teSL_PlugReadandWrite;

typedef enum {
  E_SL_MSG_VERSION_LIST = 0x8010,
  E_SL_MSG_LEAVE_INDICATION = 0x8048,
  E_SL_MSG_DEVICE_ANNOUNCE = 0x004D,
  E_SL_MSG_PERMIT_JOINING_REQUEST = 0x0049,
  E_SL_MSG_SET_ZIGBEE_CHANNEL = 0x9F03,
  E_SL_MSG_SET_ZIGBEE_TIME = 0x9F06,
  E_SL_MSG_GET_TEMPERATURE = 0x9F0A,
  E_SL_MSG_CALI_TEMPERATURE = 0x9F09,
  E_SL_MSG_ENTER_FACTORY_MODE = 0x9F00,
  E_SL_MSG_ALLOW_JOIN_IN_FACTORY_MODE = 0x9F01,
  E_SL_MSG_ENHANCE_DONGLE_POWER = 0x9F02,
  E_SL_MSG_SCAN_CHANNEL_ENERGY = 0x9F04,
  E_SL_MSG_ED_SCAN_INDICATION = 0x9F84,
  E_SL_MSG_ED_SCAN_INDICATION_EXTENDED = 0x9F97,
  E_SL_MSG_GET_EEPAN_ID = 0x001F,
  E_SL_MSG_MANAGE_LQI_REQUEST = 0x9F0C,

  E_SL_MSG_MANAGE_LQI_RSP = 0x9F8C,
  E_SL_MSG_GET_EEPAN_ID_RSP = 0x801F,

  E_SL_MSG_NWK_INFO_EXTRACTION = 0x9F18,
  
  E_SL_MSG_NWK_INFO_EXTRACT_RSP = 0x9F98,

  E_SL_MSG_GET_TEMPERATURE_RSP = 0x9F8A,
  
  E_SL_MSG_UART_COMMAND_REV_RSP = 0x9F96,
  E_SL_MSG_LEAVE_INDICATION_THROUGH_ROUTER = 0x804C,

  /*update message type*/
  UPGRADE_BLOCK_REQUEST = 0x8501,
  UPGRADE_END_REQUEST = 0x8503,
  UPGRADE_NEXT_IMAGE_REQ = 0x9F85,

  /*priv define*/
  E_SL_MSG_ATTRIBE_PV_REPORT = 0x8F00,
  E_SL_MSG_ATTRIBE_IEEE_ADDRESS = 0x8049,
  E_SL_MSG_ATTRIBE_MODELID_REPORT = 0x8F02,
  E_SL_MSG_ATTRIBE_APPVERSION_REPORT = 0x8F03,
  E_SL_MSG_NETWORK_REMOVE_DEVICE = 0x0026,
  E_SL_MSG_ONOFF = 0x0092,
  E_SL_MSG_GET_VERSION = 0x0010,
  E_SL_MSG_GET_NWK_INFO = 0x001a,
  E_SL_MSG_WRITE_ATTRIBUTE_RSP = 0x8110,  
  E_SL_MSG_ONOFF_NOEFFECTS_RETRANSMISSION = 0x0F07,

    /* Level Cluster */
  E_SL_MSG_MOVE_TO_LEVEL = 0x0080,
  E_SL_MSG_MOVE_TO_LEVEL_ONOFF = 0x0081,
  E_SL_MSG_MOVE_STEP = 0x0082,
  E_SL_MSG_MOVE_STOP_MOVE = 0x0083,
  E_SL_MSG_MOVE_STOP_ONOFF = 0x0084,
  E_SL_MSG_NWK_INFO = 0x801a,  
  E_SL_MSG_MOVE_TO_LEVEL_RETRANSMISSION = 0x0F26,

  /* Write Attribute Request&Response */
  E_SL_MSG_WRITE_ATTRIBUTE_REQ = 0x0110,
  //E_SL_MSG_WRITE_ATTRIBUTE_RSP = 0x8110,

  /* Colour Cluster */
  E_SL_MSG_MOVE_TO_HUE = 0x00B0,
  E_SL_MSG_MOVE_HUE = 0x00B1,
  E_SL_MSG_STEP_HUE = 0x00B2,
  E_SL_MSG_MOVE_TO_SATURATION = 0x00B3,
  E_SL_MSG_MOVE_SATURATION = 0x00B4,
  E_SL_MSG_STEP_SATURATION = 0x00B5,
  E_SL_MSG_MOVE_TO_HUE_SATURATION = 0x00B6,
  E_SL_MSG_MOVE_TO_COLOUR = 0x00B7,
  E_SL_MSG_MOVE_COLOUR = 0x00B8,
  E_SL_MSG_STEP_COLOUR = 0x00B9,

  /* Colour Cluster */
  E_SL_MSG_ENHANCED_MOVE_TO_HUE = 0x00BA,
  E_SL_MSG_ENHANCED_MOVE_HUE = 0x00BB,
  E_SL_MSG_ENHANCED_STEP_HUE = 0x00BC,
  E_SL_MSG_ENHANCED_MOVE_TO_HUE_SATURATION = 0x00BD,
  E_SL_MSG_COLOUR_LOOP_SET = 0x00BE,
  E_SL_MSG_STOP_MOVE_STEP = 0x00BF,
  E_SL_MSG_MOVE_TO_COLOUR_TEMPERATURE = 0x00C0,
  E_SL_MSG_MOVE_COLOUR_TEMPERATURE = 0x00C1,
  E_SL_MSG_STEP_COLOUR_TEMPERATURE = 0x00C2,

  /* Report Plug Function Attributes */
  E_SL_MSG_PLUG_FUNCTION_ONOFF_REPORT = 0xFFF0,
} teSL_MsgType;

typedef enum eZigDataType {
  /* Null */
  E_ZCL_NULL = 0x00,

  /* General Data */
  E_ZCL_GINT8 = 0x08, // General 8 bit - not specified if signed or not
  E_ZCL_GINT16,
  E_ZCL_GINT24,
  E_ZCL_GINT32,
  E_ZCL_GINT40,
  E_ZCL_GINT48,
  E_ZCL_GINT56,
  E_ZCL_GINT64,

  /* Logical */
  E_ZCL_BOOL = 0x10,

  /* Bitmap */
  E_ZCL_BMAP8 = 0x18, // 8 bit bitmap
  E_ZCL_BMAP16,
  E_ZCL_BMAP24,
  E_ZCL_BMAP32,
  E_ZCL_BMAP40,
  E_ZCL_BMAP48,
  E_ZCL_BMAP56,
  E_ZCL_BMAP64,

  /* Unsigned Integer */
  E_ZCL_UINT8 = 0x20, // Unsigned 8 bit
  E_ZCL_UINT16,
  E_ZCL_UINT24,
  E_ZCL_UINT32,
  E_ZCL_UINT40,
  E_ZCL_UINT48,
  E_ZCL_UINT56,
  E_ZCL_UINT64,

  /* Signed Integer */
  E_ZCL_INT8 = 0x28, // Signed 8 bit
  E_ZCL_INT16,
  E_ZCL_INT24,
  E_ZCL_INT32,
  E_ZCL_INT40,
  E_ZCL_INT48,
  E_ZCL_INT56,
  E_ZCL_INT64,

  /* Enumeration */
  E_ZCL_ENUM8 = 0x30, // 8 Bit enumeration
  E_ZCL_ENUM16,

  /* Floating Point */
  E_ZCL_FLOAT_SEMI = 0x38, // Semi precision
  E_ZCL_FLOAT_SINGLE, // Single precision
  E_ZCL_FLOAT_DOUBLE, // Double precision

  /* String */
  E_ZCL_OSTRING = 0x41, // Octet string
  E_ZCL_CSTRING, // Character string
  E_ZCL_LOSTRING, // Long octet string
  E_ZCL_LCSTRING, // Long character string

  /* Ordered Sequence */
  E_ZCL_ARRAY = 0x48,
  E_ZCL_STRUCT = 0x4c,
  E_ZCL_SET = 0x50,
  E_ZCL_BAG = 0x51,

  /* Time */
  E_ZCL_TOD = 0xe0, // Time of day
  E_ZCL_DATE, // Date
  E_ZCL_UTCT, // UTC Time

  /* Identifier */
  E_ZCL_CLUSTER_ID = 0xe8, // Cluster ID
  E_ZCL_ATTRIBUTE_ID, // Attribute ID
  E_ZCL_BACNET_OID, // BACnet OID

  /* Miscellaneous */
  E_ZCL_IEEE_ADDR = 0xf0, // 64 Bit IEEE Address
  E_ZCL_KEY_128, // 128 Bit security key, currently not supported as it would add to code space in u16ZCL_WriteTypeNBO and add extra 8 bytes to report config record for each reportable attribute

  /* NOTE:
  * 0xfe is a reserved value, however we are using it to denote a message signature.
  * This may have to change some time if ZigBee ever allocate this value to a data type
  */
  E_ZCL_SIGNATURE = 0xfe, // ECDSA Signature (42 bytes)

  /* Unknown */
  E_ZCL_UNKNOWN = 0xff
} eZigDataType;

typedef enum {
  E_STATE_NONE,
  E_STATE_ANNOC,
  E_STATE_ACTIVIT_EP,
  E_STATE_MODELID,
  E_STATE_VERSION,
  E_STATE_SIMPLE_DESCRIPTION,
  E_STATE_DONE,
} te_initstate;

typedef union {
  uint32_t u32_val;
  float f_val;
} float_u32_t;

#ifndef MZERO
#define MZERO(x) memset(&(x), 0, sizeof(x))
#endif

/*
小锟阶碉拷锟借备模锟斤拷没锟斤拷锟斤拷锟矫讹拷应锟斤拷枚锟斤拷值,小锟阶碉拷模锟斤拷值锟斤拷锟街凤拷锟斤拷 eg: lumi.gateway.v1 锟斤拷锟斤拷前姹撅拷锟?
锟轿加比较碉拷锟斤拷 lumi.gateway  锟斤拷锟斤拷lumi 锟角癸拷司锟斤拷锟街ｏ拷 gateway 锟角诧拷品锟斤拷锟斤拷
锟斤拷为锟斤拷锟截筹拷锟斤拷锟叫碉拷锟竭硷拷锟斤拷锟?model 锟津交碉拷,为锟剿硷拷锟斤拷锟节达拷锟斤拷锟斤拷锟斤拷枚锟劫猴拷锟斤拷锟斤拷锟斤拷锟街凤拷锟斤拷.
*/

/*
锟斤拷锟斤拷:锟斤拷锟杰筹拷始锟斤拷锟斤拷锟斤拷
锟斤拷锟斤拷:锟斤拷锟斤拷锟截筹拷始锟斤拷时锟斤拷锟斤拷
*/
//extern void init_gateway_setting();

//锟借备锟斤拷虻サ亩锟斤拷锟?

typedef struct stEnergyCol {
  uint32_t au32Data[8];
} stEnergyCol;

typedef struct
  {
  uint8_t u8Endpoint;
  uint16_t u16ProfileID;
  uint16_t u16DeviceID;
  uint8_t u8NumClusters;
  uint16_t* pasClusters;
} tsZCB_NodeEndpoint;

typedef struct device_item {
  unsigned long long sid;
  unsigned int model;
  uint32_t u32Voltage; // 锟斤拷压实锟斤拷值
  uint16_t u16Current;
  int16_t temperature;
  uint32_t last_msg_time; //load set the local time. connected network,get the time and set it.
  uint32_t power_low_event_time;
  tsZCB_NodeEndpoint* pasEndpoints;
  unsigned short short_id;
  char version;
  char online : 1; // max add this 2015.1.7
  char report_ver_flag : 1;
  char power_on : 2;
  unsigned char power; // sensor voltage, 0 -- to-- 100
  uint8_t capacity;
  bool isLuMiDevice;
  uint8_t u8NumEndpoints;
  uint8_t u8Lqi;
  uint8_t report_flag;
  uint8_t pv_state;
  uint8_t resend_sucess_cnt;
  uint8_t resend_sucess_avg_cnt;
  uint8_t send_fail_cnt;
  uint8_t cur_state;
  uint8_t pre_state;
  uint8_t power_tx;
  uint8_t CCA;
  uint16_t reset_cnt;
  uint16_t send_all_cnt;
  uint32_t OtaVersion;
  // list_t      *object_list;  not used for the time being, as all devices are in config file device_info.txt
  uint32_t bind_version : 31;
  uint32_t bind_update : 1;
  uint32_t bind_CRC32;
  union Sensor_data {
    uint8_t end_state;
    struct Neu_data {
      uint8_t end_state;
      uint32_t ctrl0_time_ms; //the time (unit ms) when neutral_0 is ctrl
      uint32_t ctrl1_time_ms; //the time (unit ms) when neutral_1 is ctrl
      uint32_t u32Vol : 25;
      uint8_t trigger : 4;
      uint8_t prop1 : 1;
      uint8_t prop2 : 1;
      uint8_t prop3 : 1;
      uint8_t update_ep : 1; //endpoint which is update state
      float power_value;
      float power_consumed; // kWh for the total power consumed
      stEnergyCol* pEnergyCol;
    } neu_data;

    struct Ht_data {
      int16_t i16Temperature;
      uint16_t ui16Humidity;
      //uint32_t u32Time;
      uint8_t eState;
      uint8_t eNewState;
    } ht_data;

    struct Switch_data {
      bool is_long_click;
      uint32_t timer;
    } switch_data;

    struct Light_data {
      uint8_t u8Status;
      uint8_t u8Level;
      uint8_t u8QueryFlag;
    } Light_data;

    struct Cube_data {
      uint8_t* alg_cfg_data; //byte0: alg data length ; bit1:num_attribute=1, bit2:datatype=0x41, bit3~4:attrid,bit5:str_len,bit6~ alg config data
      uint16_t invalid_count;
      uint16_t wakeup_num;
      uint16_t distur_num;
      uint16_t param_version;
      uint16_t cfg_value_mask; //bit0-bit1: en_motion_motor, bit2-bit3:en_swing_motion
      uint8_t cfg_idx_mask; //bit0: en_motion_motor, bit1:en_swing_motion,bit2:request algorithm config
      uint8_t en_motion_motor : 2;
      uint8_t en_swing_motion : 2;
    } Cube_data;

    struct Curtain_data {
      uint8_t param_len;
      uint8_t cfg_param[15];
    } Curtain_data;

  } sensor_data;
} device_item_t;


typedef struct Heartbeat_BasicItem {
	uint16_t u16Voltage;   //index 1
	uint16_t u16Current;	//index 2
	int8_t i8Temperature;	//index 3
	//system state		//index 4
	uint8_t u8CurrentState;
	uint8_t u8PreState;
	uint8_t u8Power_Tx;
	bool CCA_Mode;

	uint16_t u16ResetCount;	//index 5

	//LQI related data	//index 6
	uint16_t u16TotalSendNo;
	uint8_t u8FailSendNo;
	uint8_t u8ResendSuccNo;
	uint8_t u8AvgResendSuccNo;

	uint64_t u64BindTable_CRC;	//index 7
	
	//u8HardwareVer, u8SoftwareVer //index 8
	uint8_t u8HardVer;
	uint8_t u8SoftVer;
	
	uint16_t u16Neighbour_Table; //index 9
	uint16_t u16ParentShortAdd; //index 10

	uint8_t u8lqi;  //last data byte
}	HeartbeatBasic;

typedef struct Heartbeat_PlugItem{
	uint8_t u8CH0_State;    //index 100
	float fPowerConsumption;     //index 149
	uint32_t u32LoadVoltage; //index 150
	float fLoadPower;       //index 152
	uint8_t u8FaultIndex;   //index 154

}PlugItem;

typedef struct Heartbeat_Ctrl_NeutralItem{
	uint8_t u8CH0_State;	//index 100
	uint8_t u8CH1_State;	//index 101
	uint8_t u8CH0_LoadState; //index 110
	uint8_t u8CH1_LoadState; //index 111
	uint32_t u32LoadPower;	//index 153
	
}Ctrl_NeutralItem;

typedef struct Heartbeat_CubeItem{
	uint16_t u16InvalidCount;	//index 151
	uint16_t u16SensorWakeup_No;	//index 152
	uint16_t u16Disturb_No;	//index 153
	uint16_t u16ParamVersion;	//index 154
}CubeItem;

typedef struct Heartbeat_HumidityItem{
	int16_t	i16Environ_Temp;	//index 100
	uint16_t u16Environ_Humidi; //index 101
}HumidityItem;




class ZigbeeProto;
typedef int (ZigbeeProto::*ParseFun)(uint16_t type, uint8_t* data, int len);

class ZigbeeProto {
public:
  ZigbeeProto();
  ~ZigbeeProto();

public:
  int open();
  void close();

  int read_com();

public:
  void zigbee_get_netinfo();
  void enhance_zigbee_power();

  int onCommand(string command);

  int send_message(uint16_t type, uint16_t len, uint8_t* data);

  int send_message_incommandlist(uint16_t type, uint16_t len, uint8_t* data);

  int resend_message(uint16_t type, uint16_t len, uint8_t *data);
  
  int read_message();

  int recv_zigbee_data(char* recv_buf, int recv_len);

  int handle_old_heartbeat(uint16_t type, uint8_t* data, int len);
  int handle_new_heartbeat(uint16_t type, uint8_t* data, int len);
  //int say_hello(uint16_t type, uint8_t* data, int len);
private:
  int push_byte(bool bSpecialCharacter, uint8_t u8Data, uint8_t* buf, int* pos);
  uint8_t calculate_crc(uint16_t u16Type, uint16_t u16Length, uint8_t* pu8Data);
  int parse_attribe_pv_report(uint16_t type, uint8_t* data, int len);
  int parse_leave_indication(uint16_t type, uint8_t* data, int len);
  int parse_attribe_modelid_report(uint16_t type, uint8_t* data, int len);
  int parse_attribe_appversion_report(uint16_t type, uint8_t* data, int len);
  int parse_device_announce(uint16_t type, uint8_t* data, int len);
  int parse_leave_indication_through_router(uint16_t type, uint8_t* data, int len);
  int parse_energy_detection_indication(uint16_t type, uint8_t* data, int len);
  int parse_energy_detection_indication_extended(uint16_t type, uint8_t* data, int len);
  int parse_get_eepan_response(uint16_t type, uint8_t* data, int len);
  int parse_management_lqi_response(uint16_t type, uint8_t* data, int len);
  int parse_nwk_info_extract_response(uint16_t type, uint8_t* data, int len);
  int parse_attribute_response(uint16_t type, uint8_t* data, int len);
  int parse_plug_function_onoff(uint8_t* data, int len);
  //int handle_old_heartbeat(uint16_t type, uint8_t* data, int len);
  //int handle_new_heartbeat(uint16_t type, uint8_t* data, int len);
  int parse(uint16_t type, uint8_t* data, int len);
  uint8_t getLenByZigType(eZigDataType eDataType);

  int parse_write_attribute_response(uint8_t* data, int len);

public:
  int permit_zigbee_join(uint8_t value);
  int remove_zigbee_device(unsigned short short_id, unsigned long long long_id);
  void control_on_off_toggle(uint16_t short_id, uint8_t u8EndPoint, uint8_t status);
  void write_plug_function_attribute(uint16_t short_id, uint8_t operate_type, uint8_t *data_buf,uint8_t len); //poweroff_memory, charge_protect, en_night_tip_light
  void control_open_close_curtain(uint16_t short_id, uint8_t u8EndPoint, uint8_t status);
  void control_plug(uint16_t short_id, string status);
  //void control_no_neutral(uint16_t short_id, uint8_t u8EndPoint, string status);
  void control_no_neutral_1(uint16_t short_id, uint8_t u8EndPoint, string status);
  void control_no_neutral_2(uint16_t short_id, uint8_t u8EndPoint, string status);
  void control_light_x_y(uint16_t short_id, uint16_t ix, uint16_t iy);
  void control_light_hue(uint16_t short_id, uint16_t ihue);
  void control_light_level(uint16_t short_id, uint8_t ilevel);
  void control_light_color_temperature(uint16_t short_id, uint16_t icolor_temperature);
  void control_curtain_level(uint16_t short_id, float curtain_level);
  void control_curtain(uint16_t short_id, string status);
  void get_fw_version(void);
  void get_network_info(void);
  void set_gateway_channel(uint8_t channel);
  void set_time_to_network(uint32_t time);
  void calibration_temperature(int temperature);
  void get_temperature(void);
  void enter_zigbee_factory_mode(void);
  void allow_join_in_zigbee_factory_mode(void);
  void scan_zigbee_channel_energy(void);
  void get_zigbee_eepan_id(void);
  void management_zigbee_LQI_request(uint16_t short_id, uint8_t index);
  void get_zigbee_nwk_extracted_info(void);
  
#if !defined(__android__)
  serial::Serial* m_pCom; //("COM10", 115200, serial::Timeout::simpleTimeout(1000));
#endif

  int eRxState; // = (int)E_STATE_RX_WAIT_START;

public:
  char firmware_version[MAX_MESSAGE_LEN];
  uint64_t dongle_ieee_addr;

  uint64_t dongle_eepan;

  uint16_t pan_id;

  uint8_t channel;

  uint8_t temperature;

  bool bWritePlugAttr;
  

private:
  //bool updating_fw;

  //------------锟斤拷锟节讹拷取锟斤拷锟斤拷锟斤拷锟?	//int eRxState;// = (int)E_STATE_RX_WAIT_START;
  uint16_t pu16Type;
  uint16_t pu16Length;
  uint8_t message[MAX_MESSAGE_LEN];
  bool bInEsc;
  uint16_t u16Bytes;
  uint8_t u8CRC;

  uint16_t zigbee_join_short_id;
  uint64_t zigbee_join_ieee_addr;
  uint32_t zigbee_join_capacity;
  uint16_t zigbee_join_device_status;
  uint32_t zigbee_join_model;
  bool zigbee_join_device_isLuMiDevice;
  uint16_t zigbee_join_version;
};

extern ZigbeeProto zigbee;


int on_report(char* msg, int len);


void zigbee_resend_task();

void check_commandlist_task();

//
#if defined __cplusplus
}
#endif
#endif
