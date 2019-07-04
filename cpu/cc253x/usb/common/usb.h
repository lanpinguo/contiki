#ifndef USB_H_6PFTDPIMZM__
#define USB_H_6PFTDPIMZM__
#include <stdint.h>

/* Adapted from usb_kbd_enum.h in c5131-usb-kbd-light-1_0_2 package from
   Atmel */

/* These definitions assume a little endian architecture */

#ifdef __GNUC__
#define BYTE_ALIGNED __attribute__ ((__packed__))
#else
#define BYTE_ALIGNED
#endif

#define LOW_BYTE(x) ((unsigned char)x)
#define HIGH_BYTE(x) ((unsigned char)(x>>8))

typedef uint8_t Uchar;
typedef uint16_t Uint16;
typedef uint32_t Uint32;

/*_____ S T A N D A R D    R E Q U E S T S __________________________________*/

#define GET_STATUS            0x00
#define GET_DEVICE            0x01
#define CLEAR_FEATURE         0x01        /* see FEATURES below */
#define GET_STRING            0x03
#define SET_FEATURE           0x03        /* see FEATURES below */
#define SET_ADDRESS           0x05
#define GET_DESCRIPTOR        0x06
#define SET_DESCRIPTOR        0x07
#define GET_CONFIGURATION     0x08
#define SET_CONFIGURATION     0x09
#define GET_INTERFACE         0x0A
#define SET_INTERFACE         0x0B
#define SYNCH_FRAME           0x0C

#define GET_DEVICE_DESCRIPTOR           1
#define GET_CONFIGURATION_DESCRIPTOR    4

#define REQUEST_DEVICE_STATUS         0x80
#define REQUEST_INTERFACE_STATUS      0x81
#define REQUEST_ENDPOINT_STATUS       0x82
#define ZERO_TYPE                     0x00
#define INTERFACE_TYPE                0x01
#define ENDPOINT_TYPE                 0x02

/*_____ D E S C R I P T O R    T Y P E S ____________________________________*/

#define USB_DT_DEVICE                   0x01
#define USB_DT_CONFIGURATION            0x02
#define USB_DT_STRING                   0x03
#define USB_DT_INTERFACE                0x04
#define USB_DT_ENDPOINT                 0x05
#define USB_DT_DEVICE_QUALIFIER		      0x06
#define USB_DT_OTHER_SPEED_CONFIG	      0x07
#define USB_DT_INTERFACE_POWER		      0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			                0x09
#define USB_DT_DEBUG			              0x0a
#define USB_DT_INTERFACE_ASSOCIATION	  0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			            0x0c
#define USB_DT_KEY			                0x0d
#define USB_DT_ENCRYPTION_TYPE		      0x0e
#define USB_DT_BOS			                0x0f
#define USB_DT_DEVICE_CAPABILITY	      0x10

/* HID specific */
#define HID                   0x21
#define REPORT                0x22
/* *** */

/*_____ S T A N D A R D    F E A T U R E S __________________________________*/

#define DEVICE_REMOTE_WAKEUP_FEATURE     0x01
#define ENDPOINT_HALT_FEATURE            0x00

/*_____ D E V I C E   S T A T U S ___________________________________________*/

#define SELF_POWERED       1

/*_____ D E V I C E   S T A T E _____________________________________________*/

#define ATTACHED                  0
#define POWERED                   1
#define DEFAULT                   2
#define ADDRESSED                 3
#define CONFIGURED                4
#define SUSPENDED                 5

#define USB_CONFIG_BUSPOWERED     0x80
#define USB_CONFIG_SELFPOWERED    0x40
#define USB_CONFIG_REMOTEWAKEUP   0x20

/* Class specific */
#define CS_INTERFACE	0x24
#define CS_ENDPOINT	0x25

/*_________________________________________________________ S T R U C T _____*/
/*_____ U S B   D E V I C E   R E Q U E S T _________________________________*/

struct USB_request_st
{
  Uchar   bmRequestType;        /* Characteristics of the request */
  Uchar   bRequest;             /* Specific request */
  Uint16 wValue;
  Uint16  wIndex;  /* field that varies according to request */
  Uint16  wLength;              /* Number of bytes to transfer if Data */
};


/*_____ U S B   D E V I C E   D E S C R I P T O R ___________________________*/

struct usb_st_device_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;       /* DEVICE descriptor type */
  Uint16 bscUSB;                /* Binay Coded Decimal Spec. release */
  Uchar  bDeviceClass;          /* Class code assigned by the USB */
  Uchar  bDeviceSubClass;       /* Sub-class code assigned by the USB */
  Uchar  bDeviceProtocol;       /* Protocol code assigned by the USB */
  Uchar  bMaxPacketSize0;       /* Max packet size for EP0 */
  Uint16 idVendor;              /* Vendor ID. ATMEL = 0x03EB */
  Uint16 idProduct;             /* Product ID assigned by the manufacturer */
  Uint16 bcdDevice;             /* Device release number */
  Uchar  iManufacturer;         /* Index of manu. string descriptor */
  Uchar  iProduct;              /* Index of prod. string descriptor */
  Uchar  iSerialNumber;         /* Index of S.N.  string descriptor */
  Uchar  bNumConfigurations;    /* Number of possible configurations */
} BYTE_ALIGNED;


/*_____ U S B   C O N F I G U R A T I O N   D E S C R I P T O R _____________*/

struct usb_st_configuration_descriptor
{
  Uchar  bLength;               /* size of this descriptor in bytes */
  Uchar  bDescriptorType;       /* CONFIGURATION descriptor type */
  Uint16 wTotalLength;          /* total length of data returned */
  Uchar  bNumInterfaces;        /* number of interfaces for this conf. */
  Uchar  bConfigurationValue;   /* value for SetConfiguration resquest */
  Uchar  iConfiguration;        /* index of string descriptor */
  Uchar  bmAttibutes;           /* Configuration characteristics */
  Uchar  MaxPower;              /* maximum power consumption */
} BYTE_ALIGNED;


/*_____ U S B   I N T E R F A C E   D E S C R I P T O R _____________________*/

struct usb_st_interface_descriptor
{
  Uchar bLength;                /* size of this descriptor in bytes */
  Uchar bDescriptorType;        /* INTERFACE descriptor type */
  Uchar bInterfaceNumber;       /* Number of interface */
  Uchar bAlternateSetting;      /* value to select alternate setting */
  Uchar bNumEndpoints;          /* Number of EP except EP 0 */
  Uchar bInterfaceClass;        /* Class code assigned by the USB */
  Uchar bInterfaceSubClass;     /* Sub-class code assigned by the USB */
  Uchar bInterfaceProtocol;     /* Protocol code assigned by the USB */
  Uchar iInterface;             /* Index of string descriptor */
} BYTE_ALIGNED;


/*_____ U S B   E N D P O I N T   D E S C R I P T O R _______________________*/

struct usb_st_endpoint_descriptor
{
  Uchar  bLength;               /* Size of this descriptor in bytes */
  Uchar  bDescriptorType;       /* ENDPOINT descriptor type */
  Uchar  bEndpointAddress;      /* Address of the endpoint */
  Uchar  bmAttributes;          /* Endpoint's attributes */
  Uint16 wMaxPacketSize;        /* Maximum packet size for this EP */
  Uchar  bInterval;             /* Interval for polling EP in ms */
/*   Uchar  bRefresh; */
/*   Uchar  bSynchAddress; */
} BYTE_ALIGNED;


/*_____ U S B   S T R I N G   D E S C R I P T O R _______________*/

struct usb_st_string_descriptor
{
  Uchar  bLength;               /* size of this descriptor in bytes */
  Uchar  bDescriptorType;       /* STRING descriptor type */
  Uint16 wstring[1];/* unicode characters */
} BYTE_ALIGNED;


struct usb_st_language_descriptor
{
  Uchar  bLength;               /* size of this descriptor in bytes */
  Uchar  bDescriptorType;       /* STRING descriptor type */
  Uint16 wlangid[1];               /* language id */
} BYTE_ALIGNED;


/*-------------------------------------------------------------------------*/

/* USB_DT_BOS:  group of device-level capabilities */
struct usb_st_bos_descriptor {
	Uchar  bLength;
	Uchar  bDescriptorType;

	Uint16 wTotalLength;
	Uchar  bNumDeviceCaps;
} __attribute__((packed));

#define USB_DT_BOS_SIZE		5
/*-------------------------------------------------------------------------*/

/* USB_DT_DEVICE_CAPABILITY:  grouped with BOS */
struct usb_st_dev_cap_header {
	Uchar  bLength;
	Uchar  bDescriptorType;
	Uchar  bDevCapabilityType;
} __attribute__((packed));

#define	USB_CAP_TYPE_WIRELESS_USB	1

struct usb_st_wireless_cap_descriptor {	/* Ultra Wide Band */
	Uchar  bLength;
	Uchar  bDescriptorType;
	Uchar  bDevCapabilityType;

	Uchar  bmAttributes;
#define	USB_WIRELESS_P2P_DRD		(1 << 1)
#define	USB_WIRELESS_BEACON_MASK	(3 << 2)
#define	USB_WIRELESS_BEACON_SELF	(1 << 2)
#define	USB_WIRELESS_BEACON_DIRECTED	(2 << 2)
#define	USB_WIRELESS_BEACON_NONE	(3 << 2)
	Uint16 wPHYRates;	/* bit rates, Mbps */
#define	USB_WIRELESS_PHY_53		(1 << 0)	/* always set */
#define	USB_WIRELESS_PHY_80		(1 << 1)
#define	USB_WIRELESS_PHY_107		(1 << 2)	/* always set */
#define	USB_WIRELESS_PHY_160		(1 << 3)
#define	USB_WIRELESS_PHY_200		(1 << 4)	/* always set */
#define	USB_WIRELESS_PHY_320		(1 << 5)
#define	USB_WIRELESS_PHY_400		(1 << 6)
#define	USB_WIRELESS_PHY_480		(1 << 7)
	Uchar  bmTFITXPowerInfo;	/* TFI power levels */
	Uchar  bmFFITXPowerInfo;	/* FFI power levels */
	Uint16 bmBandGroup;
	Uchar  bReserved;
} __attribute__((packed));

/* USB 2.0 Extension descriptor */
#define	USB_CAP_TYPE_EXT		2

struct usb_st_ext_cap_descriptor {		/* Link Power Management */
	Uchar  bLength;
	Uchar  bDescriptorType;
	Uchar  bDevCapabilityType;
	Uint32 bmAttributes;
#define USB_LPM_SUPPORT			(1 << 1)	/* supports LPM */
#define USB_BESL_SUPPORT		(1 << 2)	/* supports BESL */
#define USB_BESL_BASELINE_VALID		(1 << 3)	/* Baseline BESL valid*/
#define USB_BESL_DEEP_VALID		(1 << 4)	/* Deep BESL valid */
#define USB_GET_BESL_BASELINE(p)	(((p) & (0xf << 8)) >> 8)
#define USB_GET_BESL_DEEP(p)		(((p) & (0xf << 12)) >> 12)
} __attribute__((packed));

#define USB_DT_USB_EXT_CAP_SIZE	7

/*
 * SuperSpeed USB Capability descriptor: Defines the set of SuperSpeed USB
 * specific device level capabilities
 */
#define		USB_SS_CAP_TYPE		3
struct usb_st_ss_cap_descriptor {		/* Link Power Management */
	Uchar  bLength;
	Uchar  bDescriptorType;
	Uchar  bDevCapabilityType;
	Uchar  bmAttributes;
#define USB_LTM_SUPPORT			(1 << 1) /* supports LTM */
	Uint16 wSpeedSupported;
#define USB_LOW_SPEED_OPERATION		(1)	 /* Low speed operation */
#define USB_FULL_SPEED_OPERATION	(1 << 1) /* Full speed operation */
#define USB_HIGH_SPEED_OPERATION	(1 << 2) /* High speed operation */
#define USB_5GBPS_OPERATION		(1 << 3) /* Operation at 5Gbps */
	Uchar  bFunctionalitySupport;
	Uchar  bU1devExitLat;
	Uint16 bU2DevExitLat;
} __attribute__((packed));




#endif /* USB_H_6PFTDPIMZM__ */
